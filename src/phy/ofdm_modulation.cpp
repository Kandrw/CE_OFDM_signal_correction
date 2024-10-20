#include "ofdm_modulation.hpp"

#include <fftw3.h>


#define VALUE_DEF_INTERVAL 0.f 
#define NO_DATA 0.f

#define FAILED exit(-1)

#define PI 3.141592
bool check_correct_param(OFDM_params &param) {
    u_int16_t sub_no_def = param.count_subcarriers - param.def_interval;
    if(param.count_subcarriers == 0) {
        print_log(ERROR_OUT, "Incorrect param: count_subcarriers = 0\n");
        return false;
    }
    if(param.step_RS == 0) {
        print_log(ERROR_OUT, "Incorrect param: step_RS = 0\n");
        return false;
    }
    if(param.def_interval == 0) {
        print_log(ERROR_OUT, "Incorrect param: def_interval = 0\n");
        return false;
    }
    // if(param.count_subcarriers < ) {
    //     print_log(ERROR_OUT, "Incorrect param: count_subcarriers = 0\n");
    //     return false;
    // }
}



#define TO_INT(x) (static_cast<int>(x))

enum class OFDM_STATE{
    EXIT,
    // ADD_PILOT,
    // ADD_DATA,
    FILL_SYMBOL,
    ADD_DEF_INTERVAL,
    ADD_OFDM_SYMBOL_TO_LIST_AND_CLEAR,
    NEXT_OFDM_SYMBOL,
};

#define BEGIN 0
#define END 1
#define PRINT_DEBUG_LINE(str) print_log(LOG, "%s", str)


void add_cyclic_prefix(VecSymbolMod &sample, int size_prefix) {
    VecSymbolMod prefix(sample.end() - size_prefix, sample.end());
    sample.insert(sample.begin(), prefix.begin(), prefix.end());
}
void delete_cyclic_prefix(VecSymbolMod &sample, int size_prefix) {
    // VecSymbolMod prefix(sample.begin() + size_prefix, sample.end());
    sample.erase(sample.begin(), sample.begin() + size_prefix);
}

VecSymbolMod OFDM_convertion_one_thread(OFDM_symbol &ofdm) {
    VecSymbolMod one;
    for(int i = 0; i < ofdm.size; ++i) {
        one.insert(one.end(), ofdm.symbol[i].begin(), ofdm.symbol[i].end());
    }
    return one;
}

OFDM_symbol samples_join_OFDM(VecSymbolMod sample, int subs, int size) {
    OFDM_symbol ofdms;
    for(int i = 0; i < sample.size(), i < size; i += subs) {
        ofdms.size++;
        VecSymbolMod s(sample.begin() + i, sample.begin() + i + subs);
        ofdms.symbol.push_back(s);
    }
    return ofdms;
}

void addPower(VecSymbolMod sample, float p) {
    mod_symbol power = mod_symbol(p, p);
    for(int i = 0; i < sample.size(); ++i) {
        sample[i] = sample[i] + power;
    }
}

void addPowerOFDM(OFDM_symbol ofdms, float power) {
    for(int i = 0; i < ofdms.size; i++) {
        addPower(ofdms.symbol[i], power);
    }
}

OFDM_symbol OFDM_modulator(VecSymbolMod samples, OFDM_params &param) {
    
    OFDM_symbol ofdms;
    VecSymbolMod def_interval((int)param.def_interval, {VALUE_DEF_INTERVAL, VALUE_DEF_INTERVAL});
    int count_sub_no_defi = 
        param.count_subcarriers - param.def_interval * 2;
    VecSymbolMod interval_no_data((int)param.step_RS, {NO_DATA, NO_DATA});
    int con = TO_INT(OFDM_STATE::NEXT_OFDM_SYMBOL);
    bool running = true;
    VecSymbolMod ofdm;
    int pos_def_interval = BEGIN;
    int index_step_data = 0;
    int count_rs = count_sub_no_defi / (param.step_RS + 1) + 1;
    // int size_block_data = (count_sub_no_defi - count_rs) + 
    //     (count_sub_no_defi % param.step_RS);
    int size_block_data = count_sub_no_defi - count_rs;
    print_log(LOG_DATA, "tx data [ %d ]:\n", samples.size());
    print_VecSymbolMod(samples);
    print_log(CONSOLE, "count_rs = %d, count_sub_no_defi = %d"
        ", size_block_data = %d\n", 
        count_rs, count_sub_no_defi, size_block_data);
    // return ofdms;
    bool fill_symbol_and_end = false;
    int step;
    int block;
    int con_add = 0;
    fftwf_complex* out;
    fftwf_complex* in;
    fftwf_plan plan;
    VecSymbolMod ofdm_ifft(count_sub_no_defi);
    while(running) {
        print_log(LOG_DATA, "[%s:%d] con state = %d\n", __func__, __LINE__, con);
        switch (con)
        {
        case TO_INT(OFDM_STATE::EXIT):
            running = false;
            break;
        case TO_INT(OFDM_STATE::NEXT_OFDM_SYMBOL):
            pos_def_interval = BEGIN;
            if(fill_symbol_and_end) {
                con = TO_INT(OFDM_STATE::EXIT);
                break;
            }
            if(samples.size() - index_step_data == size_block_data) {
                // con = TO_INT(OFDM_STATE::EXIT);
                con = TO_INT(OFDM_STATE::FILL_SYMBOL);
                fill_symbol_and_end = true;
                break;
            }
            if(samples.size() - index_step_data < size_block_data) {
                fill_symbol_and_end = true;
                VecSymbolMod interval_no_data(
                    size_block_data - (samples.size() - index_step_data), 
                    {NO_DATA, NO_DATA});
                samples.insert(samples.end(), 
                    interval_no_data.begin(), interval_no_data.end());
                PRINT_DEBUG_LINE("fill zero\n");
            }
            con = TO_INT(OFDM_STATE::FILL_SYMBOL);
            break;
        case TO_INT(OFDM_STATE::ADD_DEF_INTERVAL):
            print_log(LOG_DATA, "[%d] No def interval:\n", __LINE__);
            print_VecSymbolMod(ofdm);
            ofdm.insert(ofdm.begin(), def_interval.begin(), def_interval.end());
            print_log(LOG_DATA, "add def size = %d\n", ofdm.size());
            ofdm.insert(ofdm.end(), 
                def_interval.begin(), def_interval.end());
            con = TO_INT(OFDM_STATE::ADD_OFDM_SYMBOL_TO_LIST_AND_CLEAR);
            print_log(LOG_DATA, "add def 2 size = %d\n", ofdm.size());
            break;
        case TO_INT(OFDM_STATE::FILL_SYMBOL):
            block = size_block_data;
            
            con_add = 0;
            while(ofdm.size() < count_sub_no_defi) {
                switch (con_add)
                {
                case 0:
                    
                    ofdm.push_back(param.pilot);
                    print_log(CONSOLE, "add pilot - %d\n", ofdm.size());
                    con_add = 1;
                    break;
                case 1:
                
                    // if(block < param.step_RS) {
                    //     step = block;
                    // } else {
                    //     step = param.step_RS;
                    // }
                    // if(step > 0) {
                    //     ofdm.insert(ofdm.end(), 
                    //         samples.begin() + index_step_data, 
                    //         samples.begin() + index_step_data + step);
                    // }
                    // index_step_data += step;
                    // block -= (step );
                    if(count_sub_no_defi - ofdm.size() < param.step_RS) {
                        step = count_sub_no_defi - ofdm.size();
                    } else {
                        step = param.step_RS;
                    }
                    if(step > 0) {
                        ofdm.insert(ofdm.end(), 
                            samples.begin() + index_step_data, 
                            samples.begin() + index_step_data + step);
                        index_step_data += step;
                        block -= (step );
                    }
                    print_log(CONSOLE, "add data[%d] - %d\n", step, ofdm.size());
                    con_add = 0;
                    break;
                default:
                    break;
                }
            }

            // for(int i = 0; i < count_rs; ++i) {
            //     print_log(LOG, "\tblock = %d, step = %d, ofdm.size() = %d\n",
            //     block, step, ofdm.size());
            //     ofdm.push_back(param.pilot);
            //     if(block < param.step_RS) {
            //         step = block;
            //     } else {
            //         step = param.step_RS;
            //     }
            //     if(step > 0) {
            //         ofdm.insert(ofdm.end(), 
            //             samples.begin() + index_step_data, 
            //             samples.begin() + index_step_data + step);
            //     }
            //     print_log(LOG, "\t[%d]step = %d, ofdm.size() = %d\n", i, step, ofdm.size());
            //     index_step_data += step;
            //     block -= (step );
            // }
            print_log(LOG_DATA, "new size = %d\n", ofdm.size());
            // ofdm_ifft
            out = fftwf_alloc_complex(ofdm.size());
            in = reinterpret_cast<fftwf_complex*>(ofdm.data());
            plan = fftwf_plan_dft_1d(ofdm.size(),
                 in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
            fftwf_execute(plan);

            for (int i = 0; i < ofdm.size(); ++i) {
                ofdm_ifft[i] = mod_symbol(out[i][0] /= ofdm.size(),
                                          out[i][1] /= ofdm.size());
                // ofdm_ifft[i] = mod_symbol(out[i][0],
                //                           out[i][1]);
                print_log(LOG_DATA, "%f %f\n", ofdm_ifft[i].real(), ofdm_ifft[i].imag());
            }
            print_log(LOG_DATA, "orig: %d, new: %d\n", ofdm.size(), ofdm_ifft.size());
            ofdm = ofdm_ifft;
            fftwf_destroy_plan(plan);
            fftwf_free(out);
            con = TO_INT(OFDM_STATE::ADD_DEF_INTERVAL);
            break;
        case TO_INT(OFDM_STATE::ADD_OFDM_SYMBOL_TO_LIST_AND_CLEAR):
            if(ofdm.size() != param.count_subcarriers) {
                print_log(ERROR_OUT, 
    "Error create ofdm symbol, size != %d, size = %d\n", 
    param.count_subcarriers, ofdm.size());
                FAILED;
            }
            ofdms.size += 1;
            add_cyclic_prefix(ofdm, param.cyclic_prefix);
            ofdms.symbol.push_back(ofdm);
            print_VecSymbolMod(ofdm);
            ofdm.clear();
            
            con = TO_INT(OFDM_STATE::NEXT_OFDM_SYMBOL);
            break;
        default:
            break;
        }
    }
    return ofdms;
}


VecSymbolMod linearInterpolation(VecSymbolMod& points, size_t num_points) {
    if (points.size() < 2) {
        throw std::invalid_argument("At least two points are required for interpolation.");
    }
    VecSymbolMod interpolated;
    interpolated.reserve(num_points);
    float step = static_cast<float>(points.size() - 1) / (num_points - 1);
    for (size_t i = 0; i < num_points; ++i) {
        float index = i * step;
        size_t lower_index = static_cast<size_t>(index);
        size_t upper_index = lower_index + 1;
        if (upper_index >= points.size()) {
            interpolated.push_back(points.back());
            continue;
        }
        float t = index - lower_index;
        mod_symbol interpolated_value = points[lower_index] + t * (points[upper_index] - points[lower_index]);
        interpolated.push_back(interpolated_value);
    }

    return interpolated;
}
VecSymbolMod OFDM_demodulator(OFDM_symbol samples, OFDM_params &param) {
    // delete_cyclic_prefix(samples)
    int count_sub_no_defi = 
        param.count_subcarriers - param.def_interval * 2;
    int count_rs = count_sub_no_defi / (param.step_RS + 1) + 1;
    int size_block_data = count_sub_no_defi - count_rs;
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);
    VecSymbolMod rx;
    fftwf_complex* out;
    fftwf_complex* in;
    fftwf_plan plan;
    VecSymbolMod ofdm_fft(count_sub_no_defi);
    for(int i = 0; i < samples.size; ++i) {
        VecSymbolMod &ofdm = samples.symbol[i];
        delete_cyclic_prefix(ofdm, param.cyclic_prefix);
        print_log(CONSOLE, "ofdm.size() = %d\n", ofdm.size());
        ofdm.erase(ofdm.begin(), ofdm.begin() + param.def_interval);
        ofdm.erase(ofdm.end() - param.def_interval, ofdm.end());
        VecSymbolMod pilots_tx(count_rs, param.pilot);
        VecSymbolMod pilots_rx;
        int step_pilot = 1 + param.step_RS;

        out = fftwf_alloc_complex(ofdm.size());
        in = reinterpret_cast<fftwf_complex*>(ofdm.data());
        plan = fftwf_plan_dft_1d(ofdm.size(),
                in, out, FFTW_FORWARD, FFTW_ESTIMATE);
        fftwf_execute(plan);
        for (int i = 0; i < ofdm.size(); ++i) {
            ofdm_fft[i] = mod_symbol(out[i][0] /= ofdm.size(),
                                        out[i][1] /= ofdm.size());
            print_log(LOG_DATA, "%f %f\n", 
            ofdm_fft[i].real(), ofdm_fft[i].imag());
        }
        print_log(LOG_DATA, "orig: %d, new: %d\n", 
            ofdm.size(), ofdm_fft.size());
        ofdm = ofdm_fft;
        fftwf_destroy_plan(plan);
        fftwf_free(out);
        // for(int j = 0; j < ofdm.size(); j += step_pilot) {
            
        //     pilots_rx.push_back(ofdm[j]);
        // }
        for(int j = 0, step = 0; j < count_rs; ++j, step += param.step_RS) {
            pilots_rx.push_back(ofdm[j + step]);
        }
        print_log(CONSOLE, "ofdm.size() = %d, count pilots = %d\n", ofdm.size(), pilots_rx.size());
        print_VecSymbolMod(pilots_rx);
        VecSymbolMod H = pilots_rx / pilots_tx;
        DEBUG_LINE
        VecSymbolMod Heq = linearInterpolation(H, ofdm.size());
        VecSymbolMod ofdm_eq = ofdm / Heq;
        VecSymbolMod sample_rx;
        int block = size_block_data;
        int step;
        int index = 1;
        // for(int j = 0; j < ofdm_eq.size(); j += step_pilot) {
        //     if(block < param.step_RS) {
        //         step = block;
        //     } else {
        //         step = param.step_RS;
        //     }
        //     if(step > 0)
        //         sample_rx.insert(
        //             sample_rx.end(), ofdm_eq.begin() + j + 1, 
        //             ofdm_eq.begin() + j + step);
        //         // sample_rx.insert(
        //         //     sample_rx.end(), ofdm_eq.begin() + index, 
        //         //     ofdm_eq.begin() + index + step);
        //     block -= (step );
        //     // index += 
        // }
/*

1 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 1 0 0 0 0

0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 1 0 0 0 0






*/
        for(int j = 1; sample_rx.size() < size_block_data; ) {
            step = param.step_RS;
            if(ofdm.size() - j < param.step_RS) {
                step = ofdm.size() - j;
            }
            if(step > 0) {
                sample_rx.insert(
                    sample_rx.end(), ofdm_eq.begin() + j, 
                    ofdm_eq.begin() + j + step);
                    
            } else {
                break;
            }
            j += step + 1;
            print_log(CONSOLE, "j = %d, sample_rx.size() = %d, step = %d\n", j, sample_rx.size(), step);
        }

        print_log(LOG, "rx data [ %d ]:\n", sample_rx.size());
        print_VecSymbolMod(sample_rx);
        rx.insert(rx.end(), sample_rx.begin(), sample_rx.end());
        
    }
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);
    print_VecSymbolMod(rx);
    return rx;


}




VecSymbolMod generateZadoffChuSequence(int cellId, int N) {
    VecSymbolMod pss(N);
    int q = 1;
    for (int n = 0; n < N; ++n) {
        double realPart = cos(PI * q * n * (n + 1) / N);
        double imagPart = sin(PI * q * n * (n + 1) / N);
        pss[n] = mod_symbol(realPart, imagPart);
    }
    for (int n = 0; n < N; ++n) {
        int shift = (n * (cellId + 1)) % N;
        pss[n] = mod_symbol(cos(2 * PI * shift / N), sin(2 * PI * shift / N)) * pss[n];
    }

    return pss;
}

void create_PSS(slot_ofdms &slot, u_char count_symbol) {
    static VecSymbolMod pss = generateZadoffChuSequence(0, 127);
    slot.PSS.insert(slot.PSS.end(), pss.begin(), pss.end());
}


VecSlotsOFDM create_slots(OFDM_symbol ofdms) {
    VecSlotsOFDM slots;
    slot_ofdms slot;
    create_PSS(slot, 0);
    slot.ofdms = ofdms;
    slots.push_back(slot);
    return slots;
}





// void correlate_PSS()









// void receiver_OFDM(VecSymbolMod &samples) {
    
// }








