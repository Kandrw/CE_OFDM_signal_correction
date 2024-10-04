#include "types.hpp"



VecSymbolMod operator * (const VecSymbolMod &A, const VecSymbolMod &B) {
    VecSymbolMod C;
    if(A.size() != B.size()) {
        print_log(CONSOLE, "Error: The sizes are not equal\n");
        exit(-1);
    }
    for(int i = 0; i < (int)A.size(); ++i) {
        
        mod_symbol s = {(A[i].I * B[i].I) - (A[i].Q * B[i].Q),
                        (A[i].Q * B[i].I) + (A[i].I * B[i].Q)
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
        
        mod_symbol s = {A[i].I + B[i].I,
                        A[i].Q + B[i].Q };
        C.push_back(s);
    }
    return C;
}




