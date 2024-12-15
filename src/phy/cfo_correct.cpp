#include "cfo_correct.hpp"

#include <algorithm>
#include <fftw3.h>

#include <iostream>
#include <vector>
#include <complex>
#include <numeric>
#include <cmath>
#include <stdexcept>

#define EXP 2.718281828459
#define PI 3.141592

int evaluation_cfo_one(VecSymbolMod &samples, const OFDM_params &param_ofdm) {
    VecSymbolMod prefix = VecSymbolMod(
        samples.begin(), samples.begin() + param_ofdm.cyclic_prefix);
    VecSymbolMod postfix = VecSymbolMod(
        samples.begin() + param_ofdm.count_subcarriers,
        samples.begin() + param_ofdm.count_subcarriers + param_ofdm.cyclic_prefix);
    
    print_log(LOG, "[%s:%d] size prefix - %d, size postfix - %d\n",
        __func__, __LINE__, prefix.size(), postfix.size());
    mod_symbol sum = {0, 0};
    for(int i = 0; i < prefix.size(); ++i) {
        sum += prefix[i] * postfix[i];
    }
    print_log(LOG, "[%s:%d] sum: %f %f\n",
        __func__, __LINE__, sum.real(), sum.imag());
    for(int i = 0; i < samples.size(); ++i) {
        mod_symbol val = {0, -1};
        mod_symbol val2 = {2, 0};
        mod_symbol val3 = {PI, 0};
        mod_symbol val4 = {323, 0};
        samples[i] = samples[i] * std::exp(val * val2 * val3 * sum);
        // mod_symbol e_ = {1.f / (2.f * PI), 0};
        // e_ *= sum;
    }
}

float mean(const std::vector<float>& numbers) {
    float sum = 0.0f;
    for (const auto& num : numbers) {
        sum += num;
    }
    return sum / numbers.size();
}
#if 1
int evaluation_cfo(OFDM_symbol &ofdms, const OFDM_params &param_ofdm) {
    VecSymbolMod est_s;
    std::vector<float> est_s2;
    for(int j = 0; j < ofdms.size(); ++j) {
        VecSymbolMod &samples = ofdms[j];
        VecSymbolMod prefix = VecSymbolMod(
            samples.begin(), samples.begin() + param_ofdm.cyclic_prefix);
        VecSymbolMod postfix = VecSymbolMod(
            samples.begin() + param_ofdm.count_subcarriers,
            samples.begin() + param_ofdm.count_subcarriers + param_ofdm.cyclic_prefix);
        
        // print_log(LOG, "[%s:%d] size prefix - %d, size postfix - %d\n",
        //     __func__, __LINE__, prefix.size(), postfix.size());
        mod_symbol sum = {0, 0};
        for(int i = 0; i < prefix.size(); ++i) {
            sum += prefix[i] * postfix[i];
        }
        est_s.push_back(sum);
        
        float angle = std::arg(sum);
        float est = (1.f / (2.f * PI)) * angle;
        est_s2.push_back(est);
        print_log(LOG, "[%s:%d] sum: %f %f, angle: %f, est: %f\n",
            __func__, __LINE__, sum.real(), sum.imag(), angle, est);

        
        // for(int i = 0; i < samples.size(); ++i) {
        //     mod_symbol val = {0, -1};
        //     mod_symbol val2 = {2, 0};
        //     mod_symbol val3 = {PI, 0};
        //     mod_symbol val4 = {323, 0};
            
            
        //     // samples[i] = samples[i] * std::exp(val * val2 * val3 * sum);
        //     // mod_symbol e_ = {1.f / (2.f * PI), 0};
        //     // e_ *= sum;
            
        // }
    }
    write_file_bin_data("../data/est_cfo.bin", 
            (void*)&est_s2[0], est_s2.size() * sizeof(float));
    
    auto max_elm_it = MAX_ELM_VEC(est_s);
    mod_symbol max_elm = *max_elm_it;
    

    float est_mean = mean(est_s2);
    // est_mean = *std::min_element(est_s2.begin(), est_s2.end());
    // est_mean = ((double)est_mean * (double)1.9e6 / (double)168.f);
    // print_log(LOG, "[%s:%d] max_elm: %f %f, mean: %f\n",
    //         __func__, __LINE__, max_elm.real(), max_elm.imag(), est_mean);
#if 0
    for(int i = 0; i < ofdms.size(); ++i) {
        VecSymbolMod &samples = ofdms[i];
        mod_symbol val = {0, -1};
        mod_symbol val2 = {2, 0};
        mod_symbol val3 = {PI, 0};
        mod_symbol val4 = {est_mean, 0};
        mod_symbol val5 = {est_mean, 0};
        
        
        samples[i] = samples[i] * std::exp(val * val2 * val3 * val4);
        // samples[i] = samples[i] * std::exp(val * val2 * val3 * max_elm);
        // mod_symbol e_ = {1.f / (2.f * PI), 0};
        // e_ *= sum;
        
    }
#else
    for(int i = 0; i < ofdms.size(); ++i) {
        float est_mean = est_s2[i];
        VecSymbolMod &ofdm = ofdms[i];
        mod_symbol val = {0, -1};
        mod_symbol val2 = {2, 0};
        mod_symbol val3 = {PI, 0};
        mod_symbol val4 = {est_mean, 0};
        // mod_symbol val5 = {i * ofdm.size(), 0};
        
        for(int j = 0; j < ofdm.size(); ++j) {
            ofdm[j] = ofdm[j] * std::exp(val * val2 * val3 * val4 *\
                mod_symbol((float)(i * ofdm.size() + j) / (float)(ofdms.size() * ofdm.size()), 0));
        }
        mod_symbol ttt = mod_symbol( (float)(ofdms.size() * ofdm.size()));
        print_log(LOG, "[%s:%d] ttt: %f %f, mean: %f\n",
            __func__, __LINE__, ttt.real(), ttt.imag(), est_mean);       
        // samples[i] = samples[i] * std::exp(val * val2 * val3 * max_elm);
        // mod_symbol e_ = {1.f / (2.f * PI), 0};
        // e_ *= sum;
        
    }
#endif
    // VecSymbolMod 
}

#else

int evaluation_cfo(OFDM_symbol &ofdms, const OFDM_params &param_ofdm) {
    std::vector<float> est_s2;
    for(int j = 0; j < ofdms.size(); ++j) {
        VecSymbolMod &samples = ofdms[j];
        VecSymbolMod prefix = VecSymbolMod(
            samples.begin(), samples.begin() + param_ofdm.cyclic_prefix);
        VecSymbolMod postfix = VecSymbolMod(
            samples.begin() + param_ofdm.count_subcarriers,
            samples.begin() + param_ofdm.count_subcarriers + param_ofdm.cyclic_prefix);
        mod_symbol sum = {0, 0};
        for(int i = 0; i < prefix.size(); ++i) {
            sum += prefix[i] * postfix[i];
        }
        float angle = std::arg(sum);
        float est = (1.f / (2.f * PI)) * angle;
        est_s2.push_back(est);
    }
    float est_mean = mean(est_s2);
    for(int i = 0; i < ofdms.size(); ++i) {
        VecSymbolMod &ofdm = ofdms[i];
        mod_symbol val = {0, -1};
        mod_symbol val2 = {2, 0};
        mod_symbol val3 = {PI, 0};
        mod_symbol val4 = {est_mean, 0};
        for(int j = 0; j < ofdm.size(); ++j) {
            ofdm[j] = ofdm[j] * std::exp(val * val2 * val3 * val4 *\
                mod_symbol((float)(i * ofdm.size() + j) / (float)(ofdms.size() * ofdm.size()), 0));
        }
        mod_symbol ttt = mod_symbol( (float)(ofdms.size() * ofdm.size()));
    }
}

#endif


