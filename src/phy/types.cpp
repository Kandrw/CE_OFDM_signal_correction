#include "types.hpp"

#include <complex.h>

VecSymbolMod operator * (const VecSymbolMod &A, const VecSymbolMod &B) {
    VecSymbolMod C;
    if(A.size() != B.size()) {
        print_log(CONSOLE, "Error: The sizes are not equal\n");
        exit(-1);
    }
    for(int i = 0; i < (int)A.size(); ++i) {
        
        mod_symbol s = {(A[i].i * B[i].i) - (A[i].q * B[i].q),
                        (A[i].q * B[i].i) + (A[i].i * B[i].q)
        };
        C.push_back(s);
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
        
        mod_symbol s = {A[i].i + B[i].i,
                        A[i].q + B[i].q };
        C.push_back(s);
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
        
        mod_symbol s = {A[i].i * B[i], A[i].q * B[i]};
        C.push_back(s);
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
        
        float r = ((A[i].i*B[i].i) + (A[i].q*B[i].q)) /\
            (B[i].i*B[i].i + B[i].q*B[i].q);
        float im = ((A[i].q*B[i].i) - (A[i].i*B[i].q)) /\
            (B[i].i*B[i].i + B[i].q*B[i].q);
        
        mod_symbol s = {r, im
        };
        C.push_back(s);
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
        std::complex<float> a(A[i].i, A[i].q);
        std::complex<float> b(B[i], 0.0f);
        std::complex<float> c = a / b;
        mod_symbol s = {c.real(), c.imag()};
        C.push_back(s);
    }
    return C;
}