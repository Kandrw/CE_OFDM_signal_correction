#pragma once

#include <complex_container.hpp>
#include "components.hpp"


namespace CHANNEL_MODEL {

    int model_channel_init(ATTR_SERVICE::context &cfg_dev);
    int model_channel_deinit(ATTR_SERVICE::context &cfg_dev);
    int read_channel(VecSymbolMod &samples, size_t size);
    int write_channel(const VecSymbolMod &samples, size_t size);
};

int channel_model(int argc, char *argv[]);




