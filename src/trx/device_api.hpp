#pragma once


#include <cstddef>

#include "types_trx.hpp"

#include <signal_processing.hpp>

namespace DEVICE_PHY {
    
    using namespace DIGITAL_SIGNAL_PROCESSING;





    class DeviceTRX{

    public:

        static int initialization(config_device &cfg_param);
        static int deinitialization();
        static int send_samples(const void *samples, size_t size);
        static int while_send_samples(void *samples, size_t size);
        static int while_send_samples(VecSlotsOFDM &slots);
        static int recv_samples(void *samples, size_t size);
        static int recv_samples(VecSymbolMod &samples, size_t size);
    private:

    private:
        static unsigned int samples_count_rx;
        static unsigned int samples_count_tx;
    };



};





