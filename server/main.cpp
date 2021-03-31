#include <cstdio>

#include "config_manager.h"
#include "config_easy_api.h"

int main() {
    // 初始化 ....
    excel::config_manager::me()->init();

    // 可选
    // excel::config_manager::me()->set_version_loader([] (std::string& out) {
    //     // 读取版本号然后写出到 out
    //     return true; // 成功返回true，失败返回false
    // });

    // If you want to intergrate file loader to your system(such as UE or Unity), you should provide buffer loader handle
    // excel::config_manager::me()->set_buffer_loader([] (std::string& out, const char* file_path) {
    //     // 读取文件名为file_path的二进制数据然后写出到out
    //     // file_path 即是pb插件 option (xrescode.loader) 中的file_path字段
    //     printf("set_buffer_loader %s %s", out.c_str(), file_path);
    //     return true; // 成功返回true，失败返回false
    // });

    // Set 设置设置保留多少组不同版本的数据
    // excel::config_manager::me()->set_group_number(8);

    // 使用 set_override_same_version(true) 可以强制触发读取，即便版本号没变.
    // excel::config_manager::me()->set_override_same_version(true);

    // 设置日志输出回调，默认会输出到标准输出
    // excel::config_manager::me()->set_on_log([](const log_caller_info_t& caller, const char* content) {
    //    // ...
    // });

    // 还可以设置一些其他的事件回调，详见生成的代码

    // 调用 reload 来执行某个版本的数据加载
    // excel::config_manager::me()->reload_all();
    // excel::config_manager::me()->reload();

    // 然后就可以用config_easy_api或者config_manager的API读取数据了
    auto cfg = excel::get_role_upgrade_cfg_by_id_level(10001, 3); // using the Key-Value index: id_level
    if (cfg) {
        printf("%s\n", cfg->DebugString().c_str());
    }

    // auto itemcfg = excel::get_objects_cfg_by_id(10003);
    // if (itemcfg)
    // {
    //     printf("%s", itemcfg->DebugString().c_str());
    // }

    // auto lst = excel::get_objects_cfg_by_type(11);
    // if (lst)
    // {
    //     for (auto& v : *lst)
    //     {
    //         printf("%s", v->DebugString().c_str());
    //     }
    // }

    auto itemcfg = excel::get_objects_cfg_by_id(10014);
    if (itemcfg)
    {
        // if (itemcfg->has_exp())
        // {
        //     printf("exp %u-%u", itemcfg->id(), itemcfg->exp());
        // }
        // if (itemcfg->has_gold())
        // {
        //     printf("gold %u-%u", itemcfg->id(), itemcfg->gold());
        // }

        // printf("relation.item1: %u\n", itemcfg->relation().item1());
        // printf("relation.item2: %u\n", itemcfg->relation().item2());
        // printf("relation.item3: %u\n", itemcfg->relation().item3());
        // printf("usetime: %u\n", itemcfg->usetime());
        printf("%s\n", itemcfg->DebugString().c_str());
    }

    auto quest = excel::get_quests_cfg_by_id(10009);
    if (quest) printf("quest:%s\n", quest->DebugString().c_str());

    // printf("-------------------------------------------\n");

    // excel::config_manager* mgr = (excel::config_manager*)GetConfigManagerPtr();
    // auto group = mgr->get_current_config_group();
    // itemcfg = group->objects_cfg.get_by_id(10014);
    // if (itemcfg)  printf("%s\n", itemcfg->DebugString().c_str());
    return 0;
}