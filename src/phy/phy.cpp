#include "phy.hpp"

#include <cstring>
// #include <map>

#include <output.hpp>

#include "modulation.hpp"
#include "ofdm_modulation.hpp"


// #include "../generate_packet/generate_packet.h"

OFDM_symbol generate_frame_phy(bit_sequence &bits, ParamsPhy &param){
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);
    VecSymbolMod samples = modulation_mapper(bits, param.type_modulation);
    // VecSymbolMod ofdm_samples = 
    OFDM_symbol ofdms = OFDM_modulator(samples, param.param_ofdm);
    print_log(LOG, "Create %d ofdm symbol\n", ofdms.size);
     
    
    return ofdms;
}




bit_sequence *decode_frame_phy(OFDM_symbol &samples, ParamsPhy &param){
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);
    VecSymbolMod rx_sample = OFDM_demodulator(samples, param.param_ofdm);
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);

    return demodulation_mapper(rx_sample, param.type_modulation);
    return nullptr;
}





