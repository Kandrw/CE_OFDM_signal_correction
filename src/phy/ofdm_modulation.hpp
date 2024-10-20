#pragma once

#include "types.hpp"



OFDM_symbol OFDM_modulator(VecSymbolMod samples, OFDM_params &param);
VecSymbolMod OFDM_demodulator(OFDM_symbol samples, OFDM_params &param);

VecSymbolMod OFDM_convertion_one_thread(OFDM_symbol &ofdm);
OFDM_symbol samples_join_OFDM(VecSymbolMod sample, int subs, int size);
 
void addPowerOFDM(OFDM_symbol ofdms, float power);
VecSlotsOFDM create_slots(OFDM_symbol ofdms);




