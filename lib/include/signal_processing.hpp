#pragma once
#include <vector>
#include <complex_container.hpp>


#define LENGTH_PSS 63

namespace DIGITAL_SIGNAL_PROCESSING {

    enum class TypeModulation{
        NONE,
        BPSK = 1,
        QPSK,
        QAM16,
        QAM64,
        QAM256,
        // COUNT_TYPES_MODULATION
    };
    struct OFDM_params{
        u_int16_t count_subcarriers;
        mod_symbol pilot;
        u_int16_t step_RS;
        u_int16_t def_interval;
        u_int16_t cyclic_prefix;
        float power;
        u_int8_t count_ofdm_in_slot;
    };
    typedef std::vector<VecSymbolMod> OFDM_symbol;
    struct slot_ofdms {
        VecSymbolMod PSS;
        OFDM_symbol ofdms;

    };
    typedef std::vector<slot_ofdms> VecSlotsOFDM;
    struct bit_sequence{
        int size = 0;
        unsigned char *buffer = nullptr;
    };

    struct ParamsPhy {
        TypeModulation type_modulation;
        OFDM_params param_ofdm;
    };

    struct ParamsTRX {
        TypeModulation type_modulation;
        OFDM_params param_ofdm;
    };
    
    VecSymbolMod modulation_mapper(bit_sequence bits, TypeModulation m);
    bit_sequence *demodulation_mapper(VecSymbolMod &samples, TypeModulation m);
    TypeModulation string_to_TypeModulation(const std::string &tm);



    OFDM_symbol OFDM_modulator(VecSymbolMod samples, OFDM_params &param);

    VecSymbolMod OFDM_demodulator(OFDM_symbol samples, OFDM_params &param, bool found_cprefix = true);
    VecSymbolMod OFDM_convertion_one_thread(OFDM_symbol &ofdm);
    OFDM_symbol samples_join_OFDM(VecSymbolMod sample, int subs, int size);
    VecSymbolMod slots_OFDM_convertion_one_thread(VecSlotsOFDM &slots);

    void addPower(VecSymbolMod &sample, float p);
    void addPowerOFDM(OFDM_symbol &ofdms, float power);
    void addPowerSlots(VecSlotsOFDM &slots, float power);


    VecSlotsOFDM create_slots(const OFDM_symbol &ofdms,
        const OFDM_params &param_ofdm);


    int write_OFDM_slots(const char *filename, VecSlotsOFDM &slots, size_t count_write);
    int write_OFDMs(const char *filename, const OFDM_symbol &ofdms, 
        size_t count_write);

    VecSymbolMod generateZadoffChuSequence(int cellId, int N);


    int evaluation_cfo(OFDM_symbol &ofdms, const OFDM_params &param_ofdm);


};




