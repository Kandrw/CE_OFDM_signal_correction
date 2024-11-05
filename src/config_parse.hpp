#pragma once

#include <iostream>

#include "phy/types.hpp"

enum class ARGV_CONSOLE: int {
    ARGV_IP_DEVICE = 1,
    ARGV_FILE_DATA,
    ARGV_TARGET_PROGRAM,
    ARGV_FILE_CONFIG,
    ARGV_MAX
};


struct config_program {
    std::string file_log;
    std::string address;
    TypeModulation type_modulation;
    OFDM_params ofdm_params;
};


void print_input_param();
config_program configure(const char *file_conf);
void print_configure(const config_program &cfg);





















