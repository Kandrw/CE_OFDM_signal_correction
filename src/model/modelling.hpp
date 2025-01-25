#pragma once

#include <complex_container.hpp>
#include <signal_processing.hpp>

// VecSymbolMod add_Gaussian_noise(VecSymbolMod &samples, float dispersion);


// void model_calc_P_SER();
// void model_soft_solutions();

int modelling_signal(char target);
void modelling_channel(VecSymbolMod &samples);
int calc_bit_error(DIGITAL_SIGNAL_PROCESSING::bit_sequence &tx, DIGITAL_SIGNAL_PROCESSING::bit_sequence &rx);

namespace MODEL_COMPONENTS {
    VecSymbolMod generate_noise_by_SNR(int N, float Q_2);
};