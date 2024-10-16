#pragma once

#include "types.hpp"



OFDM_symbol OFDM_modulator(VecSymbolMod samples, OFDM_params &param);
VecSymbolMod OFDM_demodulator(OFDM_symbol samples, OFDM_params &param);






