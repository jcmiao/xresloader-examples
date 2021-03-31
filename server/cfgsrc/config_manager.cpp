﻿
// Copyright 2021 xresloader. All rights reserved.
// Generated by xres-code-generator, please don't edit it
//

#ifdef _MSC_VER
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif

#include <cstdio>
#include <sstream>
#include <cstdarg>

#if (defined(_MSC_VER) && _MSC_VER >= 1600) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L)
#define EXCEL_CONFIG_FS_OPEN(e, f, path, mode) errno_t e = fopen_s(&f, path, mode)
#define EXCEL_CONFIG_FS_CLOSE(f) fclose(f)
#else
#include <errno.h>
#define EXCEL_CONFIG_FS_OPEN(e, f, path, mode)                  f = fopen(path, mode);                              int e = errno
#define EXCEL_CONFIG_FS_CLOSE(f) fclose(f)
#endif


#if (defined(_MSC_VER) && _MSC_VER >= 1600) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || defined(__STDC_LIB_EXT1__)

#ifdef _MSC_VER
#define EXCEL_CONFIG_VSNPRINTF(buffer, bufsz, fmt, arg) vsnprintf_s(buffer, static_cast<size_t>(bufsz), _TRUNCATE, fmt, arg)
#else
#define EXCEL_CONFIG_VSNPRINTF(buffer, bufsz, fmt, arg) vsnprintf_s(buffer, static_cast<size_t>(bufsz), fmt, arg)
#endif

#else
#define EXCEL_CONFIG_VSNPRINTF(buffer, bufsz, fmt, arg) vsnprintf(buffer, static_cast<int>(bufsz), fmt, arg)
#endif

#include "config_manager.h"

namespace excel {

    bool config_manager::is_destroyed_ = false;

    namespace details {
        static bool get_file_content(std::string &out, const char *file_path) {
            FILE *f = NULL;

            EXCEL_CONFIG_FS_OPEN(error_code, f, file_path, "rb");
            ((void)error_code); // unused

            if (NULL == f) {
                return false;
            }

            fseek(f, 0, SEEK_END);
            long len = ftell(f);
            fseek(f, 0, SEEK_SET);

            bool ret = true;
            if (len > 0) {
                out.resize(static_cast<size_t>(len));
                size_t real_read_sz = fread(const_cast<char *>(out.data()), sizeof(char), static_cast<size_t>(len), f);
                if (real_read_sz < out.size()) {
                    out.resize(real_read_sz);
                    // CLRF maybe converted into CL or RF on text mode
                    ret = false;
                }
            } else {
                // 虚拟文件ftell(f)会拿不到长度，只能按流来读
                char              buf[4096]; // 4K for each block
                std::stringstream ss;
                while (true) {
                    size_t read_sz = fread(buf, 1, sizeof(buf), f);
                    ss.write(buf, read_sz);
                    if (read_sz < sizeof(buf)) {
                        break;
                    }
                }

                ss.str().swap(out);
            }

            EXCEL_CONFIG_FS_CLOSE(f);
            return ret;
        }
    }

    config_manager::log_caller_info_t::log_caller_info_t(): level_id(log_level_t::LOG_LW_DISABLED), level_name(NULL), file_path(NULL), line_number(0), func_name(NULL) {}

    config_manager::log_caller_info_t::log_caller_info_t(log_level_t::type lid, const char *lname, const char *fpath, uint32_t lnum, const char *fnname):
        level_id(lid), level_name(lname), file_path(fpath), line_number(lnum), func_name(fnname) {}

    config_manager::config_manager() : 
        override_same_version_(false),
        max_group_number_(8),
        on_log_(config_manager::default_log_writer),
        read_file_handle_(config_manager::default_buffer_loader), 
        read_version_handle_(default_version_loader) {}

    config_manager::config_manager(constructor_helper_t&) : 
        override_same_version_(false),
        max_group_number_(8),
        on_log_(config_manager::default_log_writer),
        read_file_handle_(config_manager::default_buffer_loader), 
        read_version_handle_(default_version_loader) {}

    config_manager::~config_manager() {
        is_destroyed_ = true;
    }

    std::shared_ptr<config_manager> config_manager::me() {
        static std::shared_ptr<config_manager> ret;
        if (is_destroyed_) {
            return std::shared_ptr<config_manager>();
        }

        if (ret) {
            return ret;
        }

        static std::once_flag ret_init_flag;
        std::shared_ptr<config_manager>* capture_ret = &ret;
        std::call_once(ret_init_flag, [capture_ret]() {
            constructor_helper_t helper;
            *capture_ret = std::make_shared<config_manager>(helper);
        });

        return ret;
    }

    int config_manager::init() {
        return 0;
    }

    int config_manager::init_new_group() {
        std::string version;
        {
            ::excel::lock::read_lock_holder<::excel::lock::spin_rw_lock> rlh(handle_lock_);
            if (!read_version_handle_) {
                EXCEL_CONFIG_MANAGER_LOGERROR("[EXCEL] config_manager version handle not set");
                return -1;
            }
        
            if (!read_version_handle_(version)) {
                EXCEL_CONFIG_MANAGER_LOGERROR("[EXCEL] config_manager read version failed");
                return -2;
            }
        }

        do {
            // 检查版本号
            ::excel::lock::read_lock_holder<::excel::lock::spin_rw_lock> wlh(config_group_lock_);
            if (config_group_list_.empty()) {
                break;
            }

            if (!config_group_list_.back()) {
                break;
            }

            // 如果强制覆盖新版本号则不用检查版本号
            if (override_same_version_) {
                break;
            }

            // 版本未变化，不需要reload
            // if (0 == ::utils::string::version_compare(version.c_str(), config_group_list_.back()->version.c_str())) {
            if (version == config_group_list_.back()->version) {
                return 0;
            }
        } while (false);

        config_group_ptr_t cfg_group = std::make_shared<config_group_t>();
        if (!cfg_group) {
            EXCEL_CONFIG_MANAGER_LOGERROR("[EXCEL] config_manager malloc config group failed");
            return -3;
        }
        cfg_group->version = version;

        // 加载数据走缓式评估，按需加载
        int ret = 0;
        int res = 0;
        res = cfg_group->objects_cfg.on_inited();
        if (res < 0) {
            EXCEL_CONFIG_MANAGER_LOGERROR("[EXCEL] objects_cfg.on_inited() failed, res: %d", res);
            ret = res;
        } else {
            if (ret >= 0) {
                ret += res;
            }

            EXCEL_CONFIG_MANAGER_LOGINFO("[EXCEL] initialize %s for new config_group success", "objects_cfg");
        }
        res = cfg_group->quests_cfg.on_inited();
        if (res < 0) {
            EXCEL_CONFIG_MANAGER_LOGERROR("[EXCEL] quests_cfg.on_inited() failed, res: %d", res);
            ret = res;
        } else {
            if (ret >= 0) {
                ret += res;
            }

            EXCEL_CONFIG_MANAGER_LOGINFO("[EXCEL] initialize %s for new config_group success", "quests_cfg");
        }
        res = cfg_group->role_upgrade_cfg.on_inited();
        if (res < 0) {
            EXCEL_CONFIG_MANAGER_LOGERROR("[EXCEL] role_upgrade_cfg.on_inited() failed, res: %d", res);
            ret = res;
        } else {
            if (ret >= 0) {
                ret += res;
            }

            EXCEL_CONFIG_MANAGER_LOGINFO("[EXCEL] initialize %s for new config_group success", "role_upgrade_cfg");
        }

        if (ret >= 0) {
            ::excel::lock::write_lock_holder<::excel::lock::spin_rw_lock> wlh(config_group_lock_);
            config_group_list_.push_back(cfg_group);
            if (on_group_created_ && cfg_group) {
                on_group_created_(cfg_group);
            }

            if (config_group_list_.size() > 1 && config_group_list_.size() > max_group_number_) {
                config_group_ptr_t first_group = config_group_list_.front();
                config_group_list_.pop_front();

                if (on_group_destroyed_ && first_group) {
                    on_group_destroyed_(first_group);
                }
            }
        }

        return ret;
    }

    void config_manager::reset() {
        {
            ::excel::lock::write_lock_holder<::excel::lock::spin_rw_lock> wlh(handle_lock_);
            max_group_number_ = 8;
            override_same_version_ = false;

            read_file_handle_ = nullptr;
            read_version_handle_ = nullptr;
            on_group_created_ = nullptr;
            on_group_reload_all_ = nullptr;
            on_group_destroyed_ = nullptr;
        }

        clear();
    }

    void config_manager::clear() {
        ::excel::lock::write_lock_holder<::excel::lock::spin_rw_lock> wlh(config_group_lock_);
        config_group_list_.clear();
    }

    bool config_manager::load_file_data(std::string& write_to, const std::string& file_path) {
        ::excel::lock::read_lock_holder<::excel::lock::spin_rw_lock> rlh(handle_lock_);

        if (!read_file_handle_) {
            EXCEL_CONFIG_MANAGER_LOGERROR("[EXCEL] invalid file data excel.");
            return false;
        }

        return read_file_handle_(write_to, file_path.c_str());
    }

    int config_manager::reload() {
        return init_new_group();
    }

    int config_manager::reload_all(bool del_when_failed) {
        int ret = reload();
        if (0 != ret) {
            return 0;
        }

        config_group_ptr_t cfg_group = get_current_config_group();
        if (!cfg_group) {
            EXCEL_CONFIG_MANAGER_LOGERROR("[EXCEL] mutable config group failed");
            return -2;
        }

        // 触发加载所有表
        int res = 0;
        res = cfg_group->objects_cfg.load_all();
        if (res < 0) {
            EXCEL_CONFIG_MANAGER_LOGERROR("[EXCEL] objects_cfg.load_all() failed, res: %d", res);
            ret = res;
        } else if (ret >= 0) {
            ret += res;
        }
        res = cfg_group->quests_cfg.load_all();
        if (res < 0) {
            EXCEL_CONFIG_MANAGER_LOGERROR("[EXCEL] quests_cfg.load_all() failed, res: %d", res);
            ret = res;
        } else if (ret >= 0) {
            ret += res;
        }
        res = cfg_group->role_upgrade_cfg.load_all();
        if (res < 0) {
            EXCEL_CONFIG_MANAGER_LOGERROR("[EXCEL] role_upgrade_cfg.load_all() failed, res: %d", res);
            ret = res;
        } else if (ret >= 0) {
            ret += res;
        }

        if (del_when_failed && ret < 0) {
            ::excel::lock::write_lock_holder<::excel::lock::spin_rw_lock> wlh(config_group_lock_);
            config_group_list_.pop_back();

            if (on_group_destroyed_ && cfg_group) {
                on_group_destroyed_(cfg_group);
            }
            return ret;
        }

        if (on_group_reload_all_ && cfg_group) {
            on_group_reload_all_(cfg_group);
        }

        return ret;
    }

    config_manager::read_buffer_func_t config_manager::get_buffer_loader() const { 
        ::excel::lock::read_lock_holder<::excel::lock::spin_rw_lock> rlh(handle_lock_);
        
        return read_file_handle_;
    }

    void config_manager::set_buffer_loader(read_buffer_func_t fn) { 
        ::excel::lock::write_lock_holder<::excel::lock::spin_rw_lock> wlh(handle_lock_);

        read_file_handle_ = fn; 
    }

    config_manager::read_version_func_t config_manager::get_version_loader() const { 
        ::excel::lock::read_lock_holder<::excel::lock::spin_rw_lock> rlh(handle_lock_);

        return read_version_handle_; 
    }

    void config_manager::set_version_loader(read_version_func_t fn) { 
        ::excel::lock::write_lock_holder<::excel::lock::spin_rw_lock> wlh(handle_lock_);

        read_version_handle_ = fn; 
    }

    config_manager::config_group_ptr_t config_manager::get_current_config_group() {
        {
            ::excel::lock::read_lock_holder<::excel::lock::spin_rw_lock> rlh(config_group_lock_);
            if (!config_group_list_.empty()) {
                return *config_group_list_.rbegin();
            }
        }

        if (0 == init_new_group()) {
            ::excel::lock::read_lock_holder<::excel::lock::spin_rw_lock> rlh(config_group_lock_);
            if (!config_group_list_.empty()) {
                return *config_group_list_.rbegin();
            }
        }

        return nullptr;
    }

    void config_manager::log(const log_caller_info_t &caller,
#ifdef _MSC_VER
            _In_z_ _Printf_format_string_ const char *fmt, ...
#else
            const char *fmt, ...
#endif
    ) {
        if (is_destroyed_) {
            return;
        }

        const std::shared_ptr<config_manager>& inst = me();

        if (!inst->on_log_) {
            return;
        }

        if (inst->log_buffer_.size() < 4096) {
            inst->log_buffer_.resize(4096, 0); // allocate 4K for format log
        }

        va_list va_args;
        va_start(va_args, fmt);
        int prt_res =
            EXCEL_CONFIG_VSNPRINTF(&inst->log_buffer_[0], inst->log_buffer_.size() - 1, fmt, va_args);
        va_end(va_args);
        if (prt_res >= 0) {
            inst->log_buffer_[prt_res] = 0;
            // call event callback
            inst->on_log_(caller, &inst->log_buffer_[0]);
        }
    }

    bool config_manager::default_buffer_loader(std::string& out, const char* path) {
        return details::get_file_content(out, path);
    }

    bool config_manager::default_version_loader(std::string& out) {
        out = "0";
        return true;
    }

    void config_manager::default_log_writer(const log_caller_info_t& caller, const char* content) {
        if (NULL == content || !*content) {
            return;
        }

        if (NULL != caller.level_name) {
            printf("[%s]", caller.level_name);
        }
        if (NULL != caller.file_path) {
            printf("[%s](%u)", caller.file_path, caller.line_number);
        }
        if (NULL != caller.func_name) {
            printf("[%s]", caller.func_name);
        }
        puts(content);
    }

} // excel