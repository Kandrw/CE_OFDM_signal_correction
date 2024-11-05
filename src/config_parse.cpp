#include "config_parse.hpp"

#include <cstdlib>
#include <fstream>
#include <map>

#define FAILED exit(-1)

void print_input_param() {
    printf(
        "ARGV: "
        "<ip> <filename data> <target>"
        "\n"
    );
}

void read_ofdm_parameters(std::ifstream &file, OFDM_params &ofdm_params) {
    std::string buffer;
    int b;
    while(file >> buffer && buffer != "}") {
        if(buffer == "count_subcarriers:") {
            file >> ofdm_params.count_subcarriers;
        } else if(buffer == "pilot:") {
            float real, imag;
            file >> buffer; real = std::stof(buffer);
            file >> buffer; imag = std::stof(buffer);
            ofdm_params.pilot = mod_symbol(real, imag);
        } else if(buffer == "step_RS:") {
            file >> buffer;
            ofdm_params.step_RS = std::stoi(buffer);
        } else if(buffer == "def_interval:") {
            file >> buffer;
            ofdm_params.def_interval = std::stoi(buffer);
        } else if(buffer == "cyclic_prefix:") {
            file >> buffer;
            ofdm_params.cyclic_prefix = std::stoi(buffer);
        } else if(buffer == "power:") {
            file >> buffer;
            ofdm_params.power = std::stof(buffer);
        }
    }
}

typedef std::map<std::string, TypeModulation> map_TypeModulation;

map_TypeModulation map_type_mod = {
    {"BPSK", TypeModulation::BPSK},
    {"QPSK", TypeModulation::QPSK},
    {"QAM16", TypeModulation::QAM16},
    {"QAM64", TypeModulation::QAM64},
    {"QAM256", TypeModulation::QAM256},
    
};

static TypeModulation string_to_TypeModulation(const std::string &tm) {
    auto t = map_type_mod.find(tm);
    if(t != map_type_mod.end()) {
        return t->second;
    }
    printf("Error: invalid config: no type modulation - %s\n", tm.c_str());
    FAILED;
    return TypeModulation::BPSK;
}

config_program configure(const char *file_conf) {

    std::ifstream file(file_conf);
    config_program param; 
    if(!file.is_open()) {
        printf("Error open: %s\n", file_conf);
        return param;
    }
    std::string buffer;
    while(file >> buffer) {
        if(buffer == "log_file:") {
            file >> param.file_log;
            std::cout<<param.file_log<<"\n";
        } else if(buffer == "address:") {
            file >> param.address;
        } else if(buffer == "type_modulation:") {
            file >> buffer;
            param.type_modulation = string_to_TypeModulation(buffer);
        } else if(buffer == "ofdm_parameters:") {
            read_ofdm_parameters(file, param.ofdm_params);
        }
    }
    
    file.close();
    printf("[%s:%d] loaded configure\n", __func__, __LINE__);
    return param;
}


void print_configure(const config_program &cfg) {
    std::cout<<"log_file:" << cfg.file_log << "\n";
    std::cout<<"address:" << cfg.address << "\n";
    std::cout<<"type_modulation:" << static_cast<int>(cfg.type_modulation) << "\n";
    const OFDM_params &pofdm = cfg.ofdm_params;
    std::cout<<"ofdm_parameters:" << "\n";
    std::cout<<"\tcount_subcarriers:" << pofdm.count_subcarriers << "\n";
    // std::cout<<"\tpilot:" << pofdm.pilot.real() <<" "<< pofdm.pilot.imag() << "\n";
    std::cout<<"\tpilot:" << pofdm.pilot << "\n";
    std::cout<<"\tstep_RS:" << pofdm.step_RS << "\n";
    std::cout<<"\tdef_interval:" << pofdm.def_interval << "\n";
    std::cout<<"\tcyclic_prefix:" << pofdm.cyclic_prefix << "\n";
    std::cout<<"\tpower:" << pofdm.power << "\n";
}

/*Добавить проверку корректности и присутствия конфигурации*/






