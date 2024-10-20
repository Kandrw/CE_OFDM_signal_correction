#pragma once

#include <complex>

#include "../header.hpp"
#include <output.hpp>


#define CODE_GREY "../data/practice5/code_grey.bin"



enum class TypeModulation{
    BPSK = 1,
    QPSK,
    QAM16,
    QAM64,
    QAM256,
    // QAM1024,
    // COUNT_TYPES_MODULATION
};
typedef std::complex<float> mod_symbol;

// struct mod_symbol{
//     float i;/*real*/
//     float q;/*imag*/
// };
typedef std::vector<std::complex<float>> VecSymbolMod;


struct OFDM_params{
    u_int16_t count_subcarriers;
    mod_symbol pilot;
    u_int16_t step_RS;
    u_int16_t def_interval;
    u_int16_t cyclic_prefix;
    float power;
};

struct ParamsPhy{
    TypeModulation type_modulation;
    OFDM_params param_ofdm;
};

struct OFDM_symbol {
    u_int16_t size = 0;
    std::vector<VecSymbolMod> symbol;
};

struct slot_ofdms {
    VecSymbolMod PSS;
    OFDM_symbol ofdms;
};

typedef std::vector<slot_ofdms> VecSlotsOFDM;


struct bit_sequence{
    // u_int64_t size = 0;
    int size = 0;
    
    u_char *buffer = nullptr;

};


struct qam_sequence{
    u_int16_t size;
    mod_symbol *buffer;
};


void print_VecSymbolMod( VecSymbolMod &vec, int log_level = LOG_DATA);


// struct out_modulation {
//     VecSymbolMod samples;
    
// };

// mod_symbol

VecSymbolMod operator * (const VecSymbolMod &A, const VecSymbolMod &B);
VecSymbolMod operator + (const VecSymbolMod &A, const VecSymbolMod &B);
VecSymbolMod operator * (const VecSymbolMod &A, const std::vector<float> &B);
VecSymbolMod operator / (const VecSymbolMod &A, const VecSymbolMod &B);
VecSymbolMod operator / (const VecSymbolMod &A, const std::vector<float> &B);
// #endif /*TYPES_HPP*/