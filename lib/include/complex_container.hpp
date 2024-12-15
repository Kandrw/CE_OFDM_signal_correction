#pragma once

#include <complex>
#include <vector>

#define type_complex float


#define MAX_ELM_VEC(vec)\
std::max_element(vec.begin(), vec.end(),\
        [](const mod_symbol& a, const mod_symbol& b) {\
            return std::abs(a) < std::abs(b);\
        });

typedef std::complex<type_complex> mod_symbol;
typedef std::vector<std::complex<type_complex>> VecSymbolMod;

void print_VecSymbolMod(FILE *file, const VecSymbolMod &vec);

VecSymbolMod operator * (const VecSymbolMod &A, const VecSymbolMod &B);
VecSymbolMod operator + (const VecSymbolMod &A, const VecSymbolMod &B);
VecSymbolMod operator * (const VecSymbolMod &A, const std::vector<float> &B);
VecSymbolMod operator / (const VecSymbolMod &A, const VecSymbolMod &B);
VecSymbolMod operator / (const VecSymbolMod &A, const std::vector<float> &B);
VecSymbolMod operator / (const VecSymbolMod &A, const mod_symbol &B);

void operator_div(VecSymbolMod &A, const mod_symbol &B);
void operator_add(VecSymbolMod &A, const VecSymbolMod &B);

VecSymbolMod conj_vec(const VecSymbolMod &A);
