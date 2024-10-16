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
        
        mod_symbol s = {(A[i].real() * B[i].real()) - (A[i].imag() * B[i].imag()),
                        (A[i].imag() * B[i].real()) + (A[i].real() * B[i].imag())
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
        
        mod_symbol s = {A[i].real() + B[i].real(),
                        A[i].imag() + B[i].imag() };
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
        
        mod_symbol s = {A[i].real() * B[i], A[i].imag() * B[i]};
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
        
        float r = ((A[i].real()*B[i].real()) + (A[i].imag()*B[i].imag())) /\
            (B[i].real()*B[i].real() + B[i].imag()*B[i].imag());
        float im = ((A[i].imag()*B[i].real()) - (A[i].real()*B[i].imag())) /\
            (B[i].real()*B[i].real() + B[i].imag()*B[i].imag());
        
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
        std::complex<float> a(A[i].real(), A[i].imag());
        std::complex<float> b(B[i], 0.0f);
        std::complex<float> c = a / b;
        mod_symbol s = {c.real(), c.imag()};
        C.push_back(s);
    }
    return C;
}
#endif