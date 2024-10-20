#pragma once

#include "../phy/types.hpp"

// VecSymbolMod add_Gaussian_noise(VecSymbolMod &samples, float dispersion);


// void model_calc_P_SER();
// void model_soft_solutions();

int modelling_signal(char target);
void modelling_channel(VecSymbolMod &samples);
int calc_bit_error(bit_sequence &tx, bit_sequence &rx);