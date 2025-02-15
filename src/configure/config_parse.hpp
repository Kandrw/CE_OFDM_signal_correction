#pragma once

#include <iostream>

#include <output.hpp>
#include <complex_container.hpp>
#include <signal_processing.hpp>



struct config_program {
    std::string file_log;
    std::string address;
    DIGITAL_SIGNAL_PROCESSING::TypeModulation type_modulation;
    DIGITAL_SIGNAL_PROCESSING::OFDM_params ofdm_params;
};


void print_input_param();
config_program configure(const char *file_conf);
void print_configure(const config_program &cfg);
void write_ofdm_param(const std::ofstream &file, 
                            const DIGITAL_SIGNAL_PROCESSING::OFDM_params &ofdm_params);
void write_config_file(const char *filename, const config_program &param);



















