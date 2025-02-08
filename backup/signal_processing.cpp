#include "signal_processing.hpp"

#include <algorithm>
#include <fftw3.h>

#include <iostream>
#include <vector>
#include <complex>
#include <numeric>
#include <cmath>
#include <stdexcept>

#include "types.hpp"
#include "../ipc/managment_ipc.hpp"
#include "ofdm_modulation.hpp"


std::vector<float> autoCorrelation1(const VecSymbolMod& signal, const VecSymbolMod& reference) {
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

VecSymbolMod convolve1212(const VecSymbolMod& x, const VecSymbolMod& h) {
    int n = x.size();
    int m = h.size();
    VecSymbolMod result(n + m - 1, mod_symbol(0, 0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            result[i + j] += x[i] * std::conj(h[j]);
        }
    }
    return result;
}
static VecSymbolMod convolve(const VecSymbolMod& x, const VecSymbolMod& h) {
    int n = x.size();
    int m = h.size();
    VecSymbolMod result(n + m - 1, mod_symbol(0, 0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            // result[i + j] += x[i] * std::conj(h[j]);
            result[i + j] += x[i] * h[j];
            
        }
    }
    return result;
}

int maxElement(const VecSymbolMod& vec) {
    auto maxel = std::max_element(vec.begin(), vec.end(), 
        [](const mod_symbol& a, const mod_symbol& b) {
            return std::abs(a) < std::abs(b);
        });
    return std::distance(vec.begin(), maxel);
}


int autoCorrelation(const VecSymbolMod& samples, const VecSymbolMod& prefix) {
    VecSymbolMod corr_array = convolve(samples, prefix);
    return maxElement(corr_array);
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

// mod_symbol norm_corr(VecSymbolMod& vec1, VecSymbolMod& vec2) {
//     // mod_symbol r = vdot(vec1, vec2) / ;

// }

// Функция для вычисления нормы вектора
float norm1(const std::vector<float>& vec) {
    // return std::sqrt(std::inner_product(vec.begin(), vec.end(), vec.begin(), 0.0));
    for(int i = 0; i < vec.size(); ++i){
        // vec[i] = std::norm(vec[i]);
    }
}

float norm(const std::vector<float>& vec) {
    float sum_of_squares = 0.0;
    for (float value : vec) {
        sum_of_squares += value * value; // Суммируем квадраты элементов
    }
    // return std::sqrt(sum_of_squares); // Возвращаем квадратный корень из суммы квадратов
    return (sum_of_squares);
}

std::complex<float> dot_product1(const VecSymbolMod& a, const VecSymbolMod& b) {
    if (a.size() != b.size()) {
        throw std::invalid_argument("Vectors must be of the same size.");
    }

    std::complex<float> result(0.0, 0.0);
    for (size_t i = 0; i < a.size(); ++i) {
        result += a[i] * std::conj(b[i]);
    }

    return result;
}

float dot_product(const std::vector<float>& a, const std::vector<float>& b) {
    if (a.size() != b.size()) {
        throw std::invalid_argument("Vectors must be of the same size.");
    }

    float result = 0.0f;
    for (size_t i = 0; i < a.size(); ++i) {
        result += a[i] * b[i]; // Умножаем элементы векторов
    }

    return result;
}

// Функция для вычисления нормализованной корреляции
std::complex<float> norm_corr(const VecSymbolMod& x, const VecSymbolMod& y) {
    if (x.size() != y.size() || x.empty()) {
        throw std::invalid_argument("Векторы должны быть одинаковой ненулевой длины.");
    }

    // Вычисляем реальные и мнимые части
    std::vector<float> x_real(x.size());
    std::vector<float> x_imag(x.size());
    std::vector<float> y_real(y.size());
    std::vector<float> y_imag(y.size());
    for (size_t i = 0; i < x.size(); ++i) {
        x_real[i] = x[i].real();
        x_imag[i] = x[i].imag();
        y_real[i] = y[i].real();
        y_imag[i] = y[i].imag();
    }
    float x_real_norm = norm(x_real);
    float y_real_norm = norm(y_real);
    float x_imag_norm = norm(x_imag);
    float y_imag_norm = norm(y_imag);
    if (x_real_norm == 0 || y_real_norm == 0 || x_imag_norm == 0 || y_imag_norm == 0) {
        throw std::runtime_error("Невозможно выполнить нормализацию, норма одного из векторов равна нулю.");
    }
#if 0
    float c_real = std::inner_product(x_real.begin(), x_real.end(), y_real.begin(), 0.0f) / (x_real_norm * y_real_norm);
    float c_imag = std::inner_product(x_imag.begin(), x_imag.end(), y_imag.begin(), 0.0f) / (x_imag_norm * y_imag_norm);
#endif
    float c_real = dot_product(x_real, y_real) / (x_real_norm * y_real_norm);
    float c_imag = dot_product(x_imag, y_imag) / (x_imag_norm * y_imag_norm);
    
    return std::complex<float>(c_real, c_imag);
}


VecSymbolMod autocorrelation(const VecSymbolMod& signal) {
    size_t N = signal.size();
    VecSymbolMod result(N);

    for (size_t lag = 0; lag < N; ++lag) {
        std::complex<float> sum = 0.0f;
        for (size_t n = 0; n < N; ++n) {
            sum += signal[n] * std::conj(signal[(n + lag) % N]); // Используем циклический доступ
        }
        result[lag] = sum / static_cast<float>(N); // Нормируем
    }

    return result;
}
// int Correlation(int a[], int b[]) {
//     double sum = 0;
//     for (int i = 0; i < N; i++) {
//         sum += a[i] * b[i];
//     }
//     return sum;
// }

// double NormalCorrelation1(int *a, int *b) {
//     double sum_norm = 0;
//     double sumA_squared = 0;
//     double sumB_squared = 0;
    
//     for (int i = 0; i < N; i++) {
//         sum_norm += a[i] * b[i];
//         sumA_squared += pow(a[i], 2);
//         sumB_squared += pow(b[i], 2);
//     }
    
//     double normalization = sqrt(sumA_squared * sumB_squared);
//     return sum_norm / normalization;
// }

float normalized_correlation1(const VecSymbolMod& signal1, const VecSymbolMod& signal2) {
    float sum_product = 0.0;
    float sum_sq_signal1 = 0.0;
    float sum_sq_signal2 = 0.0;

    for (size_t i = 0; i < signal1.size(); ++i) {
        // sum_product += std::norm(signal1[i]) * std::norm(signal2[i]);
        sum_product += std::norm(signal1[i]) * std::norm(signal2[i]);
        sum_sq_signal1 += std::norm(signal1[i]);
        sum_sq_signal2 += std::norm(signal2[i]);
    }
    return sum_product / (std::sqrt(sum_sq_signal1) * std::sqrt(sum_sq_signal2));
    // return sum_product * (std::sqrt(sum_sq_signal1 * sum_sq_signal1 + sum_sq_signal2 * sum_sq_signal2));
    
}


std::vector<float> correlation(const VecSymbolMod& y1, const VecSymbolMod& y2) {
    size_t len1 = y1.size();
    size_t len2 = y2.size();
    size_t maxShift = len1 - len2 + 1;  // Максимальный сдвиг для корреляции
    std::vector<float> result(maxShift, 0.0);

    // Предварительные вычисления нормировочных коэффициентов
    float normY2 = 0.0;
    for (const auto& v : y2) {
        normY2 += std::norm(v);
    }
    normY2 = std::sqrt(normY2);

    // Цикл по сдвигам
    for (size_t shift = 0; shift < maxShift; ++shift) {
        std::complex<float> sum(0.0, 0.0);
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


int find_pss(VecSymbolMod &samples, VecSymbolMod &pss) {

    VecSymbolMod corr_array = convolve(samples, pss);
    print_log(CONSOLE, "samples size - %d, corr_array size = %d\n", samples.size(), corr_array.size());
    write_file_bin_data(
        "../data/corr_array_convolve.bin", 
        (void*)&corr_array[0], 
        sizeof(float) * corr_array.size() * 2);
    return maxElement(corr_array);
    // return std::max_element(corr_array.begin(), corr_array.end()) - corr_array.begin();
}



float normalized_correlation(const VecSymbolMod& signal1, const VecSymbolMod& signal2) {
    if (signal1.size() != signal2.size() || signal1.empty()) {
        throw std::invalid_argument("Input signals must have the same non-zero size.");
    }

    float sum_product = 0.0;
    float sum_sq_signal1 = 0.0;
    float sum_sq_signal2 = 0.0;

    // Вычисляем средние значения
    float mean_signal1 = 0.0;
    float mean_signal2 = 0.0;

    for (size_t i = 0; i < signal1.size(); ++i) {
        mean_signal1 += std::abs(signal1[i]);
        mean_signal2 += std::abs(signal2[i]);
    }
    mean_signal1 /= signal1.size();
    mean_signal2 /= signal2.size();

    for (size_t i = 0; i < signal1.size(); ++i) {
        sum_product += (std::abs(signal1[i]) - mean_signal1) * (std::abs(signal2[i]) - mean_signal2);
        sum_sq_signal1 += std::pow(std::abs(signal1[i]) - mean_signal1, 2);
        sum_sq_signal2 += std::pow(std::abs(signal2[i]) - mean_signal2, 2);
    }

    return sum_product / (std::sqrt(sum_sq_signal1) * std::sqrt(sum_sq_signal2));
}

float normalized_correlation2(const VecSymbolMod& signal1, const VecSymbolMod& signal2) {
    VecSymbolMod corr_array = convolve(signal1, signal2);
    print_log(CONSOLE, "signal1 size - %d, corr_array size = %d\n", signal1.size(), corr_array.size());
    write_file_bin_data(
        "../data/corr_array_convolve.bin", 
        (void*)&corr_array[0], 
        sizeof(float) * corr_array.size() * 2);
    return maxElement(corr_array);
}


int receiver_OFDM1(VecSymbolMod &samples, 
    OFDM_params &param_ofdm, OFDM_symbol &ofdms) 
{
    VecSymbolMod pss = generateZadoffChuSequence(0, LENGTH_PSS);
#if 0
    std::vector<float> corr_array = correlate_PSS(samples, pss);
    write_file_bin_data(
            "../data/corr_array2.bin", 
            (void*)&corr_array[0], 
            sizeof(float) * corr_array.size());
#endif
#if 1
    std::vector<float> corr_array = correlation(samples, pss);
    write_file_bin_data(
            "../data/corr_array_test.bin", 
            (void*)&corr_array[0], 
            sizeof(float) * corr_array.size());
#endif
    int pos_data;
    //  = max_element(corr_array.begin(), corr_array.end()) - corr_array.begin();
    // convolve(samples, pss);
    print_log(CONSOLE, "samples.size() = %d\n", samples.size() );
    // pos_data = correlate(samples, pss);
    pos_data = find_pss(samples, pss);
    for(int i = 0; i < corr_array.size(); ++i) {
        if(corr_array[i] > 0.7f) {
            pos_data = i;
            break;
        }
    }
#if 1
    print_log(LOG_DATA, "rx PSS:\n");
    VecSymbolMod pss_rx(samples.begin() + pos_data - LENGTH_PSS, samples.begin() + pos_data);
    print_VecSymbolMod(pss_rx);

#endif
#if 0
    int pos_data2 = pos_data - 1000;
    VecSymbolMod sofdms(samples.begin() + pos_data2, samples.begin() + pos_data2 + 3528);
    OFDM_symbol f1 = samples_join_OFDM(sofdms, 168, samples.size());
    write_OFDMs("../data/test_ofdm_rx.bin", f1, f1.size);

#endif
#if 1
    int pos_data2 = pos_data;
    VecSymbolMod sofdms(samples.begin() + pos_data2, samples.begin() + pos_data2 + 840);
    OFDM_symbol f1 = samples_join_OFDM(sofdms, 168, samples.size());
    print_log(LOG, "f1 = %d\n", f1.size());
    write_OFDMs("../data/test_ofdm_rx.bin", f1, f1.size());

#endif
    print_log(LOG, "[%s:%d] pos data = %d\n", __func__, __LINE__, pos_data);
    if(pos_data > samples.size() || pos_data < 0) {
        print_log(ERROR_OUT, "Error find pss: %d out range(0, %d)\n", pos_data, samples.size());
        return -1;
    }

#if 1
    int p1 = pos_data;
    int p2 = pos_data + 1000;
    VecSymbolMod test_buffer(samples.begin() + p1, samples.begin() + p2);
    print_log(LOG_DATA, "Slice buffer[%d]:\n", test_buffer.size());
    print_VecSymbolMod(test_buffer);
    print_log(LOG_DATA, "\n");

#endif

    // return;
    int count = 7;
    // std::vector<float> norm_corr_ofdm;
    // std::vector<int> indexes;
    
    // int iter_end = 3;
    // pos_data -= 40;
    // pos_data = 0;
    float activate_sample = 0.3;
    // OFDM_symbol ofdms;
    // pos_data = 100 + 83;
    for(int i = 0; i < count; ++i) {
        // float max_corr = 0;
        // float limit_activate = 0.017;
        // int iter = 0;
        std::vector<float> norm_corr_ofdm;
        std::vector<int> indexes;
        print_log(LOG_DATA, "pos_data = %d, samples.size() = %d\n", pos_data, samples.size());
        for(int j = pos_data; j < samples.size() - 
            (param_ofdm.def_interval + param_ofdm.count_subcarriers); ++j) 
        {
            DEBUG_LINE
            VecSymbolMod prefix(samples.begin() + j, samples.begin() + j + param_ofdm.def_interval);

            VecSymbolMod s(samples.begin() + j + param_ofdm.def_interval, 
                samples.begin() + j + param_ofdm.def_interval + param_ofdm.count_subcarriers);

            VecSymbolMod postfix(samples.begin() + j + param_ofdm.count_subcarriers, 
                samples.begin() + j + param_ofdm.def_interval + param_ofdm.count_subcarriers);
                
            DEBUG_LINE
            float corr = normalized_correlation(prefix, postfix);
            // print_log(CONSOLE, "corr = %f\n", corr);  
            norm_corr_ofdm.push_back(corr);
            indexes.push_back(j);
            if(j > pos_data + 100) {
                print_log(LOG, "limited\n");
                break;
            }
            DEBUG_LINE
#if 0     
            if(corr >= activate_sample) {
                VecSymbolMod ofdm(samples.begin() + j + param_ofdm.def_interval, 
                    samples.begin() + j + param_ofdm.def_interval + param_ofdm.count_subcarriers);
                // VecSymbolMod ofdm(samples.begin() + j + param_ofdm.def_interval, 
                //     samples.begin() + j + param_ofdm.def_interval + 
                //     param_ofdm.count_subcarriers + param_ofdm.cyclic_prefix);
                
                // print_log(CONSOLE, "size ofdm - %d\n", ofdm.size());
                pos_data = j + param_ofdm.count_subcarriers;
                ofdms.push_back(ofdm);

                break;
            }
#endif
        }
#if 1
        DEBUG_LINE
        auto max_corr = std::max_element(norm_corr_ofdm.begin(), norm_corr_ofdm.end());
        DEBUG_LINE
        // norm_corr_ofdm = 100;
        print_log(LOG, "norm_corr_ofdm - %d\n", norm_corr_ofdm.size());
        if(norm_corr_ofdm.size() == 0) {
            break;
        }
        int index_mc = std::distance(norm_corr_ofdm.begin(), max_corr);
        index_mc = 100;
        DEBUG_LINE
        print_log(CONSOLE, "size indexes - %d, max corr = %f, index_mc = %d\n",
            indexes.size(), *max_corr, index_mc);
        
        index_mc = indexes[index_mc];
        DEBUG_LINE
        
        if(*max_corr >= activate_sample) {
            VecSymbolMod ofdm(samples.begin() + index_mc + param_ofdm.def_interval, 
                samples.begin() + index_mc + param_ofdm.def_interval + param_ofdm.count_subcarriers);
            // VecSymbolMod ofdm(samples.begin() + j + param_ofdm.def_interval, 
            //     samples.begin() + j + param_ofdm.def_interval + 
            //     param_ofdm.count_subcarriers + param_ofdm.cyclic_prefix);
            
            // print_log(CONSOLE, "size ofdm - %d\n", ofdm.size());
            pos_data = index_mc + param_ofdm.count_subcarriers;
            ofdms.push_back(ofdm);
            // break;
        }
        DEBUG_LINE
#endif 
    }

    // write_file_bin_data(
    //         "../data/norm_corr_ofdm.bin", 
    //         (void*)&norm_corr_ofdm[0], 
    //         sizeof(float) * norm_corr_ofdm.size());
    if(ofdms.size() != count) {
        print_log(CONSOLE, "Not all ofdms symbol are accepted - %d\n", ofdms.size());
        if(ofdms.size() > 0) {
            write_OFDMs("../data/read_ofdms.bin", ofdms, ofdms.size());
        }
        return -1;
    } else {
        write_OFDMs("../data/read_ofdms.bin", ofdms, ofdms.size());
        // VecSymbolMod rtert = OFDM_demodulator(ofdms, param_ofdm, false);
    }
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);
    print_log(CONSOLE, "count ofdms symbol = %d\n", ofdms.size());
    return 0;
}

enum class STATE_RECV_OFDM {
    FIND_PSS,
    FIND_OFDM,
    EXIT,
};

int receiver_OFDM(VecSymbolMod &samples, 
    OFDM_params &param_ofdm, OFDM_symbol &ofdms) 
{
    float activate_find_pss = 0.7;
    int count_send_ofdm = 7;
    int pos_data = -1;
    VecSymbolMod pss = generateZadoffChuSequence(0, LENGTH_PSS);
#if 0
    std::vector<float> corr_array = correlate_PSS(samples, pss);
    write_file_bin_data(
            "../data/corr_array2.bin", 
            (void*)&corr_array[0], 
            sizeof(float) * corr_array.size());
#endif
#if 1
    std::vector<float> corr_array = correlation(samples, pss);
    write_file_bin_data(
            "../data/corr_array_test.bin", 
            (void*)&corr_array[0], 
            sizeof(float) * corr_array.size());
#endif
    //  = max_element(corr_array.begin(), corr_array.end()) - corr_array.begin();
    // convolve(samples, pss);
    print_log(CONSOLE, "samples.size() = %d\n", samples.size() );
    // pos_data = correlate(samples, pss);
    // pos_data = find_pss(samples, pss);
    // for(int i = 0; i < corr_array.size(); ++i) {
    //     if(corr_array[i] > 0.7f) {
    //         pos_data = i;
    //         break;
    //     }
    // }
    bool run = true;
    STATE_RECV_OFDM state = STATE_RECV_OFDM::FIND_PSS;
    int pos_iter = 0;
    int status = 0;
    int iter_pos_ofdm;
    int slice;
    int count_pss = 0;
    while (run) {
        print_log(LOG, "[%s:%d] \t\tstate: %d\n", __func__, __LINE__, static_cast<int>(state));
            
        switch (state)
        {
        case STATE_RECV_OFDM::FIND_PSS:
#if 1
            if(count_pss == 1) {
                run = false;
                break;
            }

#endif

            pos_data = -1;
            print_log(CONSOLE, "[%s:%d] pos_iter - %d\n", __func__, __LINE__, pos_iter);
            for(int i = pos_iter; i < corr_array.size(); ++i) {
                if(corr_array[i] > activate_find_pss) {
                    pos_data = i + pss.size();
                    break;
                }
            }
            print_log(LOG, "[%s:%d] pos_data: %d\n", __func__, __LINE__, pos_data);
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
                    print_log(CONSOLE, "Out of range buffer: slice [%d:%d], buffer size - %d\n",
                        iter_pos_ofdm, slice, samples.size());
                    state = STATE_RECV_OFDM::EXIT;
                    break;
                }
                print_log(CONSOLE, "[%s:%d] add ofdm symbol - %d, [%d:%d]\n",
                    __func__, __LINE__, ofdms.size(), iter_pos_ofdm, slice);
                ofdms.push_back(
                    VecSymbolMod(
                        samples.begin() + iter_pos_ofdm, 
                        samples.begin() + slice));
                iter_pos_ofdm += param_ofdm.count_subcarriers;
            }
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
    if(ofdms.size() != count_send_ofdm) {
        print_log(CONSOLE, "Not all ofdms symbol are accepted - %d\n", ofdms.size());
        if(ofdms.size() > 0) {
            write_OFDMs("../data/read_ofdms.bin", ofdms, ofdms.size());
        }
        return -1;
    } else {
        write_OFDMs("../data/read_ofdms.bin", ofdms, ofdms.size());
        // VecSymbolMod rtert = OFDM_demodulator(ofdms, param_ofdm, false);
    }
    return 0;
}








