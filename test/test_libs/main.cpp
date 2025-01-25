#include <output.hpp>
#include <complex_container.hpp>
#include <signal_processing.hpp>
#include <configure.hpp>

#include <vector>
#include <complex>
#include <stdexcept>


VecSymbolMod divide_vectors(const VecSymbolMod& v1, const VecSymbolMod& v2) {
    if (v1.size() != v2.size()) {
        throw std::invalid_argument("Vectors must be of the same size");
    }

    VecSymbolMod result(v1.size());
    for (size_t i = 0; i < v1.size(); ++i) {
        result[i] = v1[i] / v2[i]; // Деление комплексных чисел
    }
    return result;
}
#if 0
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
#endif

int main() {
    init_log("../log.log");

    VecSymbolMod rt(100);
    for(int i = 0; i < 10; ++i) {
        VecSymbolMod &prt = rt;
        VecSymbolMod rt2 = rt2 / rt2;
        VecSymbolMod rt3 = prt / rt2;
    }
    DIGITAL_SIGNAL_PROCESSING::addPower(rt, 100);
    printf("size = %d\n", rt.size());

    deinit_log();

    return 0;
}





