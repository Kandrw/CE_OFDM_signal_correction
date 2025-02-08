#include "ofdm_modulation.hpp"

#include <algorithm>
#include <fftw3.h>


#define VALUE_DEF_INTERVAL 0.f 
#define NO_DATA 0.f
#define LENGTH_PSS 127
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

VecSymbolMod slots_OFDM_convertion_one_thread(VecSlotsOFDM &slots) {
    VecSymbolMod one;

    for(int i =0; i < slots.size(); ++i) {
        one.insert(one.end(), slots[i].PSS.begin(), slots[i].PSS.end());
        VecSymbolMod ofdms = OFDM_convertion_one_thread(slots[i].ofdms);
        one.insert(one.end(), ofdms.begin(), ofdms.end());
    }
    return one;
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
                    {NO_DATA, NO_DATA});
                samples.insert(samples.end(), 
                    interval_no_data.begin(), interval_no_data.end());
                PRINT_DEBUG_LINE("fill zero\n");
            }
            con = TO_INT(OFDM_STATE::FILL_SYMBOL);
            break;
        case TO_INT(OFDM_STATE::ADD_DEF_INTERVAL):
            // print_log(LOG_DATA, "[%d] No def interval:\n", __LINE__);
            // print_VecSymbolMod(ofdm);
            // ofdm.insert(ofdm.begin(), def_interval.begin(), def_interval.end());
            // print_log(LOG_DATA, "add def size = %d\n", ofdm.size());
            // ofdm.insert(ofdm.end(), 
            //     def_interval.begin(), def_interval.end());
            // con = TO_INT(OFDM_STATE::ADD_OFDM_SYMBOL_TO_LIST_AND_CLEAR);
            // print_log(LOG_DATA, "add def 2 size = %d\n", ofdm.size());
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
            ofdm.insert(ofdm.begin(), def_interval.begin(), def_interval.end());
            print_log(LOG_DATA, "add def size = %d\n", ofdm.size());
            ofdm.insert(ofdm.end(), 
                def_interval.begin(), def_interval.end());
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

int write_OFDM_slots(const char *filename, VecSlotsOFDM &slots, size_t count_write) {
    FILE *file = fopen(filename, "wb");
    for(size_t i = 0; i < slots.size(), i < count_write; ++i) {
        auto ofdms = slots[i].ofdms;
        u_int32_t size;
        size = slots[i].PSS.size();
        fwrite((void*)&size, 1, sizeof(size), file);
        fwrite((void*)&slots[i].PSS[0], sizeof(mod_symbol), slots[i].PSS.size(), file);
    
        size = ofdms.symbol[0].size();
        print_log(CONSOLE, "size sub = %d\n", size);
        fwrite((void*)&size, 1, sizeof(size), file);

        size = ofdms.size;
        fwrite((void*)&size, 1, sizeof(size), file);
        
        for(size_t j = 0; j < size; ++j) {
            void *data = (void*)&ofdms.symbol[j][0];
            fwrite(data, sizeof(mod_symbol), ofdms.symbol[0].size(), file);
        }
    }
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
        // ofdm.erase(ofdm.begin(), ofdm.begin() + param.def_interval);
        // ofdm.erase(ofdm.end() - param.def_interval, ofdm.end());
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


VecSlotsOFDM create_slots(OFDM_symbol ofdms) {
    VecSlotsOFDM slots;
    slot_ofdms slot;
    create_PSS(slot, 0);
    slot.ofdms = ofdms;
    slots.push_back(slot);
    //Разбить
    return slots;
}


std::vector<float> autoCorrelation(const VecSymbolMod& signal, const VecSymbolMod& reference) {
    size_t signalLength = signal.size();
    size_t refLength = reference.size();
    std::vector<float> correlation(signalLength, 0.0);

    for (size_t lag = 0; lag < signalLength; ++lag) {
        for (size_t n = 0; n < refLength; ++n) {
            if (lag + n < signalLength) {
                correlation[lag] += fabs(signal[lag + n] * conj(reference[n])); // Используем модуль для корреляции
            }
        }
    }
    return correlation;
}

int findSymbolStart(std::vector<float>& correlation, double threshold) {
    for (size_t i = 0; i < correlation.size(); ++i) {
        if (correlation[i] > threshold) {
            return i; // Возвращаем индекс начала символа
        }
    }
    return -1; // Если не найдено
}


int correlate(VecSymbolMod &samples, VecSymbolMod &pss) {
    std::vector<float> corr_array(samples.size() - pss.size() + 1);
    for(int i = 0; i <= samples.size() - pss.size(); ++i) {
        corr_array[i] = 0.f;
        for(int j = 0; j < pss.size(); ++j) {
            corr_array[i] += std::norm(samples[i + j] * std::conj(pss[j]));
        }
    }
    write_file_bin_data(
            "../data/corr_array.bin", 
            (void*)&corr_array[0], 
            sizeof(float) * corr_array.size());
    return std::max_element(corr_array.begin(), corr_array.end()) - corr_array.begin();
    // return corr_array;
}

int find_pss_start_fft1(const VecSymbolMod& received_signal, const VecSymbolMod& pss) {
    int received_length = received_signal.size();
    int pss_length = pss.size();
    int fft_size = received_length + pss_length - 1;


    fftwf_complex *in_signal = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * fft_size);
    fftwf_complex *in_pss = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * fft_size);
    fftwf_complex *out_signal = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * fft_size);
    fftwf_complex *out_pss = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * fft_size);
    
    for (int i = 0; i < received_length; ++i) {
        in_signal[i][0] = received_signal[i].real();
        in_signal[i][1] = received_signal[i].imag();
    }
    for (int i = 0; i < pss_length; ++i) {
        in_pss[i][0] = pss[i].real();
        in_pss[i][1] = pss[i].imag();
    }
    fftwf_plan plan_signal = fftwf_plan_dft_1d(fft_size, in_signal, out_signal, FFTW_FORWARD, FFTW_ESTIMATE);
    fftwf_plan plan_pss = fftwf_plan_dft_1d(fft_size, in_pss, out_pss, FFTW_FORWARD, FFTW_ESTIMATE);

    fftwf_execute(plan_signal);
    fftwf_execute(plan_pss);

    std::vector<float> correlation(fft_size);
    for (int i = 0; i < fft_size; ++i) {
        correlation[i] = abs(out_signal[i][0] * std::conj(out_pss[i][0]) + 
            out_signal[i][1] * std::conj(out_pss[i][1]));
    }
    write_file_bin_data(
            "../data/corr_array4.bin", 
            (void*)&correlation[0], 
            sizeof(float) * correlation.size());
    int start_index = std::max_element(correlation.begin(), correlation.end()) - correlation.begin();
    fftwf_destroy_plan(plan_signal);
    fftwf_destroy_plan(plan_pss);
    fftwf_free(in_signal);
    fftwf_free(in_pss);
    fftwf_free(out_signal);
    fftwf_free(out_pss);

    return start_index;
}
int find_pss_start_fft(const VecSymbolMod& received_signal, const VecSymbolMod& pss) {
    int received_length = received_signal.size();
    int pss_length = pss.size();
    int fft_size = received_length + pss_length - 1;


    std::vector<float> correlation(fft_size);
    for (int i = 0; i < fft_size; ++i) {
        correlation[i] = abs(received_signal[i].real() * std::conj(pss[i].real()) + 
            received_signal[i].imag() * std::conj(pss[i].imag()));
    }
    write_file_bin_data(
            "../data/corr_array2.bin", 
            (void*)&correlation[0], 
            sizeof(float) * correlation.size());
    int start_index = std::max_element(correlation.begin(), correlation.end()) - correlation.begin();

    return start_index;
}

std::vector<float> normalizedCorrelation(const VecSymbolMod& signal, const VecSymbolMod& reference) {
    size_t signalLength = signal.size();
    size_t refLength = reference.size();
    std::vector<float> correlation(signalLength, 0.0);
    float refMean = 0.0;
    for (const auto& val : reference) {
        refMean += abs(val);
    }
    refMean /= refLength;
    float refStdDev = 0.0;
    for (const auto& val : reference) {
        refStdDev += pow(abs(val) - refMean, 2);
    }
    refStdDev = sqrt(refStdDev / refLength);
    for (size_t lag = 0; lag < signalLength; ++lag) {
        for (size_t n = 0; n < refLength; ++n) {
            if (lag + n < signalLength) {
                correlation[lag] += (abs(signal[lag + n]) - refMean) * (abs(reference[n]) - refMean);
            }
        } 
        // if (refStdDev != 0) {
            correlation[lag] /= (refStdDev * refLength);
        // }
    }
    write_file_bin_data(
            "../data/corr_array3.bin", 
            (void*)&correlation[0], 
            sizeof(float) * correlation.size());
    return correlation;
}


int findSymbolStart1(const std::vector<float>& correlation, float threshold) {
    for (size_t i = 0; i < correlation.size(); ++i) {
        // if (correlation[i] > threshold) {
        //     return i;
        // }
        if (fabs(correlation[i]) > threshold) {
            return i;
        }
    }
    return -1;
}


double euclideanNorm(const VecSymbolMod& vec) {
    double sum = 0.0;
    for (const auto& val : vec) {
        sum += std::norm(val);
    }
    return sqrt(sum);
}
mod_symbol vdot1(const VecSymbolMod& vec1, const VecSymbolMod& vec2) {

    mod_symbol result = 0.0;
    for (size_t i = 0; i < vec1.size(); ++i) {
        result += std::conj(vec1[i]) * vec2[i]; 
    }
    return result;
}
// float vdot(const VecSymbolMod& vec1, const VecSymbolMod& vec2) {

//     float result = 0.0f;
//     for (size_t i = 0; i < vec1.size(); ++i) {
//         result += vec1[i] * vec2[i];
//     }
//     return result;
// }

mod_symbol norm_corr(VecSymbolMod& vec1, VecSymbolMod& vec2) {
    // mod_symbol r = vdot(vec1, vec2) / ;

}
VecSymbolMod convolve(const VecSymbolMod& x, const VecSymbolMod& h) {
    int n = x.size();
    int m = h.size();
    VecSymbolMod result(n + m - 1, std::complex<float>(0, 0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            result[i + j] += x[i] * std::conj(h[j]);
        }
    }
    write_file_bin_data(
        "../data/corr_array_convolve.bin", 
        (void*)&result[0], 
        sizeof(float) * result.size() * 2);
    return result;
}


void receiver_OFDM(VecSymbolMod &samples, OFDM_params &param_ofdm) {
    VecSymbolMod pss = generateZadoffChuSequence(0, LENGTH_PSS);
#if 0
    std::vector<float> corr_array = correlate_PSS(samples, pss);
    write_file_bin_data(
            "../data/corr_array2.bin", 
            (void*)&corr_array[0], 
            sizeof(float) * corr_array.size());
#endif
    int pos_data;
    //  = max_element(corr_array.begin(), corr_array.end()) - corr_array.begin();
    convolve(samples, pss);
    print_log(CONSOLE, "samples.size() = %d\n", samples.size() );
    pos_data = find_pss_start_fft(samples, pss);
    print_log(LOG, "[%s:%d] pos data = %d\n", __func__, __LINE__, pos_data);
    // pos_data = normalizedCorrelation(samples, pss);
    pos_data = findSymbolStart1(normalizedCorrelation(samples, pss), 0.5);
    print_log(LOG, "[%s:%d] pos data = %d\n", __func__, __LINE__, pos_data);
    pos_data = correlate(samples, pss);
    // pos_data -= 100;
    print_log(LOG, "[%s:%d] pos data = %d\n", __func__, __LINE__, pos_data);
    if(pos_data > samples.size()) {
        print_log(ERROR_OUT, "Error find pss: %d > %d\n", pos_data, samples.size());
        return;
    }
    return;
    int count = 1;
    for(int i = 0; i < count; ++i) {
        for(int j = pos_data; j < samples.size(); ++j) {
            VecSymbolMod prefix(samples.begin() + j, samples.begin() + j + param_ofdm.count_subcarriers);
            VecSymbolMod s(samples.begin() + j, samples.end());
            int p = correlate(s, prefix);
            print_log(CONSOLE, "1p = %d\n", p);
            // p = find_pss_start_fft(s, prefix);
            // p = normalizedCorrelation(s, prefix);
            
            print_log(CONSOLE, "2p = %d\n", p);
            auto sdf = autoCorrelation(s, prefix);
            // print_VecSymbolMod(sdf, LOG_DATA);
            for(int g = 0; g < sdf.size(); ++g) {
                print_log(LOG_DATA, "%f ", sdf[g]);
            }
            print_log(LOG_DATA, "\n");
            p = findSymbolStart(sdf, 0.6);
            print_log(CONSOLE, "3p = %d\n", p);
        }
        // VecSymbolMod prefix(samples.begin() + pos_data, samples.begin() + pos_data + param_ofdm.count_subcarriers);
        // VecSymbolMod s(samples.begin() + pos_data, samples.end());
        // int p = correlate(s, prefix);
        // print_log(CONSOLE, "1p = %d\n", p);
        // // p = find_pss_start_fft(s, prefix);
        // // p = normalizedCorrelation(s, prefix);
        
        // print_log(CONSOLE, "2p = %d\n", p);
        // auto sdf = autoCorrelation(s, prefix);
        // // print_VecSymbolMod(sdf, LOG_DATA);
        // for(int g = 0; g < sdf.size(); ++g) {
        //     print_log(LOG_DATA, "%f ", sdf[g]);
        // }
        // print_log(LOG_DATA, "\n");
        // p = findSymbolStart(sdf, 0.6);
        // print_log(CONSOLE, "3p = %d\n", p);
    }
    

}








