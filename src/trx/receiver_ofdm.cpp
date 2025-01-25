#include "receiver_ofdm.hpp"

#include <algorithm>
#include <fftw3.h>

#include <iostream>
#include <vector>
#include <complex>
#include <numeric>
#include <cmath>
#include <stdexcept>

// #include "../ipc/managment_ipc.hpp"

#include <output.hpp>
#include <complex_container.hpp>
#include <signal_processing.hpp>

#define LENGTH_PSS 63

using namespace DIGITAL_SIGNAL_PROCESSING;

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

#if 1
    std::vector<float> corr_array = correlation(samples, pss);
    write_file_bin_data(
            "../data/corr_array_test.bin", 
            (void*)&corr_array[0], 
            sizeof(float) * corr_array.size());
#endif

    print_log(CONSOLE, "samples.size() = %d\n", samples.size() );

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
                VecSymbolMod ofdm_symbol(samples.begin() + iter_pos_ofdm,
                    samples.begin() + slice);
                // evaluation_cfo_one(ofdm_symbol, param_ofdm);
                ofdms.push_back(ofdm_symbol);
                // ofdms.push_back(
                //     VecSymbolMod(
                //         samples.begin() + iter_pos_ofdm, 
                //         samples.begin() + slice));
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
    if(ofdms.size() > 0) {
        evaluation_cfo(ofdms, param_ofdm);
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








