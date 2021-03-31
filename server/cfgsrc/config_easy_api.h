﻿
// Copyright 2021 xresloader. All rights reserved.
// Generated by xres-code-generator, please don't edit it
//
#ifndef CONFIG_EXCEL_EASY_API_H
#define CONFIG_EXCEL_EASY_API_H

#pragma once

#include "config_manager.h"

namespace excel {
    // ======================================== objects_cfg ========================================
    const excel::config_set_objects_cfg::id_container_type& get_objects_cfg_all_of_id();
    excel::config_set_objects_cfg::id_value_type get_objects_cfg_by_id(uint32_t Id);
    const excel::config_set_objects_cfg::type_container_type& get_objects_cfg_all_of_type();
    const excel::config_set_objects_cfg::type_value_type* get_objects_cfg_by_type(uint32_t Type);
    excel::config_set_objects_cfg::item_ptr_type get_objects_cfg_by_type(uint32_t Type, size_t idx);
    // ======================================== quests_cfg ========================================
    const excel::config_set_quests_cfg::id_container_type& get_quests_cfg_all_of_id();
    excel::config_set_quests_cfg::id_value_type get_quests_cfg_by_id(uint32_t Id);
    const excel::config_set_quests_cfg::type_container_type& get_quests_cfg_all_of_type();
    const excel::config_set_quests_cfg::type_value_type* get_quests_cfg_by_type(uint32_t Type);
    excel::config_set_quests_cfg::item_ptr_type get_quests_cfg_by_type(uint32_t Type, size_t idx);
    // ======================================== role_upgrade_cfg ========================================
    const excel::config_set_role_upgrade_cfg::id_container_type& get_role_upgrade_cfg_all_of_id();
    const excel::config_set_role_upgrade_cfg::id_value_type* get_role_upgrade_cfg_by_id(uint32_t Id);
    excel::config_set_role_upgrade_cfg::item_ptr_type get_role_upgrade_cfg_by_id(uint32_t Id, size_t idx);
    const excel::config_set_role_upgrade_cfg::id_level_container_type& get_role_upgrade_cfg_all_of_id_level();
    excel::config_set_role_upgrade_cfg::id_level_value_type get_role_upgrade_cfg_by_id_level(uint32_t Id, uint32_t Level);
} // excel

#endif