#pragma once
// #ifndef TYPES_HPP
// #define TYPES_HPP

#include "../header.hpp"
#include <output.hpp>
// #include "../output/output.hpp"

#define CODE_GREY "data/practic5/code_grey.bin"

enum class TypeModulation{
    BPSK = 1,
    QPSK,
    QAM16,
    QAM64,
    QAM256,
    // QAM1024,
    // COUNT_TYPES_MODULATION
};


struct OFDM_params{

};

struct ParamsPhy{
    TypeModulation type_modulation;
    OFDM_params param_ofdm;
};

struct bit_sequence{
    int size = 0;
    u_char *buffer = nullptr;

};

struct mod_symbol{
    float I;/*real*/
    float Q;/*imag*/
};
struct qam_sequence{
    int size;
    mod_symbol *buffer;
};

typedef std::vector<mod_symbol> VecSymbolMod;

// struct out_modulation {
//     VecSymbolMod samples;
    
// };

// mod_symbol

VecSymbolMod operator * (const VecSymbolMod &A, const VecSymbolMod &B);
VecSymbolMod operator + (const VecSymbolMod &A, const VecSymbolMod &B);

// #endif /*TYPES_HPP*/