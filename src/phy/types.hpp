#pragma once

#include <complex>

#include "../header.hpp"
#include <output.hpp>


#define CODE_GREY "../data/practice5/code_grey.bin"

#define MAX_ELM_VEC(vec)\
std::max_element(vec.begin(), vec.end(),\
        [](const mod_symbol& a, const mod_symbol& b) {\
            return std::abs(a) < std::abs(b);\
        });




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

typedef std::vector<std::complex<float>> VecSymbolMod;


struct OFDM_params{
    u_int16_t count_subcarriers;
    mod_symbol pilot;
    u_int16_t step_RS;
    u_int16_t def_interval;
    u_int16_t cyclic_prefix;
    float power;
    u_int8_t count_ofdm_in_slot;
};

struct ParamsPhy{
    TypeModulation type_modulation;
    OFDM_params param_ofdm;
};

typedef std::vector<VecSymbolMod> OFDM_symbol;

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


VecSymbolMod operator * (const VecSymbolMod &A, const VecSymbolMod &B);
VecSymbolMod operator + (const VecSymbolMod &A, const VecSymbolMod &B);
VecSymbolMod operator * (const VecSymbolMod &A, const std::vector<float> &B);
VecSymbolMod operator / (const VecSymbolMod &A, const VecSymbolMod &B);
VecSymbolMod operator / (const VecSymbolMod &A, const std::vector<float> &B);
VecSymbolMod operator / (const VecSymbolMod &A, const mod_symbol &B);

void operator_div(VecSymbolMod &A, const mod_symbol &B);
void operator_add(VecSymbolMod &A, const VecSymbolMod &B);

VecSymbolMod conj_vec(const VecSymbolMod &A);
// #endif /*TYPES_HPP*/