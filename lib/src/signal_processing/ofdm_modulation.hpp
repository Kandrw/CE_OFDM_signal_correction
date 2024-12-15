#pragma once


// OFDM_symbol OFDM_modulator(VecSymbolMod samples, OFDM_params &param);

// VecSymbolMod OFDM_demodulator(OFDM_symbol samples, OFDM_params &param, bool found_cprefix = true);
// VecSymbolMod OFDM_convertion_one_thread(OFDM_symbol &ofdm);
// OFDM_symbol samples_join_OFDM(VecSymbolMod sample, int subs, int size);
// VecSymbolMod slots_OFDM_convertion_one_thread(VecSlotsOFDM &slots);

// void addPower(VecSymbolMod &sample, float p);
// void addPowerOFDM(OFDM_symbol &ofdms, float power);
// void addPowerSlots(VecSlotsOFDM &slots, float power);


// VecSlotsOFDM create_slots(const OFDM_symbol &ofdms,
//     const OFDM_params &param_ofdm);


// int write_OFDM_slots(const char *filename, VecSlotsOFDM &slots, size_t count_write);
// int write_OFDMs(const char *filename, const OFDM_symbol &ofdms, 
//     size_t count_write);

// VecSymbolMod generateZadoffChuSequence(int cellId, int N);


