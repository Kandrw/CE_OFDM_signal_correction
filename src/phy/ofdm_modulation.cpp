#include "ofdm_modulation.hpp"

#include <algorithm>
#include <fftw3.h>


#define VALUE_DEF_INTERVAL 0.f 
#define NO_DATA 0.f
#define LENGTH_PSS 63
#define FAILED exit(-1)
#define CENTRAL_SHIFT 0
#define FILLING_EMPTY 0.5f

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
    for(int i = 0; i < ofdm.size(); ++i) {
        one.insert(one.end(), ofdm[i].begin(), ofdm[i].end());
    }
    return one;
}

OFDM_symbol samples_join_OFDM(VecSymbolMod sample, int subs, int size) {
    OFDM_symbol ofdms;
    for(int i = 0; i < sample.size() && i < size; i += subs) {

        VecSymbolMod s(sample.begin() + i, sample.begin() + i + subs);
        ofdms.push_back(s);
    }
    return ofdms;
}

VecSymbolMod slots_OFDM_convertion_one_thread(VecSlotsOFDM &slots) {
    VecSymbolMod one;

    for(int i =0; i < slots.size(); ++i) {
        one.insert(one.end(), slots[i].PSS.begin(), slots[i].PSS.end());
        VecSymbolMod ofdms = OFDM_convertion_one_thread(slots[i].ofdms);
        one.insert(one.end(), ofdms.begin(), ofdms.end());
    }
    return one;
}
void addPower(VecSymbolMod &sample, float p) {
    mod_symbol power = mod_symbol(p, p);
    for(int i = 0; i < sample.size(); ++i) {
        // sample[i] = sample[i] * power;
        sample[i] = mod_symbol(sample[i].real() * p,
            sample[i].imag() * p);
            
    }
}

void addPowerOFDM(OFDM_symbol &ofdms, float power) {
    for(int i = 0; i < ofdms.size(); i++) {
        addPower(ofdms[i], power);
    }
}

void addPowerSlots(VecSlotsOFDM &slots, float power) {
    for(int i = 0; i < slots.size(); ++i) {
        addPower(slots[i].PSS, power);
        addPowerOFDM(slots[i].ofdms, power);
    }
}


void shift_fft(VecSymbolMod &data) {
    int N = data.size();
    // Сдвиг по частотам
    VecSymbolMod shifted(N);
    
    for (int i = 0; i < N; ++i) {
        shifted[i] = data[(i + N / 2) % N]; // Сдвигаем
    }
    
    data = shifted;
}

// void reverse_shift_fft(VecSymbolMod &data) {
//     int N = data.size();
//     // Обратный сдвиг по частотам
//     VecSymbolMod reversed(N);
    
//     for (int i = 0; i < N; ++i) {
//         reversed[i] = data[(i + N / 2) % N]; // Обратный сдвиг
//     }
    
//     data = reversed;
// }



OFDM_symbol OFDM_modulator(VecSymbolMod samples, OFDM_params &param) {
    
    OFDM_symbol ofdms;
    VecSymbolMod def_interval((int)param.def_interval, {VALUE_DEF_INTERVAL, VALUE_DEF_INTERVAL});
    int central_shift = CENTRAL_SHIFT;
    int count_sub_no_defi = 
        param.count_subcarriers - param.def_interval * 2 - central_shift;
    // VecSymbolMod interval_no_data((int)param.step_RS, {NO_DATA, NO_DATA});
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
    VecSymbolMod ofdm_ifft(param.count_subcarriers);
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
                    {FILLING_EMPTY, FILLING_EMPTY});
                samples.insert(samples.end(), 
                    interval_no_data.begin(), interval_no_data.end());
                PRINT_DEBUG_LINE("fill zero\n");
            }
            con = TO_INT(OFDM_STATE::FILL_SYMBOL);
            break;
        case TO_INT(OFDM_STATE::FILL_SYMBOL):
            block = size_block_data;
            
            con_add = 0;
            while(ofdm.size() < count_sub_no_defi) {
                switch (con_add)
                {
                case 0:
                    ofdm.push_back(param.pilot);
                    print_log(LOG_DATA, "add pilot - %d\n", ofdm.size());
                    con_add = 1;
                    break;
                case 1:
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
                    print_log(LOG_DATA, "add data[%d] - %d\n", step, ofdm.size());
                    con_add = 0;
                    break;
                default:
                    break;
                }
            }
            print_log(LOG_DATA, "[line: %d] size = %d\n", __LINE__, ofdm.size());
            ofdm.insert(ofdm.begin(), def_interval.begin(), def_interval.end());
            print_log(LOG_DATA, "add def size = %d\n", ofdm.size());
            ofdm.insert(ofdm.end(), 
                def_interval.begin(), def_interval.end());
            if(central_shift) {
                int position = ofdm.size() / 2;
                int size_old = ofdm.size();
                if(central_shift > 1) {
                    VecSymbolMod cent(central_shift, mod_symbol(NO_DATA, NO_DATA));
                    ofdm.insert(ofdm.begin() + position, cent.begin(), cent.end());
                
                } else 
                    ofdm.insert(ofdm.begin() + position, mod_symbol(NO_DATA, NO_DATA));
                print_log(LOG_DATA, "[%s:%d] add central - %d, size: old - %d, new  -%d\n", __func__, __LINE__, position, size_old, ofdm.size());
            }
            print_log(LOG_DATA, "new size = %d\n", ofdm.size());
            shift_fft(ofdm);
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
                // print_log(LOG_DATA, "%f %f\n", ofdm_ifft[i].real(), ofdm_ifft[i].imag());
            }
            print_log(LOG_DATA, "orig: %d, new: %d\n", ofdm.size(), ofdm_ifft.size());
            ofdm = ofdm_ifft;
            fftwf_destroy_plan(plan);
            fftwf_free(out);
            con = TO_INT(OFDM_STATE::ADD_OFDM_SYMBOL_TO_LIST_AND_CLEAR);
            break;
        case TO_INT(OFDM_STATE::ADD_OFDM_SYMBOL_TO_LIST_AND_CLEAR):
            if(ofdm.size() != param.count_subcarriers) {
                print_log(ERROR_OUT, 
    "Error create ofdm symbol, size != %d, size = %d\n", 
    param.count_subcarriers, ofdm.size());
                FAILED;
            }
            add_cyclic_prefix(ofdm, param.cyclic_prefix);
            ofdms.push_back(ofdm);
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

int write_OFDM_slots(const char *filename, VecSlotsOFDM &slots, size_t count_write) {
    FILE *file = fopen(filename, "wb");
    print_log(LOG_DATA, "[%s:%d]\n", __func__, __LINE__);
    u_int32_t size;
    size = slots.size();
    fwrite((void*)&size, 1, sizeof(size), file);
    for(size_t i = 0; i < slots.size(), i < count_write; ++i) {
        auto ofdms = slots[i].ofdms;
        size = slots[i].PSS.size();
        // print_log(CONSOLE, "%d %d\n", slots.size(), slots[i].ofdms.size());
        fwrite((void*)&size, 1, sizeof(size), file);
        // print_log(CONSOLE, "size pss = %d\n", size);
        
        fwrite((void*)&slots[i].PSS[0], sizeof(mod_symbol), slots[i].PSS.size(), file);
    
        size = ofdms[0].size();
        // print_log(CONSOLE, "size sub = %d\n", size);
        fwrite((void*)&size, 1, sizeof(size), file);

        size = ofdms.size();
        // print_log(CONSOLE, "[2] size sub = %d\n", size);
        fwrite((void*)&size, 1, sizeof(size), file);
        
        for(size_t j = 0; j < size; ++j) {
            void *data = (void*)&ofdms[j][0];
            fwrite(data, sizeof(mod_symbol), ofdms[0].size(), file);
        }
    }
    fclose(file);
}

int write_OFDMs(const char *filename, const OFDM_symbol &ofdms, 
    size_t count_write) 
{
    FILE *file = fopen(filename, "wb");
    u_int32_t size;
    size = ofdms[0].size();
    print_log(CONSOLE, "size sub = %d\n", size);
    fwrite((void*)&size, 1, sizeof(size), file);
    size = count_write;
    fwrite((void*)&size, 1, sizeof(size), file);
    for(size_t j = 0; j < size; ++j) {
        void *data = (void*)&ofdms[j][0];
        fwrite(data, sizeof(mod_symbol), ofdms[0].size(), file);
    }
    fclose(file);
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
VecSymbolMod OFDM_demodulator(OFDM_symbol samples, OFDM_params &param, bool found_cprefix) {
    // delete_cyclic_prefix(samples)
    int central_shift = CENTRAL_SHIFT;
    int count_sub_no_defi = 
        param.count_subcarriers - param.def_interval * 2 - central_shift;
    int count_rs = count_sub_no_defi / (param.step_RS + 1) + 1;
    int size_block_data = count_sub_no_defi - count_rs;
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);
    VecSymbolMod rx;
    fftwf_complex* out;
    fftwf_complex* in;
    fftwf_plan plan;
    VecSymbolMod ofdm_fft(count_sub_no_defi);

    print_log(LOG_DATA, "ofdms:\n");
    for(int i = 0; i < samples.size(); ++i) {
        print_log(LOG_DATA, "\t%d ofdm:\n", i + 1);
        print_VecSymbolMod(samples[i]);
    }
        

    for(int i = 0; i < samples.size(); ++i) {
// #if 0
        VecSymbolMod &ofdm = samples[i];
        if(found_cprefix)
            delete_cyclic_prefix(ofdm, param.cyclic_prefix);
        // ofdm.erase(ofdm.begin(), ofdm.begin() + param.def_interval);
        // ofdm.erase(ofdm.end() - param.def_interval, ofdm.end());
        print_log(LOG_DATA, "[%s:%d] ofdm.size() = %d\n", __func__, __LINE__, ofdm.size());

        VecSymbolMod pilots_tx(count_rs, param.pilot);
        VecSymbolMod pilots_rx;
        int step_pilot = 1 + param.step_RS;

        out = fftwf_alloc_complex(ofdm.size());
        in = reinterpret_cast<fftwf_complex*>(ofdm.data());
        plan = fftwf_plan_dft_1d(ofdm.size(),
                in, out, FFTW_FORWARD, FFTW_ESTIMATE);

        fftwf_execute(plan);

       
        print_log(LOG_DATA, "ofdm - %d - fft - %d\n", ofdm.size(), ofdm_fft.size());
#if 1
        for (int i = 0; i < ofdm.size(); ++i) {
            // ofdm_fft[i] = mod_symbol(out[i][0] /= ofdm.size(),
            //                             out[i][1] /= ofdm.size());
            ofdm[i] = mod_symbol(out[i][0] /= ofdm.size(),
                                        out[i][1] /= ofdm.size());
            // print_log(LOG_DATA, "%f %f\n", 
            //     ofdm_fft[i].real(), ofdm_fft[i].imag());
            print_log(LOG_DATA, "%f %f\n", 
                ofdm[i].real(), ofdm[i].imag());
            
        }
#endif
        shift_fft(ofdm);
        if(central_shift) {
            int position = ofdm.size() / 2;
            int size_old = ofdm.size();
            ofdm.erase(ofdm.begin() + position, 
                    ofdm.begin() + position + central_shift);
            print_log(LOG_DATA, "[%s:%d] del central - %d, size: old - %d, new - %d\n", __func__, __LINE__, position, size_old, ofdm.size());
        } 
        ofdm.erase(ofdm.begin(), ofdm.begin() + param.def_interval);
        ofdm.erase(ofdm.end() - param.def_interval, ofdm.end());
        print_log(LOG_DATA, "ofdm.size() = %d\n", ofdm.size());

        

#if 1
        print_log(LOG_DATA, "orig: %d, new: %d\n", 
            ofdm.size(), ofdm_fft.size());
        // ofdm = ofdm_fft;
        fftwf_destroy_plan(plan);
        fftwf_free(out);
        
        for(int j = 0, step = 0; j < count_rs; ++j, step += param.step_RS) {
            pilots_rx.push_back(ofdm[j + step]);
        }
        print_log(LOG_DATA, "ofdm.size() = %d, count pilots = %d\n", ofdm.size(), pilots_rx.size());
        print_log(LOG_DATA, "pilot tx:\n");
        print_VecSymbolMod(pilots_tx);
        print_log(LOG_DATA, "pilot rx:\n");
        print_VecSymbolMod(pilots_rx);


        VecSymbolMod H = pilots_rx / pilots_tx;
#endif
        
#if 1
        VecSymbolMod Heq = linearInterpolation(H, ofdm.size());
        VecSymbolMod ofdm_eq = ofdm / Heq;
        VecSymbolMod sample_rx;
        int block = size_block_data;
        int step;
        int index = 1;
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
            print_log(LOG_DATA, "j = %d, sample_rx.size() = %d, step = %d\n", j, sample_rx.size(), step);
        }

        print_log(LOG_DATA, "rx data [ %d ]:\n", sample_rx.size());
        print_VecSymbolMod(sample_rx);
        rx.insert(rx.end(), sample_rx.begin(), sample_rx.end());
#endif
    }
    print_VecSymbolMod(rx);
    print_log(LOG_DATA, "\t[%s:%d]\n", __func__, __LINE__);
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
    fftwf_complex* out;
    fftwf_complex* in;
    fftwf_plan plan;
    out = fftwf_alloc_complex(pss.size());
    in = reinterpret_cast<fftwf_complex*>(pss.data());
    plan = fftwf_plan_dft_1d(pss.size(),
            in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
    fftwf_execute(plan);
    for (int i = 0; i < pss.size(); ++i) {
        pss[i] = mod_symbol(out[i][0] /= pss.size(),
                                    out[i][1] /= pss.size());
        // print_log(LOG_DATA, "%f %f\n", ofdm_ifft[i].real(), ofdm_ifft[i].imag());
    }
    fftwf_destroy_plan(plan);
    fftwf_free(out);
    return pss;
}

void create_PSS(slot_ofdms &slot, u_char count_symbol) {

    slot.PSS = generateZadoffChuSequence(0, LENGTH_PSS);
#if 1
    write_file_bin_data(
        "../data/pss.bin", 
        (void*)&slot.PSS[0], 
        sizeof(float) * slot.PSS.size() * 2);
#endif
}

// enum class STATE_SLOTS {
    // FILL_SLOT,
    // ADD_SLOT,
    // EXIT
// };
/*
VecSlotsOFDM create_slots1(const OFDM_symbol &ofdms,
    const OFDM_params &param_ofdm)
{
    VecSlotsOFDM slots;

    int iter = 0, slice = 0;
    bool run = true;
    STATE_SLOTS state = STATE_SLOTS::FILL_SLOT;
    slot_ofdms slot;
    create_PSS(slot, 0);
    while(run) {
        switch (state)
        {
        case STATE_SLOTS::FILL_SLOT:
            state = 
            slice = iter + param_ofdm.count_ofdm_in_slot;

            if(slice > ofdms.size()) {
                slice = ofdms.size();
                if(slice == iter) {
                    break;
                }    
            }
            
            std::copy(ofdms.begin() + iter, ofdms.begin() + slice, std::back_inserter(slot));
            break;
        case STATE_SLOTS::ADD_SLOT:
            
            break;
        case STATE_SLOTS::EXIT:
            
            run = false;
            break;
        
        default:
            run = false;
            break;
        }
    }

    slot.ofdms = ofdms;
    slots.push_back(slot);
    
    return slots;
}
*/
VecSlotsOFDM create_slots(const OFDM_symbol &ofdms,
    const OFDM_params &param_ofdm)
{
    VecSlotsOFDM slots;

    int iter = 0, slice = 0;
    bool run = true;
    slot_ofdms slot;
    // print_log(CONSOLE, "ofdms.size = %d\n", ofdms.size());     
    create_PSS(slot, 0);
    while(run) {
        slice = iter + param_ofdm.count_ofdm_in_slot;
        // print_log(CONSOLE, "size - %d, iter - %d, slice - %d\n", ofdms.size(), iter, slice);
        if(slice >= ofdms.size()) {
            slice = ofdms.size();
            // print_log(CONSOLE, "[2] size - %d, iter - %d, slice - %d\n", ofdms.size(), iter, slice);
        
            if(slice == iter) {
                break;
            }    
        }
        
        
        std::copy(ofdms.begin() + iter, ofdms.begin() + slice, std::back_inserter(slot.ofdms));
        // print_log(CONSOLE, "==== slot.size = %d\n", slot.ofdms.size());        
        slots.push_back(slot); 
        slot.ofdms.clear();
        iter = slice;
    }
    // print_log(CONSOLE, "slots.size = %d\n", slots.size());
    // for(int i = 0; i < slots.size(); ++i) {
    //     print_log(CONSOLE, ") %d\n", slots[i].ofdms.size());
    // }
    
    return slots;
}






