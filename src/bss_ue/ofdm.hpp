#pragma once

VecSymbolMod convert_msg_to_samples(
    ATTR_SERVICE::context &ctx_dev, const u_char *data, int size);
int convert_samples_to_msg(ATTR_SERVICE::context &ctx_dev, const VecSymbolMod &samples,
    u_char *data, int size);

