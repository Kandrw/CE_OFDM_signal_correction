#include "modelling.hpp"
#include <complex>
#include <random>
#include <vector>
#include <cmath>
#include <cstring>

#include <complex>
#include "../phy/types.hpp"
#include "../loaders/load_data.hpp"
#include "../phy/modulation.hpp"


#define PI 3.1415926535

#define FILE_SER "../data/ser.txt"
#define FILE_BER_SOFT "../data/practice5/ber_soft.txt"


#define PRINT_BITS( mask, shift)\
(bits & mask) >> shift

#define SHIFT 8
void print_bit_char(u_char a, int count_bit){
    int i;
    for(i = count_bit-1; i >= 0; --i){
        print_log(CONSOLE, "%d", ((a & (u_char)pow(2, i)) > 0?1:0));

    }
    //fprintf(out, " ");
}
void print_bit2(u_char *p, int N){
    int i;

    for(i = 0; i < N; ++i){
        print_bit_char(*p, SHIFT);
        //print_bit_char(*p, SHIFT, stdout); printf("  %d %c\n", *p, *p);
        ++p;
    }
    print_log(CONSOLE, "\n");
}


VecSymbolMod add_Gaussian_noise(VecSymbolMod &samples, float dispersion){
            
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> d(0.0, dispersion);
    VecSymbolMod samples_noise;
    for (int i = 0 ; i < (int)samples.size(); i++){
        mod_symbol t1 = {d(gen) + samples[i].real(), 
                            d(gen) + samples[i].imag()};
        samples_noise.push_back(t1);
    }
    return samples_noise;
}

static std::mt19937 generator(std::random_device{}()); 
std::normal_distribution<double> distribution(0.0, 1.0); 

double randn() {
    return distribution(generator);
}

// Генерация случайного числа с нормальным распределением (с заданным средним и стандартным отклонением)
double randn(double mean, double stddev) {
    return mean + stddev * distribution(generator);
}




#define GET_RANDOM_VAL_IN_ARRAY(A, SIZE)\
A[ (int)(rand() %  SIZE) ]

VecSymbolMod generate_random_QAM16(int N) {
    VecSymbolMod samples;

    static int qam1[] = {-3, -1, 1, 3};
    for(int i = 0; i < N; ++i){
        mod_symbol s = {
            (float)GET_RANDOM_VAL_IN_ARRAY(qam1, 4),
            (float)GET_RANDOM_VAL_IN_ARRAY(qam1, 4),
        };
        samples.push_back(s);
    }
    return samples;
}

float calc_Ps(int N, const VecSymbolMod &S) {

    float sum = 0.f;
    for(int i = 0; i < (int)S.size(); ++i){
        sum += (S[i].real() * S[i].real() + S[i].imag() * S[i].imag());
    }

    return (float)(1.0 / (float(N))) * sum;
}

VecSymbolMod generate_noise_by_SNR(int N, float Q_2) {
    VecSymbolMod n;
    float koeff = sqrt(Q_2 / 2.f);
    for(int i = 0; i < N; ++i) {
        mod_symbol ms = {(float)randn(0, 1) * koeff, 
                (float)randn(0, 1) * koeff};
        n.push_back(ms);
    }
    return n;
}

std::vector<float> generate_channel_relay_channel(float mat, float Q2, int N) {
    int i;
    std::vector<float> h;
    for(i = 0; i < N; ++i){
        float hi = randn(mat, Q2);
        float hq = randn(mat, Q2);

        float value = sqrtf(hi + hq);
        h.push_back(value);
        // print_log(LOG_DATA, "h = %f\n", h);
    }
    return h;
}

VecSymbolMod convolve1(VecSymbolMod s, VecSymbolMod h) {
    VecSymbolMod r(s.size());
    for(int i = 0; i < s.size(); ++i) {
        for(int j = 0; j < h.size(); ++j) {
            if (i-j >= 0 && i-j < s.size()){
                r[i] += s[i-j] * h[j];
            }
        }
    }
    VecSymbolMod zeros(h);
    r.insert(r.end(), h.begin(), h.end());
    return r;
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
int calc_error(VecSymbolMod &tx, VecSymbolMod &rx) {
    int ce = 0;
    float im, re;
    float interval = 1.5;
    for(int i = 0; i < (int)tx.size(); ++i) {        
        float dist = sqrt(powf(tx[i].real() - rx[i].real(), 2) + powf(tx[i].imag() - rx[i].imag(), 2));
        if(dist > interval) {
            // print_log(CONSOLE, "tx: %f %f, rx: %f %f\n", 
            //     tx[i].real(), tx[i].imag(), rx[i].real(), rx[i].imag());

            ce += 1;
        }
    }
    return ce;
}

int calc_error1(VecSymbolMod &tx, VecSymbolMod &rx) {
    int ce = 0;
    float im, re;
    float interval = 0.4;
    for(int i = 0; i < (int)tx.size(); ++i) {
        im = tx[i].real() - rx[i].real();
        re = tx[i].imag() - rx[i].imag();
        // print_log(CONSOLE, "tx: %f %f, rx: %f %f\n", tx[i].real(), tx[i].imag(), rx[i].real(), rx[i].imag());
        // print_log(CONSOLE, "i: %f, q: %f\n", i, q);
        if( !(im < interval && im > interval * -1.0f) ) {
            ce += 1;
        } else {
            if( (re > interval || re < interval * -1.0f) ) {
                ce += 1;
            }
        }
    }
    return ce;
}

// void print_VecSymbolMod( VecSymbolMod &vec) {
//     for(int i = 0; i < (int)vec.size(); ++i) {
//         print_log(CONSOLE, "%f + %fi  ", vec[i].real(), vec[i].imag());
//     }
//     print_log(CONSOLE, "\n");
// }

/*3 Практика*/
void model_calc_P_SER() {
    
    // int P_err = 10e-1;
    int N = 70000;

    int snr_start = 0;
    int snr_end = 25;
    int i;
    float h2;
    print_to_file(FILE_SER, "w", "");
    for(i = snr_start; i <= snr_end; ++i) {
        int count_err = 0;
        int SNR = i;
        VecSymbolMod samples = generate_random_QAM16(N);
        float Ps = calc_Ps(N, samples);
        h2 = pow(10, SNR * 0.1);
        float Q2 = Ps / h2;

        VecSymbolMod n = generate_noise_by_SNR(N, Q2);
        VecSymbolMod r = samples + n;
   
        print_log(CONSOLE, "%d\n", i);
        print_log(CONSOLE, "Ps: %f, s: %f %f\n", Ps, samples[0].real(), samples[0].imag());
        
        print_log(CONSOLE, "h2 = %f\n", h2);        
        
        count_err = calc_error(samples, r);
        
        float Pser = (float)count_err / (float)N;
        print_log(CONSOLE, "Pser = %f, count_err = %d\n", Pser, count_err);
        print_to_file(FILE_SER, "a", "%f %d\n", Pser, SNR);
        char filename[30];
        memset(filename, 0, sizeof(filename));
        sprintf(filename, "../data/samples/rx_%d", (int)SNR);
        write_file_bin_data(
                filename, &r[0], 
                r.size() * sizeof(VecSymbolMod::value_type) );
        samples.clear();

    }
}
/*4 Практика*/

float calc_part_equation(float y0, float y, int b, float q) {
#if 0
    print_log(CONSOLE, "\t\t1.f / (sqrt(2.f * (float)PI) * q) = %f\n",
        1.f / (sqrt(2.f * (float)PI * q)));
    print_log(CONSOLE, "\t\t(( -1.f * (y - y0) * (y - y0))"
    " / (2.f * powf(q, 2))) = %f\n",
        (( -1.f * (y - y0) * (y - y0)) / (2.f * powf(q, 2))));
    print_log(CONSOLE, "\t\t e ^ %f = %f\n", ( -1.f * (y - y0) * (y - y0)) / (2 * q),

        powf(2.7182818, 
            (( -1.f * (y - y0) * (y - y0)) / (q)))
        );
#endif
#if 0
    return 1.f / (sqrt(2.f * (float)PI) * q) * \
        powf(2.7182818, 
            (( -1.f * (y - y0) * (y - y0)) / (2.f * powf(q, 2))) );
#else
    return 1.f / (sqrt(2.f * (float)PI * q)) * \
        powf(2.7182818f, 
            (( -1.f * (y - y0) * (y - y0)) / (2.f * q)) );
#endif
}

float calc_P_y_b(float y01, float y02, float y, int b, float q) {
    float P = calc_part_equation(y01, y, b, q);
    P += calc_part_equation(y02, y, b, q);
#if 0
    print_log(CONSOLE, "\t|| %f + %f\n", 
    calc_part_equation(y01, y, b, q), calc_part_equation(y02, y, b, q));
#endif
    return P;
}

int calc_bit_error(bit_sequence &tx, bit_sequence &rx) {
    int error = 0;
    
    // print_bit2((u_char*)tx.buffer, tx.size);
    // print_bit2((u_char*)rx.buffer, rx.size);
    for(int i = 0; i < tx.size; ++i) {
        u_char otx = tx.buffer[i];
        u_char orx = rx.buffer[i];
        for(int i2 = 0; i2 < 8; ++i2) {
            u_char bt = otx & 0b1;
            u_char br = orx & 0b1;
            
            if(bt != br) {
                error++;
            }
            otx >>= 1;
            orx >>= 1;
        }
    }
    return error;
}

int calc_hard_error(bit_sequence &tx, bit_sequence &rx) {
    int error = 0;
    
    // print_bit2((u_char*)tx.buffer, tx.size);
    // print_bit2((u_char*)rx.buffer, rx.size);
    for(int i = 0; i < tx.size; ++i) {
        u_char otx = tx.buffer[i];
        u_char orx = rx.buffer[i];
        for(int i2 = 0; i2 < 8; ++i2) {
            u_char bt = otx & 0b1;
            u_char br = orx & 0b1;
            
            if(bt != br) {
                error++;
            }
            otx >>= 1;
            orx >>= 1;
        }
    }
    return error;
}

bit_sequence *decode_soft_solutions(VecSymbolMod &samples_rx, float Q2, bool debug) {
    int con_state = 0;
    int N = samples_rx.size();
    bit_sequence *soft_result = new bit_sequence;
    soft_result->size = samples_rx.size() / 2;
    soft_result->buffer = new u_char[soft_result->size];
    memset(soft_result->buffer, 0, soft_result->size);
    u_char *step = soft_result->buffer;
    for(int i = 0; i < N; ++i) {

        u_char bits = 0;
        float B0_r0 = calc_P_y_b(-3.f, 1.f, samples_rx[i].real(), 0, Q2);
        float B0_r1 = calc_P_y_b(-1.f, 3.f, samples_rx[i].real(), 1, Q2);
        float B1_r0 = calc_P_y_b(-3.f, -1.f, samples_rx[i].real(), 0, Q2);
        float B1_r1 = calc_P_y_b(1.f, 3.f, samples_rx[i].real(), 1, Q2);
        float B2_r0 = calc_P_y_b(1.f, -3.f, samples_rx[i].imag(), 0, Q2);
        float B2_r1 = calc_P_y_b(3.f, -1.f, samples_rx[i].imag(), 1, Q2);
        float B3_r0 = calc_P_y_b(-1.f, -3.f, samples_rx[i].imag(), 0, Q2);
        float B3_r1 = calc_P_y_b(1.f, 3.f, samples_rx[i].imag(), 1, Q2);

        if(B0_r0 > B0_r1) bits = 0b0000;
        else bits = 0b1000;
        if(B1_r0 > B1_r1) bits |= 0b0000;
        else bits |= 0b0100;
        if(B2_r0 > B2_r1) bits |= 0b0000;
        else bits |= 0b0010;
        if(B3_r0 > B3_r1) bits |= 0b0000;
        else bits |= 0b0001;

        switch (con_state) {
        case 0:
            *step |= (bits << 4);
            con_state = 1;
            break;
        default:
            *step |= (bits);
            ++step;
            con_state = 0;
            break;
        }
        if(debug && 0) {
            print_log(CONSOLE, "[%d] r[i](i,q) = (%f %f)\n", 
                i, samples_rx[i].real(), samples_rx[i].imag());
            print_log(CONSOLE, "B0=0: %f, B0=1: %f\n", B0_r0, B0_r1);
            print_log(CONSOLE, "B1=0: %f, B1=1: %f\n", B1_r0, B1_r1);
            print_log(CONSOLE, "B2=0: %f, B2=1: %f\n", B2_r0, B2_r1);
            print_log(CONSOLE, "B3=0: %f, B3=1: %f\n", B3_r0, B3_r1);

            print_log(CONSOLE, "0b%d%d%d%d = %d\n", 
            PRINT_BITS(0b1000, 3), PRINT_BITS(0b0100, 2),
            PRINT_BITS(0b0010, 1), PRINT_BITS(0b0001, 0), bits);
        }
    }
    return soft_result;
}

void model_soft_solutions() {

#define TEST_DATA 2
#if TEST_DATA == 0
    // char test_data[] = "Test message ";
    char test_data[] = "A";
#elif TEST_DATA == 1   
    char test_data[90];
    for(int i = 0; i < sizeof(test_data); ++i){
        test_data[i] = (rand() % 200);
        // test_data[i] = (rand() % 20) + 70;
    }
    test_data[sizeof(test_data) - 1] = 0;
#endif
    const char filename[] = "../data/data_test.txt";
    
    bit_sequence data, data_grey;

    

    int SNR_min = 0;
    int SNR_max = 30;
    float h2;
    
    // print_bit2((u_char*)data_grey.buffer, data_grey.size);
    // int count_samples = 100000;
    print_to_file(FILE_BER_SOFT, "w", "");
    for(int i = SNR_min; i <= SNR_max; ++i) {
        
#if TEST_DATA == 2
        char test_data[100000];
        for(int i = 0; i < (int)sizeof(test_data); ++i){
            test_data[i] = (rand() % 200);
            // test_data[i] = (rand() % 20) + 70;
        }
        test_data[sizeof(test_data) - 1] = 0;
#endif
        write_file_bin_data(filename, test_data, sizeof(test_data));
        data.buffer = read_file_data(filename, &data.size);
        VecSymbolMod samples = modulation_mapper(data, TypeModulation::QAM16);
        // VecSymbolMod samples = generate_random_QAM16(count_samples);
        float Ps = calc_Ps(samples.size(), samples);
        data_grey.buffer = read_file_data(CODE_GREY, &data_grey.size);

        float SNR = i;
        h2 = powf(10.f, (float)SNR * 0.1);
        float Q2 = Ps / h2;
        VecSymbolMod n = generate_noise_by_SNR(samples.size(), Q2);
        VecSymbolMod r = samples + n;
        
        // print_log(CONSOLE, "%f %f - %f %f\n",
        //     samples[0].real(), samples[0].imag(), r[0].real(), r[0].imag());
#if 0
        write_file_bin_data(
                    "../data/practiec5/s.bin", &samples[0], 
                    samples.size() * sizeof(VecSymbolMod::value_type) );
        write_file_bin_data(
                    "../data/practice5/s_noise.bin", &r[0], 
                    r.size() * sizeof(VecSymbolMod::value_type) );

        print_log(CONSOLE, "\n[%s:%d] q = %f\n", __func__, __LINE__, Q2);
#endif

        bit_sequence *soft_result = decode_soft_solutions(r, Q2, false);
        bit_sequence *hard_result = demodulation_mapper(r, TypeModulation::QAM16);

#if 0
        print_bit2((u_char*)data_grey.buffer, data_grey.size);
        print_bit2((u_char*)soft_result->buffer, soft_result->size);
        print_bit2((u_char*)hard_result->buffer, hard_result->size);
#endif

        int error_soft = calc_bit_error(data_grey, *soft_result);
        int error_hard = calc_bit_error(data_grey, *hard_result);
        
        print_log(CONSOLE, "error_soft = %d, error_hard = %d\n", error_soft, error_hard);

        print_to_file(FILE_BER_SOFT, "a", "%f %f %d\n", 
            (float)error_soft / ((float)data_grey.size * 8.f), 
            (float)error_hard / ((float)data_grey.size * 8.f), 
            (int)SNR);

        delete[] soft_result->buffer;
        delete soft_result;
        delete[] data_grey.buffer;
        delete[] data.buffer;
        samples.clear();
    }
    // print_log(CONSOLE, "res: %s\n", soft_result->buffer);

    // print_bit2((u_char*)test_data, sizeof(test_data));
    // print_bit2((u_char*)data_grey.buffer, data_grey.size);
    // print_bit2((u_char*)soft_result->buffer, soft_result->size);

    print_log(CONSOLE, "\n[%s:%d] End\n", __func__, __LINE__);
}

#define LOG_FILE "../log.log"

int test_base(){
	const char filename[] = "../data/data_test.txt";
	char test_data[9] = {
        (char)0b00000001,
        (char)0b00100011,
        (char)0b01000101,
        (char)0b01100111,
        (char)0b10001001,
        (char)0b10101011,
        (char)0b11001101,
        (char)0b11101111
    };

    test_data[sizeof(test_data) - 1] = 0;
	int size = sizeof(test_data) - 1;
	write_file_bin_data(filename, test_data, size);

    // Sleep(4 * 1000);
    bit_sequence data;

    data.buffer = read_file_data(filename, &data.size);
    if(data.size == 0){
        print_log(CONSOLE, "[ERROR] [main.cpp] main: empty data_bin, EXIT\n");
        return -1;
    }
    OFDM_params param_ofdm = {

    };
    ParamsPhy param_phy = {
        .type_modulation = TypeModulation::QAM16,
        .param_ofdm = param_ofdm,
    };
    print_log(LOG_DATA, "size = %d, data: %s\n", data.size, data.buffer);
    // VecSymbolMod samples = generate_frame_phy(data, param_phy);
    return 0;
}

int model_relay_channel() {

        // int P_err = 10e-1;
    int N = 200000;

    int snr_start = 0;
    int snr_end = 60;
    int i;
    float h2;
    print_to_file(FILE_SER, "w", "");
    for(i = snr_start; i <= snr_end; ++i) {
        int count_err = 0;
        int count_err2 = 0;
        
        int SNR = i;
        VecSymbolMod samples = generate_random_QAM16(N);
        // VecSymbolMod samples2 = samples;
        float Ps = calc_Ps(N, samples);
        h2 = pow(10, SNR * 0.1);
        float Q2 = Ps / h2;
        
        VecSymbolMod n = generate_noise_by_SNR(N, Q2);
        std::vector<float> h = generate_channel_relay_channel(0, 0.5, N);
        VecSymbolMod r = samples + n;
        VecSymbolMod r2 = (samples * h) + n;
        VecSymbolMod r3 = r2 / h;
        r2.clear();
        print_log(CONSOLE, "%d\n", i);
        print_log(CONSOLE, "Ps: %f, s: %f %f\n", Ps, samples[0].real(), samples[0].imag());
        
        print_log(CONSOLE, "h2 = %f\n", h2);        
        count_err = calc_error(samples, r);
        count_err2 = calc_error(samples, r3);

        
        float Pser = (float)count_err / (float)N;
        print_log(CONSOLE, "Pser = %f, count_err = %d\n", Pser, count_err);
        float Pser2 = (float)count_err2 / (float)N;
        print_log(CONSOLE, "Pser2 = %f, count_err2 = %d\n", Pser2, count_err2);
        
        print_to_file(FILE_SER, "a", "%f %d %f\n", Pser, SNR, Pser2);
#if 0
        char filename[30];
        memset(filename, 0, sizeof(filename));
        sprintf(filename, "../data/samples/rx_%d", (int)SNR);
        write_file_bin_data(
                filename, &r[0], 
                r.size() * sizeof(VecSymbolMod::value_type) );
#endif     
        samples.clear();

    }
    return 0;
}

void toeplitz1(VecSymbolMod h, int n) {
    std::vector<VecSymbolMod> h_matrix;
    VecSymbolMod sl(n);
    int c = 1;
    // return;
    for(int i = 0; i < n; i++){
        int r = 0;
        for(int j = 0; j < (h.size() / 2) - c; ++j) {
            print_log(CONSOLE, "[%s:%d] j = %d\n", __func__, __LINE__, j);
            sl[j] = mod_symbol(0, 0);
        }
        for(int j = (h.size() / 2), k = 0; j < n, k < h.size(); ++j, ++k, r++) {
            print_log(CONSOLE, "[%s:%d] j = %d\n", __func__, __LINE__, j);
            sl[j] = h[k];
            
        }
        for(int j = r; j < n; ++j) {
            print_log(CONSOLE, "[%s:%d] j = %d\n", __func__, __LINE__, j);
            sl[j] = mod_symbol(0, 0);
            
        }
        c--;
        h_matrix.push_back(sl);
    }
    print_log(CONSOLE, "h_matrix:\n");
    for(int i = 0; i < n; ++i) {
        print_VecSymbolMod(h_matrix[i]);
    }
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);
}

void toeplitz2(VecSymbolMod h, int n) {
    std::vector<VecSymbolMod> h_matrix;
    VecSymbolMod sl(n);
    int c = 1;
    // return;
    for(int i = 0; i < n; i++){
        int r = 0;


        for(int j = n - 1; j >= 0; --j) {

        }
        for(int j = (h.size() / 2); j >= 0; --j) {
            print_log(CONSOLE, "[%s:%d] j = %d\n", __func__, __LINE__, j);

        }

        // for(int j = 0; j < (h.size() / 2); ++j) {
        //     print_log(CONSOLE, "[%s:%d] j = %d\n", __func__, __LINE__, j);
        //     sl[j] = mod_symbol(0, 0);
        // }
        // for(int j = (h.size() / 2), k = 0; j < n, k < h.size(); ++j, ++k, r++) {
        //     print_log(CONSOLE, "[%s:%d] j = %d\n", __func__, __LINE__, j);
        //     sl[j] = h[k];
            
        // }
        // for(int j = r; j < n; ++j) {
        //     print_log(CONSOLE, "[%s:%d] j = %d\n", __func__, __LINE__, j);
        //     sl[j] = mod_symbol(0, 0);
            
        // }
        c--;
        h_matrix.push_back(sl);
    }
    print_log(CONSOLE, "h_matrix:\n");
    for(int i = 0; i < n; ++i) {
        print_VecSymbolMod(h_matrix[i]);
    }
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);
}


void toeplitz3(const VecSymbolMod& h, int n) {
    // Создаем матрицу Топлица размером n x n 
    std::vector<VecSymbolMod> matrix(n, VecSymbolMod(n, mod_symbol(0, 0)));

    // Заполняем матрицу Топлица
    // for (int i = 0; i < n; ++i) {
    //     for (int j = n - 1; j >= 0; --j) {
    //         if (i - j >= 0) {
    //             matrix[i][j] = h[i - j]; // Заполняем значениями из h
    //         } else {
    //             matrix[i][j] = h[n + (i - j)]; // Заполняем для отрицательных индексов
    //         }
    //     }
    // }
    for (int i = n - 1; i >= 0; --i) {
        for (int j = n - 1; j >= 0; --j) {
            if (i - j >= 0) {
                matrix[i][j] = h[i - j]; // Заполняем значениями из h
            } else {
                matrix[i][j] = h[n + (i - j)]; // Заполняем для отрицательных индексов
            }
        }
    }
    print_log(CONSOLE, "h_matrix:\n");
    for(int i = 0; i < n; ++i) {
        print_VecSymbolMod(matrix[i]);
    }
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);

}

void toeplitz(const VecSymbolMod& h, int n) {
    // Создаем матрицу Топлица размером n x n
    // std::vector<std::vector<std::complex<float>>> matrix(n, std::vector<std::complex<float>>(n, std::complex<float>(0, 0)));
    std::vector<VecSymbolMod> matrix(n, VecSymbolMod(n, mod_symbol(0, 0)));

    // Заполняем матрицу Топлица
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i - j >= 0) {
                matrix[i][j] = h[i - j];
            } else {
                matrix[i][j] = h[n + (i - j)]; 
            }
        }
    }
    print_log(CONSOLE, "h_matrix:\n");
    for(int i = 0; i < n; ++i) {
        print_VecSymbolMod(matrix[i]);
    }
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);

}
std::vector<VecSymbolMod> toeplitz4(const VecSymbolMod& h, int n) {
    std::vector<VecSymbolMod> matrix;//(n, VecSymbolMod(n, mod_symbol(0, 0)));
    VecSymbolMod rt = {h[1], h[0], mod_symbol(0, 0)};
    VecSymbolMod rt1 = {h[2], h[1], h[0]};
    VecSymbolMod rt2 = {mod_symbol(0, 0), h[2], h[1]};
    
    matrix.push_back(rt);
    matrix.push_back(rt1);
    matrix.push_back(rt2);
    print_log(CONSOLE, "h_matrix:\n");
    for(int i = 0; i < n; ++i) {
        print_VecSymbolMod(matrix[i]);
    }
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);

    return matrix;
    
}

std::vector<VecSymbolMod> inverseMatrix(const std::vector<VecSymbolMod>& matrix) {
    const int N = 3;
    std::vector<VecSymbolMod> augmented(N, VecSymbolMod(2 * N, {0.0f, 0.0f})); // Увеличенная матрица

    // Заполняем увеличенную матрицу
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            augmented[i][j] = matrix[i][j];
        }
        augmented[i][i + N] = {1.0f, 0.0f}; // Добавляем единичную матрицу
    }

    // Применяем метод Гаусса-Жордана для приведения к единичной матрице
    for (int i = 0; i < N; ++i) {
        // Нормализуем строку
        std::complex<float> diag = augmented[i][i];
        if (diag == std::complex<float>(0.0f, 0.0f)) {
            throw std::runtime_error("Матрица вырождена и не имеет обратной.");
        }
        for (int j = 0; j < 2 * N; ++j) {
            augmented[i][j] /= diag;
        }

        // Обнуляем остальные строки
        for (int k = 0; k < N; ++k) {
            if (k != i) {
                std::complex<float> factor = augmented[k][i];
                for (int j = 0; j < 2 * N; ++j) {
                    augmented[k][j] -= factor * augmented[i][j];
                }
            }
        }
    }

    // Извлекаем обратную матрицу
    std::vector<VecSymbolMod> inverse(N, VecSymbolMod(N));
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            inverse[i][j] = augmented[i][j + N];
        }
    }

    return inverse;
}


// Функция для нахождения обратной матрицы 3x3
std::vector<VecSymbolMod> inverseMatrix2(const std::vector<VecSymbolMod>& matrix) {
    const int N = 3; // Размерность матрицы
    std::vector<VecSymbolMod> augmented(N, VecSymbolMod(2 * N, {0.0f, 0.0f})); // Увеличенная матрица

    // Заполняем увеличенную матрицу
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            augmented[i][j] = matrix[i][j];
        }
        augmented[i][i + N] = {1.0f, 0.0f}; // Добавляем единичную матрицу
    }

    // Применяем метод Гаусса-Жордана для приведения к единичной матрице
    for (int i = 0; i < N; ++i) {
        // Нормализуем строку
        std::complex<float> diag = augmented[i][i];
        if (diag == std::complex<float>(0.0f, 0.0f)) {
            throw std::runtime_error("Матрица вырождена и не имеет обратной.");
        }
        for (int j = 0; j < 2 * N; ++j) {
            augmented[i][j] /= diag;
        }

        // Обнуляем остальные строки
        for (int k = 0; k < N; ++k) {
            if (k != i) {
                std::complex<float> factor = augmented[k][i];
                for (int j = 0; j < 2 * N; ++j) {
                    augmented[k][j] -= factor * augmented[i][j];
                }
            }
        }
    }

    // Извлекаем обратную матрицу
    std::vector<VecSymbolMod> inverse(N, VecSymbolMod(N));
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            inverse[i][j] = augmented[i][j + N];
        }
    }

    return inverse;
}

VecSymbolMod matrixVectorMultiply(const std::vector<VecSymbolMod>& matrix, const VecSymbolMod& vector) {
    if (matrix.empty() || matrix[0].size() != vector.size()) {
        throw std::invalid_argument("Размеры матрицы и вектора не совместимы для умножения.");
    }

    VecSymbolMod result(matrix.size(), {0.0f, 0.0f}); // Инициализируем результат

    for (size_t i = 0; i < matrix.size(); ++i) {
        for (size_t j = 0; j < matrix[0].size(); ++j) {
            result[i] += matrix[i][j] * vector[j];
        }
    }

    return result;
}
/*  https://dsplog.com/2009/11/29/ber-bpsk-isi-channel-zero-forcing-equalization/   */
void multipath_channel() {
    // VecSymbolMod h = {(0.5, 0.5), (0.7, 0.7), (0.1, 0.1)};
    VecSymbolMod h = {(0.2, 0.2), (0.9, 0.9), (0.3, 0.3)};
    
    // VecSymbolMod h = {(0.5, 0.), (0.7, 0.), (0.1, 0.)};
    
    int N = 10000;
    int snr_start = 0;
    int snr_end = 25;
    int i;
    float h2;
    print_to_file(FILE_SER, "w", "");
    for(i = snr_start; i <= snr_end; ++i) {
        int count_err = 0;
        int count_err2 = 0;
        
        int SNR = i;
        // print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);
        VecSymbolMod samples = generate_random_QAM16(N);

        // VecSymbolMod samples2 = samples;
        float Ps = calc_Ps(N, samples);
        h2 = pow(10, SNR * 0.1);
        float Q2 = Ps / h2;
        // print_log(LOG_DATA, "samples:\n");
        // print_VecSymbolMod(samples);
        
        // std::vector<float> h = generate_channel_relay_channel(0, 0.5, N);
        // VecSymbolMod r = samples + n;
        VecSymbolMod zer = {mod_symbol(0, 0), mod_symbol(1, 0), mod_symbol(0, 0)};
        // VecSymbolMod r = convolve(samples, h);
        VecSymbolMod r = convolve(h, samples);
        
        // print_log(LOG_DATA, "samples convolve h:\n");
        // print_VecSymbolMod(r);
        VecSymbolMod n = generate_noise_by_SNR(samples.size(), Q2);
        VecSymbolMod n2 = generate_noise_by_SNR(r.size(), Q2);
        
        VecSymbolMod r2 = r + n2;
        VecSymbolMod samples_noise = samples + n;
        
        // VecSymbolMod r3 = r2 / h;
        auto matrix = toeplitz4(h, 3);
        auto matrix2 = inverseMatrix(matrix);
        // print_log(CONSOLE, "h_matrix:\n");
        // for(int i = 0; i < 3; ++i) {
        //     print_VecSymbolMod(matrix2[i]);
        // }
        // print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);
        VecSymbolMod ck = matrixVectorMultiply(matrix2, zer);
        // print_log(CONSOLE, "c:\n");
        // print_VecSymbolMod(ck);
        VecSymbolMod r3 = convolve(r2, ck);
        // print_log(CONSOLE, "r3:\n");
        // print_VecSymbolMod(r3);
        VecSymbolMod r4 = VecSymbolMod(r3.begin() + 2, r3.begin() + N + 2);
        // print_log(CONSOLE, "r4[%d]:\n", r4.size());
        // print_VecSymbolMod(r4);  
        
#if 1
        // print_log(CONSOLE, "%d\n", i);
        // print_log(CONSOLE, "Ps: %f, s: %f %f\n", Ps, samples[0].real(), samples[0].imag());
        // print_log(CONSOLE, "h2 = %f\n", h2);        
        count_err = calc_error(samples, samples_noise);
        float Pser = (float)count_err / (float)N;
        print_log(CONSOLE, "Pser = %f, count_err = %d\n", Pser, count_err);
        count_err2 = calc_error(samples, r4);
        float Pser2 = (float)count_err2 / (float)N;
        print_log(CONSOLE, "Pser2 = %f, count_err2 = %d\n", Pser2, count_err2);
        
        print_to_file(FILE_SER, "a", "%f %d %f\n", Pser, SNR, Pser2);
#endif
#if 1
        char filename[30];
        memset(filename, 0, sizeof(filename));
        sprintf(filename, "../data/samples/rx_%d", (int)SNR);
        write_file_bin_data(
                filename, &r4[0], 
                r4.size() * sizeof(VecSymbolMod::value_type) );
#endif     
        // samples.clear();

    }

}


void modelling_channel(VecSymbolMod &samples) {
    float SNR = 25;
    VecSymbolMod zeros(100);
    samples.insert(samples.begin(), zeros.begin(), zeros.end());
    samples.insert(samples.end(), zeros.begin(), zeros.end());
    
    write_file_bin_data("../data/orig_sample.bin",
     (void*)&samples[0], samples.size() * sizeof(mod_symbol));

    float Ps = calc_Ps(samples.size(), samples);
    float h2 = pow(10, SNR * 0.1);
    float Q2 = Ps / h2;

    VecSymbolMod n = generate_noise_by_SNR(samples.size(), Q2);
    // VecSymbolMod n = generate_noise_by_SNR(samples.size(), 30000);
    
    samples = samples + n;
    // operator_add(samples, n);
    // samples = samples / mod_symbol(100.f, 0.f);
    operator_div(samples, mod_symbol(100.f, 0.f));
    // write_file_bin_data("../data/model_sample.bin",
    //  (void*)&samples[0], samples.size() * sizeof(mod_symbol));
    write_file_bin_data("../data/noise.bin",
        (void*)&n[0], n.size() * sizeof(mod_symbol));
}

int modelling_signal(char target) {

    switch (target)
    {
    case '0':
        test_base();
        break;
    case '3':
        model_calc_P_SER();
        break;
    case '4':
        model_soft_solutions();
        break;
    case '5':
        model_relay_channel();
        break;
    case '6':
        multipath_channel();
        break;
    default:
        print_log(CONSOLE, "No target for program\n");
        break;
    }
    return 0;
}


