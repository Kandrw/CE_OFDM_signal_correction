#include "types.hpp"

#include <complex.h>
#if 1
VecSymbolMod operator * (const VecSymbolMod &A, const VecSymbolMod &B) {
    VecSymbolMod C;
    if(A.size() != B.size()) {
        print_log(CONSOLE, "Error: The sizes are not equal\n");
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
        print_log(CONSOLE, "Error: The sizes are not equal\n");
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
        print_log(CONSOLE, "Error: The sizes are not equal\n");
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
        print_log(CONSOLE, "Error: The sizes are not equal\n");
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
        print_log(CONSOLE, "Error: The sizes are not equal\n");
        exit(-1);
    }
    for(int i = 0; i < (int)A.size(); ++i) {
        mod_symbol B_c = mod_symbol(B[i], 0);
        C.push_back(A[i] / B_c);
    }
    return C;
}

void print_VecSymbolMod( VecSymbolMod &vec, int log_level) {
    for(int i = 0; i < (int)vec.size(); ++i) {
        if(i != 0 && i % 8 == 0) {
            print_log(log_level, "\n");
        }
        print_log(log_level, "%f + %fi  ", vec[i].real(), vec[i].imag());
        
    }
    print_log(log_level, "\n");
}

#endif