#include "phy.hpp"

#include <cstring>
// #include <map>

#include <output.hpp>

#include "modulation.hpp"

// #include "../generate_packet/generate_packet.h"

VecSymbolMod generate_frame_phy(bit_sequence &bits, ParamsPhy &param){
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);
    return modulation_mapper(bits, param.type_modulation);
}




bit_sequence *decode_frame_phy(VecSymbolMod &samples, ParamsPhy &param){
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);
    return demodulation_mapper(samples, param.type_modulation);
}





