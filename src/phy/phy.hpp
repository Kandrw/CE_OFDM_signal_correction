#pragma once

#include <vector>

#include "../header.hpp"
#include "types.hpp"
// #include "modulation.hpp"




// int modulation_mapper(struct bit_sequence *data, type_modulation m);


OFDM_symbol generate_frame_phy(bit_sequence &bits, 
    ParamsPhy &param, VecSymbolMod &samples_tx);

// OFDM_symbol generate_frame_phy(bit_sequence &bits, ParamsPhy &param);
bit_sequence *decode_frame_phy(OFDM_symbol &samples, 
    ParamsPhy &param, bool fprefix = true);
   