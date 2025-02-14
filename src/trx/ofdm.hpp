#pragma once

#include <complex_container.hpp>
#include "types_trx.hpp"


VecSymbolMod convert_msg_to_samples(
    context &ctx_dev, const u_char *data, int size);
int convert_samples_to_msg(context &ctx_dev, const VecSymbolMod &samples,
    u_char *data, int size);

