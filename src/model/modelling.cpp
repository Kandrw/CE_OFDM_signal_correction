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
        mod_symbol t1 = {d(gen) + samples[i].I, 
                            d(gen) + samples[i].Q};
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

float calc_Ps(int N, VecSymbolMod &S) {

    float sum = 0.f;
    for(int i = 0; i < (int)S.size(); ++i){
        sum += (S[i].I * S[i].I + S[i].Q * S[i].Q);
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

int calc_error(VecSymbolMod &tx, VecSymbolMod &rx) {
    int ce = 0;
    float I, Q;
    float interval = 0.4;
    for(int i = 0; i < (int)tx.size(); ++i) {
        I = tx[i].I - rx[i].I;
        Q = tx[i].Q - rx[i].Q;
        // print_log(CONSOLE, "tx: %f %f, rx: %f %f\n", tx[i].I, tx[i].Q, rx[i].I, rx[i].Q);
        // print_log(CONSOLE, "I: %f, Q: %f\n", I, Q);
        if( !(I < interval && I > interval * -1.0f) ) {
            ce += 1;
        } else {
            if( (Q > interval || Q < interval * -1.0f) ) {
                ce += 1;
            }
        }
    }
    return ce;
}

void print_VecSymbolMod( VecSymbolMod &vec) {
    for(int i = 0; i < (int)vec.size(); ++i) {
        print_log(CONSOLE, "%f + %fi  ", vec[i].I, vec[i].Q);
    }
    print_log(CONSOLE, "\n");
}

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
        print_log(CONSOLE, "Ps: %f, s: %f %f\n", Ps, samples[0].I, samples[0].Q);
        
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

float calc_part_equation(float y0, float y, int b, float Q) {
#if 0
    print_log(CONSOLE, "\t\t1.f / (sqrt(2.f * (float)PI) * Q) = %f\n",
        1.f / (sqrt(2.f * (float)PI * Q)));
    print_log(CONSOLE, "\t\t(( -1.f * (y - y0) * (y - y0))"
    " / (2.f * powf(Q, 2))) = %f\n",
        (( -1.f * (y - y0) * (y - y0)) / (2.f * powf(Q, 2))));
    print_log(CONSOLE, "\t\t e ^ %f = %f\n", ( -1.f * (y - y0) * (y - y0)) / (2 * Q),

        powf(2.7182818, 
            (( -1.f * (y - y0) * (y - y0)) / (Q)))
        );
#endif
#if 0
    return 1.f / (sqrt(2.f * (float)PI) * Q) * \
        powf(2.7182818, 
            (( -1.f * (y - y0) * (y - y0)) / (2.f * powf(Q, 2))) );
#else
    return 1.f / (sqrt(2.f * (float)PI * Q)) * \
        powf(2.7182818f, 
            (( -1.f * (y - y0) * (y - y0)) / (2.f * Q)) );
#endif
}

float calc_P_y_b(float y01, float y02, float y, int b, float Q) {
    float P = calc_part_equation(y01, y, b, Q);
    P += calc_part_equation(y02, y, b, Q);
#if 0
    print_log(CONSOLE, "\t|| %f + %f\n", 
    calc_part_equation(y01, y, b, Q), calc_part_equation(y02, y, b, Q));
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
        float B0_r0 = calc_P_y_b(-3.f, 1.f, samples_rx[i].I, 0, Q2);
        float B0_r1 = calc_P_y_b(-1.f, 3.f, samples_rx[i].I, 1, Q2);
        float B1_r0 = calc_P_y_b(-3.f, -1.f, samples_rx[i].I, 0, Q2);
        float B1_r1 = calc_P_y_b(1.f, 3.f, samples_rx[i].I, 1, Q2);
        float B2_r0 = calc_P_y_b(1.f, -3.f, samples_rx[i].Q, 0, Q2);
        float B2_r1 = calc_P_y_b(3.f, -1.f, samples_rx[i].Q, 1, Q2);
        float B3_r0 = calc_P_y_b(-1.f, -3.f, samples_rx[i].Q, 0, Q2);
        float B3_r1 = calc_P_y_b(1.f, 3.f, samples_rx[i].Q, 1, Q2);

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
            print_log(CONSOLE, "[%d] r[i](I,Q) = (%f %f)\n", 
                i, samples_rx[i].I, samples_rx[i].Q);
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
        //     samples[0].I, samples[0].Q, r[0].I, r[0].Q);
#if 0
        write_file_bin_data(
                    "../data/practiec5/s.bin", &samples[0], 
                    samples.size() * sizeof(VecSymbolMod::value_type) );
        write_file_bin_data(
                    "../data/practice5/s_noise.bin", &r[0], 
                    r.size() * sizeof(VecSymbolMod::value_type) );

        print_log(CONSOLE, "\n[%s:%d] Q = %f\n", __func__, __LINE__, Q2);
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
