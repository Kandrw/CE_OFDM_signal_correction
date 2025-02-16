#include "ofdm.hpp"
#include "types_trx.hpp"

#include <cstring>

#include <output.hpp>
#include <signal_processing.hpp>

#include "receiver_ofdm.hpp"


using namespace DIGITAL_SIGNAL_PROCESSING;

enum class STATE_RECV_OFDM {
    FIND_PSS,
    FIND_OFDM,
    EXIT,
};

struct status_slot{
    slot_ofdms slot;
    bool cfo_correct = false;
    bool full = true;
};
typedef std::vector<status_slot> vec_status_slot;

static int decode_buffer_rx(const VecSymbolMod &samples, 
    const OFDM_params &param_ofdm, vec_status_slot &slots_rx);

#define DEBUG_CSTM
#define DEBUG_DUMP_DATA

VecSymbolMod convert_msg_to_samples(context &ctx_dev, const u_char *data, int size) {
    DIGITAL_SIGNAL_PROCESSING::TypeModulation mod =
        DIGITAL_SIGNAL_PROCESSING::string_to_TypeModulation(
            ctx_dev.cfg["type_modulation"].as<std::string>());
    bit_sequence raw_data;
    raw_data.buffer = (unsigned char*)data;
    raw_data.size = size;
    VecSymbolMod samples = modulation_mapper(raw_data, mod);
    OFDM_symbol ofdms = OFDM_modulator(samples, ctx_dev.ofdm_param);
    if(ofdms.size() % ctx_dev.ofdm_param.count_ofdm_in_slot != 0) {
        DEBUG_LINE
        OFDM_symbol f = create_zero_ofdms(ctx_dev.ofdm_param.count_ofdm_in_slot -
            (ofdms.size() % ctx_dev.ofdm_param.count_ofdm_in_slot), ctx_dev.ofdm_param);
        DEBUG_LINE
        ofdms.insert(ofdms.end(), f.begin(), f.end());
        DEBUG_LINE
    }
    VecSlotsOFDM slots = create_slots(ofdms, ctx_dev.ofdm_param);
#ifdef DEBUG_DUMP_DATA
    // write_OFDMs("../data/dump_data/slots_tx.bin",
    //         slots[0].ofdms, slots[0].ofdms.size());
    write_OFDM_slots("../data/dump_data/slots_tx_2.bin", slots, slots.size());
#endif
    print_log(CONSOLE, "tx slot: %d, s[0] - %d\n",
        slots.size(), slots[0].ofdms.size());
    addPowerSlots(slots, ctx_dev.ofdm_param.power);
    samples = slots_OFDM_convertion_one_thread(slots);
    return samples;
}



int convert_samples_to_msg(context &ctx_dev, const VecSymbolMod &samples,
    u_char *data, int size) {
    vec_status_slot slots_rx; /*pss is not filled*/
    clock_t t1 = 0, t2 = 0;
    t1 = clock();
    decode_buffer_rx(samples, ctx_dev.ofdm_param, slots_rx);

#if 0
    {
        VecSymbolMod s1 = samples;
        OFDM_symbol ofdms_rx;
        int result = receiver_OFDM(s1, ctx_dev.ofdm_param, ofdms_rx);
    }
#endif


    t1 = clock() - t1;
    // print_log(CONSOLE, "End proc buffer\n");
#ifdef DEBUG_DUMP_DATA
    if(slots_rx.size() > 0) {

        #if 1
            print_log(LOG, "[%s:%d] write size: %d\n",
                __func__, __LINE__, samples.size() * 2 * 4);
            write_file_bin_data("../data/dump_data/rx_sample.bin", 
                (void*)&samples[0], samples.size() * 2 * 4);
            DEBUG_LINE
        #endif
        
        write_OFDMs("../data/dump_data/slots_rx.bin",
            slots_rx[0].slot.ofdms, slots_rx[0].slot.ofdms.size());
    }
    VecSymbolMod demod_ofdm;
#endif
    int shift_data = 0;
    if(slots_rx.size() > 0) {
#ifdef DEBUG_CSTM
        print_log(LOG, "[%s:%d] slots_rx.size() = %d, slots_rx[i].slot.ofdms - %d\n",
        __func__, __LINE__, slots_rx.size(), slots_rx[0].slot.ofdms.size());
#endif
        t2 = clock();
        for(int i = 0; i < slots_rx.size(); ++i) {
            if(slots_rx[i].full && slots_rx[i].cfo_correct) {
                VecSymbolMod rx_sample = OFDM_demodulator(
                        slots_rx[i].slot.ofdms, ctx_dev.ofdm_param, false);
#ifdef DEBUG_DUMP_DATA
                demod_ofdm.insert(demod_ofdm.end(),
                    rx_sample.begin(), rx_sample.end());

#endif
                bit_sequence *read_data = demodulation_mapper(rx_sample,
                    TypeModulation::QPSK);

                if(!read_data) {
                    print_log(CONSOLE, "[%s:%d] Error decode\n",
                    __func__, __LINE__);
                    continue;
                }
                memcpy(data + shift_data, read_data->buffer, read_data->size);
                shift_data += read_data->size;
                char *msg = (char*)read_data->buffer;
#ifdef DEBUG_CSTM
                // print_log(LOG, "GET MESSAGE: %s, size: %d\n", msg, read_data->size);
                // for(int k = 6; k < 20; ++k) {
                //     print_log(LOG, "%d - %c | ",
                //         read_data->buffer[k], read_data->buffer[k]);
                // }print_log(LOG, "\n");
#endif

            } else {
                print_log(LOG, "[%s:%d] the message is corrupted\n",
                    __func__, __LINE__);
            }

        }
#ifdef DEBUG_DUMP_DATA
        if(slots_rx.size() > 0) {
            write_file_bin_data("../data/dump_data/demod_ofdm.bin",
                (void*)&demod_ofdm[0], sizeof(mod_symbol) * (demod_ofdm.size()));
        }

#endif
        t2 = clock() - t2;
        print_log(LOG, "[%s:%d] shift size = %d\n",
                    __func__, __LINE__, shift_data);
        return shift_data;
    }
#ifdef DEBUG_CSTM
    print_log(LOG_DATA, "[%s:%d] decode_buffer_rx: %f, demod: %f\n",
                    __func__, __LINE__, (double) (t1) / CLOCKS_PER_SEC,
                    (double) (t2) / CLOCKS_PER_SEC);
#endif
    return 0;
}

// #define DEBUG_DECODE_OFDM
// #include <thread>
// #include <future>

static std::vector<float> correlation(const VecSymbolMod& y1, const VecSymbolMod& y2) {
    size_t len1 = y1.size();
    size_t len2 = y2.size();
    size_t maxShift = len1 - len2 + 1;  // Максимальный сдвиг для корреляции
    static std::vector<float> result(maxShift, 0.0);
    // Предварительные вычисления нормировочных коэффициентов
    float normY2 = 0.0;
    for (const auto& v : y2) {
        normY2 += std::norm(v);
    }
    normY2 = std::sqrt(normY2);
    static std::complex<float> sum;
    // Цикл по сдвигам
    for (size_t shift = 0; shift < maxShift; ++shift) {
        // std::complex<float> sum(0.0, 0.0);
        sum = {0.0, 0.0};
        float normY1 = 0.0;
        // Вычисление корреляции на текущем сдвиге
        for (size_t i = 0; i < len2; ++i) {
            sum += y1[i + shift] * std::conj(y2[i]);
            normY1 += std::norm(y1[i + shift]);
        }
        // Нормирование
        float normFactor = std::sqrt(normY1) * normY2;
        result[shift] = std::abs(sum) / normFactor;
    }
    return result;
}


/*
    decode_buffer_rx:
        find PSS
        collect OFDM symbol
        CFO
*/
static int decode_buffer_rx(const VecSymbolMod &samples, 
    const OFDM_params &param_ofdm, vec_status_slot &slots_rx) 
{
    
    static float activate_find_pss = 0.7;
    static const int count_send_ofdm = param_ofdm.count_ofdm_in_slot;
    int pos_data = -1;
    static VecSymbolMod pss = generateZadoffChuSequence(0, LENGTH_PSS);

    clock_t t1 = clock();
    std::vector<float> corr_array = correlation(samples, pss);
    t1 = clock() - t1;
#if 0
    write_file_bin_data(
            "../data/corr_array_test.bin", 
            (void*)&corr_array[0], 
            sizeof(float) * corr_array.size());
#endif
#ifdef DEBUG_DECODE_OFDM
    print_log(LOG_DATA, "samples.size() = %d\n", samples.size() );
#endif
    bool run = true;
    STATE_RECV_OFDM state = STATE_RECV_OFDM::FIND_PSS;
    int pos_iter = 0;
    int status = 0;
    int iter_pos_ofdm;
    int slice;
    int count_pss = 0;
    status_slot slot_tmp;
    bool add_slot = false;
    clock_t t2 = clock();
    while (run) {
#ifdef DEBUG_DECODE_OFDM
        print_log(LOG_DATA, "[%s:%d] \t\tstate: %d\n", __func__, __LINE__, static_cast<int>(state));
#endif
        switch (state) {
        case STATE_RECV_OFDM::FIND_PSS:
            if(add_slot) {
                slot_tmp.full = true;
                slots_rx.push_back(slot_tmp);
                slot_tmp.slot.ofdms.clear();
                // slot_tmp.slot.pss.clear();
                add_slot = false;
            }
            pos_data = -1;
#ifdef DEBUG_DECODE_OFDM
            print_log(LOG_DATA, "[%s:%d] pos_iter - %d\n", __func__, __LINE__, pos_iter);
#endif
            for(int i = pos_iter; i < corr_array.size(); ++i) {
                if(corr_array[i] > activate_find_pss) {
                    pos_data = i + pss.size();
                    break;
                }
            }
#ifdef DEBUG_DECODE_OFDM
            print_log(LOG_DATA, "[%s:%d] pos_data: %d\n", __func__, __LINE__, pos_data);
#endif
            if(pos_data == -1) {
                state = STATE_RECV_OFDM::EXIT;
            } else {
                state = STATE_RECV_OFDM::FIND_OFDM;
                count_pss++;
            }
            break;
        case STATE_RECV_OFDM::FIND_OFDM:
            iter_pos_ofdm = pos_data;
            slice = -1;
            state = STATE_RECV_OFDM::FIND_PSS;
            for(int i = 0; i < count_send_ofdm; ++i) {
                iter_pos_ofdm += param_ofdm.cyclic_prefix;
                slice = iter_pos_ofdm + param_ofdm.count_subcarriers;
                if(iter_pos_ofdm > samples.size() || slice > samples.size()) {
                    print_log(LOG_DATA, "Out of range buffer: slice [%d:%d], buffer size - %d\n",
                        iter_pos_ofdm, slice, samples.size());
                    state = STATE_RECV_OFDM::EXIT;
                    if(slot_tmp.slot.ofdms.size() < count_send_ofdm) {
                        print_log(LOG_DATA, "[%s:%d] no fill ofdms: %d\n",
                            __func__, __LINE__, slot_tmp.slot.ofdms.size());
                        slot_tmp.full = false;
                        slots_rx.push_back(slot_tmp);
                    }
                    
                    break;
                }
#ifdef DEBUG_DECODE_OFDM
                print_log(LOG_DATA, "[%s:%d] add ofdm symbol - %d, [%d:%d]\n",
                    __func__, __LINE__, slot_tmp.slot.ofdms.size(), iter_pos_ofdm, slice);
#endif
                VecSymbolMod ofdm_symbol(samples.begin() + iter_pos_ofdm,
                    samples.begin() + slice);
                // evaluation_cfo_one(ofdm_symbol, param_ofdm);
                slot_tmp.slot.ofdms.push_back(ofdm_symbol);
                // ofdms.push_back(
                //     VecSymbolMod(
                //         samples.begin() + iter_pos_ofdm, 
                //         samples.begin() + slice));
                iter_pos_ofdm += param_ofdm.count_subcarriers;
            }
            add_slot = true;
            pos_iter = iter_pos_ofdm; 
            break;
        case STATE_RECV_OFDM::EXIT:
            run = false;
            break;
        default:
            run = false;
            break;
        }
    }
    t2 = clock() - t2;
    clock_t t3 = clock();
    for(int i = 0; i < slots_rx.size(); ++i) {

        if(slots_rx[i].full) {
            // evaluation_cfo(slots_rx[i].slot.ofdms, param_ofdm);
            slots_rx[i].cfo_correct = true;
        } else {
            slots_rx[i].cfo_correct = false;
        }

    }
    t3 = clock() - t3;
    print_log(LOG_DATA, "[%s:%d] corr: %f, find ofdm: %f, cfo: %f\n",
                    __func__, __LINE__, (double) (t1) / CLOCKS_PER_SEC,
                    (double) (t2) / CLOCKS_PER_SEC, (double) (t3) / CLOCKS_PER_SEC);
    return 0;
}












