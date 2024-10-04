#pragma once

#include <vector>

#include "../header.hpp"
#include "types.hpp"
// #include "modulation.hpp"




// int modulation_mapper(struct bit_sequence *data, type_modulation m);




VecSymbolMod generate_frame_phy(bit_sequence &bits, ParamsPhy &param);
bit_sequence *decode_frame_phy(VecSymbolMod &samples, ParamsPhy &param);
