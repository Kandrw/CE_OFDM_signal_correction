#include "complex_container.hpp"

#include <complex.h>
#include <cstdio>

VecSymbolMod operator * (const VecSymbolMod &A, const VecSymbolMod &B) {
    VecSymbolMod C;
    if(A.size() != B.size()) {
        fprintf(stderr, "Error: The sizes are not equal\n");
        exit(-1);
    }
    for(int i = 0; i < (int)A.size(); ++i) {
        C.push_back(A[i] * B[i]);
    }
    return C;
}

VecSymbolMod operator + (const VecSymbolMod &A, const VecSymbolMod &B) {
    VecSymbolMod C;
    if(A.size() != B.size()) {
        fprintf(stderr, "Error: The sizes are not equal\n");
        exit(-1);
    }
    for(int i = 0; i < (int)A.size(); ++i) {
        C.push_back(A[i] + B[i]);
    }
    return C;
}

VecSymbolMod operator * (const VecSymbolMod &A, const std::vector<float> &B) {
    VecSymbolMod C;
    if(A.size() != B.size()) {
        fprintf(stderr, "Error: The sizes are not equal\n");
        exit(-1);
    }
    for(int i = 0; i < (int)A.size(); ++i) {
        C.push_back(A[i] * B[i]);
    }
    return C;
}

VecSymbolMod operator / (const VecSymbolMod &A, const VecSymbolMod &B) {
    VecSymbolMod C;
    if(A.size() != B.size()) {
        fprintf(stderr, "Error: The sizes are not equal\n");
        exit(-1);
    }
    for(int i = 0; i < (int)A.size(); ++i) {
        C.push_back(A[i] / B[i]);
    }
    return C;
}

VecSymbolMod operator / (const VecSymbolMod &A, const std::vector<float> &B) {
    VecSymbolMod C;
    if(A.size() != B.size()) {
        fprintf(stderr, "Error: The sizes are not equal\n");
        exit(-1);
    }
    for(int i = 0; i < (int)A.size(); ++i) {
        mod_symbol B_c = mod_symbol(B[i], 0);
        C.push_back(A[i] / B_c);
    }
    return C;
}

VecSymbolMod operator / (const VecSymbolMod &A, const mod_symbol &B) {
    VecSymbolMod C;
    for(int i = 0; i < (int)A.size(); ++i) {
        C.push_back(A[i] / B);
    }
    return C;
}

void operator_div(VecSymbolMod &A, const mod_symbol &B) {
    for(int i = 0; i < (int)A.size(); ++i) {
        A[i] = A[i] / B;
    }
}

void operator_add(VecSymbolMod &A, const VecSymbolMod &B) {
    VecSymbolMod C;
    if(A.size() != B.size()) {
        fprintf(stderr, "Error: The sizes are not equal\n");
        exit(-1);
    }
    for(int i = 0; i < (int)A.size(); ++i) {
        A[i] = A[i] / B[i];
    }
}

VecSymbolMod conj_vec(const VecSymbolMod &A) {
    VecSymbolMod B;
    for(int i = 0; i < A.size(); ++i) {
        B.push_back(std::conj(A[i]));
    }
    return B;
}

void print_VecSymbolMod(FILE *file, const VecSymbolMod &vec) {
    for(int i = 0; i < (int)vec.size(); ++i) {
        if(i != 0 && i % 8 == 0) {
            fprintf(file, "\n");
        }
        fprintf(file, "%f + %fi  ", vec[i].real(), vec[i].imag());
    }
    fprintf(file, "\n");
}

