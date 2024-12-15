#pragma once

#include <complex_container.hpp>
#include <signal_processing.hpp>

int receiver_OFDM(VecSymbolMod &samples, 
    DIGITAL_SIGNAL_PROCESSING::OFDM_params &param_ofdm, DIGITAL_SIGNAL_PROCESSING::OFDM_symbol &ofdms);

