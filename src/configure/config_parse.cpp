#include "config_parse.hpp"

#include <cstdlib>
#include <fstream>
#include <map>

#include <output.hpp>
#include <complex_container.hpp>
#include <signal_processing.hpp>

#define FAILED exit(-1)



using namespace DIGITAL_SIGNAL_PROCESSING;

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
        } else if(buffer == "count_ofdm_in_slot:") {
            file >> buffer;
            ofdm_params.count_ofdm_in_slot = std::stof(buffer);
        }
    }
}

static std::map<std::string, TypeModulation> map_type_mod = {
    {"BPSK", TypeModulation::BPSK},
    {"QPSK", TypeModulation::QPSK},
    {"QAM16", TypeModulation::QAM16},
    {"QAM64", TypeModulation::QAM64},
    {"QAM256", TypeModulation::QAM256},
};
static std::map<TypeModulation, std::string> map_type_str_mod = {
    {TypeModulation::BPSK, "BPSK"},
    {TypeModulation::QPSK, "QPSK"},
    {TypeModulation::QAM16, "QAM16"},
    {TypeModulation::QAM64, "QAM64"},
    {TypeModulation::QAM256, "QAM256"},
};

static TypeModulation string_to_TypeM(const std::string &tm) {
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
        exit(-1);
        return param;
    }
    std::string buffer;
    while(file >> buffer) {
        if(buffer == "log_file:") {
            file >> param.file_log;
        } else if(buffer == "address:") {
            file >> param.address;
        } else if(buffer == "type_modulation:") {
            file >> buffer;
            param.type_modulation = string_to_TypeM(buffer);
        } else if(buffer == "ofdm_parameters:") {
            read_ofdm_parameters(file, param.ofdm_params);
        }
    }
    
    file.close();
    printf("[%s:%d] loaded configure\n", __func__, __LINE__);
    return param;
}

void write_ofdm_param(std::ofstream &file, 
                            const OFDM_params &ofdm_params) 
{
    /*без проверки*/
    file<<"ofdm_parameters: {\n";
    file<<"count_subcarriers: " << ofdm_params.count_subcarriers<<"\n";
    file<<"pilot: " << ofdm_params.pilot.real() << " " << ofdm_params.pilot.imag() << "\n";
    file<<"step_RS: " << ofdm_params.step_RS << "\n";
    file<<"def_interval: " << ofdm_params.def_interval << "\n";
    file<<"cyclic_prefix: " << ofdm_params.cyclic_prefix << "\n";
    file<<"power: " << ofdm_params.power << "\n";
    file<<"count_ofdm_in_slot: " << ofdm_params.count_ofdm_in_slot << "\n";
    file<<"}\n";
    
}

void write_config_file(const char *filename, const config_program &param) {
    std::ofstream file(filename);
    if(!file.is_open()) {
        print_log(ERROR_OUT, "Error open %s\n", filename);
        return;
    }
    file<<"log_file: "<<param.file_log<<"\n";
    file<<"address: "<<param.address<<"\n";
    auto tm = map_type_str_mod.find(param.type_modulation);
    if(tm != map_type_str_mod.end())
        file<<"type_modulation: "<<tm->second.c_str()<<"\n";
    else {
        print_log(ERROR_OUT, "Error: type modulation\n");
        return;
    }
    write_ofdm_param(file, param.ofdm_params);    
    
    
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






