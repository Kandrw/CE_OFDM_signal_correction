#include "modulation.hpp"

#include <cstring>
#include <map>
#include <iostream>
#include <output.hpp>
#include <ctime>
#include <cmath>

#include <output.hpp>
#include <signal_processing.hpp>

mod_symbol table_QPSK[] = {
        {0.707107, 0.707107}, {0.707107, -0.707107}, 
        {-0.707107, 0.707107}, {-0.707107, -0.707107}, 
};
#define QAM16_LEVEL_1 (1.0f)
#define QAM16_LEVEL_2 (3.0f)

#define BITS_TO_BYTE 8
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))


namespace DIGITAL_SIGNAL_PROCESSING {

mod_symbol table_QAM16[] = {
  // LTE-16QAM constellation:
  //                q
  //  1011  1001  |   0001  0011
  //  1010  1000  |   0000  0010
  //---------------------------------> i
  //  1110    1100  |  0100  0110
  //  1111    1101  |  0101  0111
#if 0
   {QAM16_LEVEL_1, QAM16_LEVEL_1 },
   { QAM16_LEVEL_1,QAM16_LEVEL_2 },
   { QAM16_LEVEL_2,   QAM16_LEVEL_1 },
   { QAM16_LEVEL_2,   QAM16_LEVEL_2 },
   { QAM16_LEVEL_1, - QAM16_LEVEL_1 },
   { QAM16_LEVEL_1, - QAM16_LEVEL_2 },
   { QAM16_LEVEL_2, - QAM16_LEVEL_1 },
   { QAM16_LEVEL_2, - QAM16_LEVEL_2 },
   { -QAM16_LEVEL_1,   QAM16_LEVEL_1 },
   { -QAM16_LEVEL_1,   QAM16_LEVEL_2 },
   { -QAM16_LEVEL_2,   QAM16_LEVEL_1 },
   { -QAM16_LEVEL_2,   QAM16_LEVEL_2 },
   { -QAM16_LEVEL_1, - QAM16_LEVEL_1 },
   { -QAM16_LEVEL_1, - QAM16_LEVEL_2 },
   { -QAM16_LEVEL_2, - QAM16_LEVEL_1 },
   { -QAM16_LEVEL_2, - QAM16_LEVEL_2 }
#endif
    { -3.000000, -3.000000 },
    { 3.000000, -3.000000 },
    { 1.000000, -1.000000 },
    { -1.000000, -1.000000 },
    { -3.000000, 3.000000 },
    { 3.000000, 3.000000 },
    { 1.000000, 1.000000 },
    { -1.000000, 1.000000 },
    { -3.000000, 1.000000 },
    { 3.000000, 1.000000 },
    { 1.000000, 3.000000 },
    { -1.000000, 3.000000 },
    { -3.000000, -1.000000 },
    { 3.000000, -1.000000 },
    { 1.000000, -3.000000 },
    { -1.000000, -3.000000 },
};

u_char code_tableQAM16[] = {
    0b0000,
    0b1100,
    0b0110,
    0b1010,
    0b0011,
    0b1111,
    0b0101,
    0b1001,
    0b0001,
    0b1101,
    0b0111,
    0b1011,
    0b0010,
    0b1110,
    0b0100,
    0b1000
};


mod_symbol table_QAM64[] = {
        {0.46291, 0.46291}, {0.46291, 0.154303}, {0.154303, 0.46291}, {0.154303, 0.154303}, 
        {0.46291, 0.771517}, {0.46291, 1.080123}, {0.154303, 0.771517}, {0.154303, 1.080123}, 
        {0.771517, 0.46291}, {0.771517, 0.154303}, {1.080123, 0.46291}, {1.080123, 0.154303}, 
        {0.771517, 0.771517}, {0.771517, 1.080123}, {1.080123, 0.771517}, {1.080123, 1.080123}, 
        {0.46291, -0.46291}, {0.46291, -0.154303}, {0.154303, -0.46291}, {0.154303, -0.154303}, 
        {0.46291, -0.771517}, {0.46291, -1.080123}, {0.154303, -0.771517}, {0.154303, -1.080123}, 
        {0.771517, -0.46291}, {0.771517, -0.154303}, {1.080123, -0.46291}, {1.080123, -0.154303}, 
        {0.771517, -0.771517}, {0.771517, -1.080123}, {1.080123, -0.771517}, {1.080123, -1.080123}, 
        {-0.46291, 0.46291}, {-0.46291, 0.154303}, {-0.154303, 0.46291}, {-0.154303, 0.154303}, 
        {-0.46291, 0.771517}, {-0.46291, 1.080123}, {-0.154303, 0.771517}, {-0.154303, 1.080123}, 
        {-0.771517, 0.46291}, {-0.771517, 0.154303}, {-1.080123, 0.46291}, {-1.080123, 0.154303}, 
        {-0.771517, 0.771517}, {-0.771517, 1.080123}, {-1.080123, 0.771517}, {-1.080123, 1.080123}, 
        {-0.46291, -0.46291}, {-0.46291, -0.154303}, {-0.154303, -0.46291}, {-0.154303, -0.154303}, 
        {-0.46291, -0.771517}, {-0.46291, -1.080123}, {-0.154303, -0.771517}, {-0.154303, -1.080123}, 
        {-0.771517, -0.46291}, {-0.771517, -0.154303}, {-1.080123, -0.46291}, {-1.080123, -0.154303}, 
        {-0.771517, -0.771517}, {-0.771517, -1.080123}, {-1.080123, -0.771517}, {-1.080123, -1.080123},
};

int get_bits_per_symbol(TypeModulation m){
    switch (m)
    {
    case TypeModulation::BPSK:
        return 1;
    case TypeModulation::QPSK:
        return 2;
    case TypeModulation::QAM16:
        return 4;        
    case TypeModulation::QAM64:
        return 6;  
    case TypeModulation::QAM256:
        return 8;
    }
    return -1;
}

VecSymbolMod modulation_QPSK(bit_sequence &bits){
    print_log(LOG, "[%s:%d] start\n", __func__, __LINE__);
    time_counting_start();
    // int bits_symbol = 4;
    VecSymbolMod samples;
    int i, j;
    u_char *step = bits.buffer;
    // u_char con = 0;
    u_char val;
    for(i = 0; i < bits.size; ++i){
        u_char mask = 0b11000000;
        u_char shift = 6;
        for(j = 0; j < 4; ++j){
            val = (*step & mask) >> shift;
            print_log(LOG_DATA, "[%d][%d]val = %d, shit = %d, mask = %d, size = %d\n", 
                i, j, val, shift, mask, samples.size());
            if(val < 0 || val > 3){
                print_log(ERROR_OUT, "Error logic\n");
                exit(-1);
            }
            mod_symbol t1 = table_QPSK[val];
            // t1.real() += 4000.f;
            // t1.imag() += 4000.f;
            samples.push_back(t1);
            mask >>=2;
            shift -= 2;
        }
        ++step;
    }
    time_counting_end(CONSOLE, __func__);
    for(i = 0; i < (int)samples.size(); ++i){
        print_log(LOG_DATA, "%f + %fi\t", samples[i].real(), samples[i].imag());
    }
    print_log(LOG, "\n");
    print_log(LOG, "vector size = %d\n", samples.size());
    return samples;

}


/*grey*/
VecSymbolMod modulation_QAM16(bit_sequence &bits){
    // print_log(LOG, "[%s:%d] start\n", __func__, __LINE__);
    // print_to_file(
    //         FILE_NAME_DEBUG_DATA_ANNOTATE_MODULATION, "w", NULL);
    // time_counting_start();
   
    u_char *step = bits.buffer;
    u_char con = 0;
    VecSymbolMod samples;
    int i;
    u_char g1, g2, g3, g4;
    u_char val;
    float Y, q;
    std::vector<u_char> code_grey; 
    u_char tmp_s = 0;
    for(i = 0; i < bits.size;){
        switch (con)
        {
        case 0:
            con = 1;
            val = *step & 0b11110000;
            val >>= 4;
            break;
        default:
            con = 0;
            val = *step & 0b00001111;
            step++;
            ++i;
            break;
        }
        g1 = (val & 0b1); 
        g2 = (g1 ^ ((val & 0b10) >> 1));
        g3 = ((val & 0b10) >> 1) ^ ((val & 0b100) >> 2);
        g4 = ((val & 0b100) >> 2) ^ ((val & 0b1000) >> 3);
        Y = (g1) | (g2<<1);
        q = (g3) | (g4<<1);
        switch (con)
        {
        case 1:
            tmp_s = (g1<<3) | (g2<<2) | (g3<<1) | g4;
            tmp_s <<= 4;
            break;
        default:
            tmp_s |= (g1<<3) | (g2<<2) | (g3<<1) | g4;
            code_grey.push_back(tmp_s);
            tmp_s = 0;
            break;
        }
        // print_to_file(
        //     FILE_NAME_DEBUG_DATA_ANNOTATE_MODULATION,
        //     "a", "%d%d%d%d ", g1, g2, g3, g4);
        
        mod_symbol t1 = {Y * 2.f - 3.f, q * 2.f - 3.f};

        // print_log(LOG_DATA, "\t%f %f - %d%d%d%d\n", t1.real(), t1.imag());

        // print_log( CONSOLE, "0b%d%d%d%d,\n", g1, g2, g3, g4);
        // if(i %  == 0){
        //     print_log(CONSOLE, "\n");
        // }
        // print_log( CONSOLE, "%d%d%d%d - %f + %f\n", 
        //     g1, g2, g3, g4, t1.real(), t1.imag());
        // print_log( CONSOLE, "{ %f, %f },\n", 
        //     g1, g2, g3, g4, t1.real(), t1.imag());

        samples.push_back(t1);
    }
    // print_log(LOG, "\n");
    // print_log(LOG, "vector size = %d\n", samples.size());
    // write_file_bin_data(
    //             CODE_GREY, &code_grey[0], 
    //             code_grey.size());
    return samples;
}

VecSymbolMod modulation_QAM16_1(bit_sequence &bits){
    u_char *step = bits.buffer;
    u_char con = 0;
    VecSymbolMod samples;
    int i;
    u_char g1, g2, g3, g4;
    u_char val;
    float Y, q;
    std::vector<u_char> code_grey; 
    u_char tmp_s = 0;
    for(i = 0; i < bits.size;){
        switch (con)
        {
        case 0:
            con = 1;
            val = *step & 0b11110000;
            val >>= 4;
            break;
        default:
            con = 0;
            val = *step & 0b00001111;
            step++;
            ++i;
            break;
        }

        mod_symbol t1 = table_QAM16[val];
        print_log(LOG_DATA, "%d -> %f %f\n", val, t1.real(), t1.imag());
        samples.push_back(t1);
    }
    return samples;
}

VecSymbolMod modulation_QAM64(bit_sequence &bits){
    print_log(LOG, "[%s:%d] start\n", __func__, __LINE__);

    time_counting_start();
    // int bits_symbol = 6;
    u_char *buffer_bits = bits.buffer;

    VecSymbolMod samples;
    int i;
    short val, val2;
    u_char *step = buffer_bits;
    u_char con = 0;

    for(i = 0; i < bits.size;){
        switch(con)
        {
        case 0:
            val = *step & 0b11111100;
            val >>=2;
            print_log(LOG_DATA, "con0: val: %d, val>>2: %d\n", val<<2, val);
            con = 1;
            break;
        case 1:
            i++;
            val = (*step & 0b00000011) << 4;
            if(i == bits.size){
                break;
            }
            ++step;
            val2 = (*step & 0b11110000) >> 4;
            val = val | val2;
            print_log(LOG_DATA, "con1: val: %d\n", val);
            con = 2;
            break;
        case 2:
            i++;
            val = (*step & 0b00001111) << 2;
            if(i == bits.size){
                break;
            }
            ++step;
            val2 = (*step & 0b11000000) >> 6;
            val = val | val2;
            print_log(LOG_DATA, "con2: val: %d\n", val);
            con = 3;
            break;
        case 3:
            i++;
            val = (*step & 0b00111111);
            ++step;
            print_log(LOG_DATA, "con3: val: %d\n", val);
            con = 0;
            break;
        }
        if(val > 63 || val < 0){
            print_log(ERROR_OUT, "Error logic\n");
            exit(-1);
        }
        samples.push_back(table_QAM64[val]);
    }

    
    time_counting_end(CONSOLE, __func__);
    for(i = 0; i < (int)samples.size(); ++i){
        print_log(LOG_DATA, "%f + %fi\t", samples[i].real(), samples[i].imag());
    }
    print_log(LOG, "\n");
    print_log(LOG, "vector size = %d\n", samples.size());
    return samples;
}

bool symbol_cmp(float a, float b, float error){
    if(a < 0 && b < 0){
        a *= -1;
        b *= -1;
        return (a < (b + error)) && (a > (b - error));
    } 
    if(a > 0 && b > 0){
        return (a < (b + error)) && (a > (b - error));
    }
    return false;
}

/*comparison taking into account the error*/
bool mod_symbol_cmp(mod_symbol &A, mod_symbol &B, float error){
    // print_log(LOG, "[%s:%d] start\n", __func__, __LINE__);
    return symbol_cmp(A.real(), B.real(), error) && symbol_cmp(A.imag(), B.imag(), error);
}

float calc_dist(mod_symbol &A, mod_symbol &B) {
    return sqrt(powf(A.real() - B.real(), 2) + powf(A.imag() - B.imag(), 2));
}

bit_sequence *demodulation_QAM16(VecSymbolMod &samples){
    bit_sequence *data = new bit_sequence;
    data->size = samples.size() / 2;
    data->buffer = new u_char[data->size];
    float err_interval = 2.f;
    u_char *step = data->buffer;
    int con_state = 0;
    int conD = 0;
    
    for( int i = 0; i < (int)samples.size(); ++i) {
        float min_dist = 999.f;
        int pos = 0;
        for(int i2 = 0; i2 < (int)ARRAY_SIZE(table_QAM16); ++i2) {
            
            float dist = calc_dist(table_QAM16[i2], samples[i]);
            if(dist <= min_dist) {
                conD = 1;
                min_dist = dist;
                pos = i2;
            }
        }
        print_log(LOG_DATA, "min_dist: %f, pos - %d, %f %f - %f - %f -> %d\n", min_dist, pos,
                samples[i].real(), samples[i].imag(),
                table_QAM16[pos].real(), table_QAM16[pos].imag(),
                code_tableQAM16[pos]);
        // if(min_dist <= err_interval) {
            conD = 1;
#if 0
            switch(con_state) {
            case 0:
                *step = code_tableQAM16[pos] << 4;
                con_state = 1;
                break;
            default:    
                *step |= code_tableQAM16[pos];
                ++step;
                con_state = 0;
                break;
            }
#else
            switch(con_state) {
            case 0:
                *step = pos << 4;
                con_state = 1;
                break;
            default:    
                *step |= pos;
                ++step;
                con_state = 0;
                break;
            }

#endif
            
        // }
        if(!conD) {
            // print_log(CONSOLE, "Error decode\n");
        }
    }
    return data;
}


bit_sequence *demodulation_QAM64(VecSymbolMod &samples){

    // {
    //     float a = 0.462910;
    //     float b = -0.462910;
    //     bool res = symbol_cmp(a, b, 0.004f);
    //     print_log(CONSOLE, "\t\tTEST: res = %d\n", res);
    // }
    // {
    //     float a = 0.462910;
    //     float b = 0.462910;
    //     bool res = symbol_cmp(a, b, 0.004f);
    //     print_log(CONSOLE, "\t\tTEST: res = %d\n", res);
    // }
    // {
    //     float a =  -1.080123;
    //     float b =  -1.080123;
    //     bool res = symbol_cmp(a, b, 0.004f);
    //     print_log(CONSOLE, "\t\tTEST: res = %d\n", res);
    // }


    print_log(LOG, "[%s:%d] start\n", __func__, __LINE__);
    time_counting_start();
    int i, j;
    float error_cmp = 0.004;
    int bits_symbol = 6;
    struct bit_sequence *bits = new bit_sequence;
    bits->size = samples.size() * bits_symbol / BITS_TO_BYTE;

    if( (samples.size() * bits_symbol) % BITS_TO_BYTE != 0){
        bits->size++;
    }
    print_log(DEBUG, "[%s:%d] size vector = %d, ARRAY_SIZE(table_QAM64) = %d\n",
         __func__, __LINE__, samples.size(), ARRAY_SIZE(table_QAM64));
    // int size_mod = samples.size() / bits_symbol;
    bits->buffer = new u_char[bits->size]{0};
    u_char con = 0;
    u_char *step = bits->buffer;
    u_char val;
    int if_decode_symbol;
    for(i = 0; i < (int)samples.size(); ++i){
        if_decode_symbol = 0;
        for(j = 0; j < (int)ARRAY_SIZE(table_QAM64); ++j){
#if 0
            нужно переделать сравнение
            if(mod_symbol_cmp(samples[i], table_QAM64[j], error_cmp)){
                u_char bits_data = j;
                if_decode_symbol = 1;
                // print_log(LOG_DATA, "[%d] bits_data: %d\n", i, bits_data);
                switch (con)
                {
                case 0:
                    con = 1;
                    *step = bits_data << 2;
                    print_log(LOG_DATA, "con0: step: %d\n", *step);
                    // print_log(LOG_DATA, "con0: val: %d, val>>2: %d\n", val<<2, val);
                    break;
                case 1:
                    con = 2;
                    val = bits_data & 0b00110000;
                    *step = *step | (val >> 4);
                    ++step;
                    val = bits_data & 0b00001111;
                    *step = *step | (val << 4);
                    print_log(LOG_DATA, "con1: val: %d\n", val);
                    break;
                case 2:
                    con = 3;
                    val = bits_data & 0b00111100;
                    *step = *step | (val >> 2);
                    ++step;
                    val = bits_data & 0b00000011;
                    *step = *step | (val << 6);
                    print_log(LOG_DATA, "con2: val: %d\n", val);
                    break;
                case 3:
                    con = 0;
                    val = bits_data & 0b00111111;
                    *step = *step | (val);
                    ++step;
                    print_log(LOG_DATA, "con3: val: %d\n", val);
                    break;
                }
                break;
            }
#endif
        }
        if(!if_decode_symbol){
            print_log(CONSOLE, "[%s:%d] Error decode symbol: %f + %f\n", __func__, __LINE__, samples[i].real(), samples[i].imag());
        }
    }
    print_log(LOG, "[end %s:%d]\n", __func__, __LINE__);
    time_counting_end(CONSOLE, __func__);
    return bits;
}


bit_sequence *demodulation_QPSK(VecSymbolMod &samples){
    print_log(LOG, "[%s:%d] start\n", __func__, __LINE__);
    time_counting_start();
    int i, j, k = 0;
    float error_cmp = 0.004;
    int bits_symbol = 2;
    bit_sequence *bits = new bit_sequence;
    bits->size = samples.size() * bits_symbol / BITS_TO_BYTE;
    if( (samples.size() * bits_symbol) %  BITS_TO_BYTE != 0){
        print_log(LOG_DATA, "[%s:%d] Error: %d % 8 != 0\n", __func__, __LINE__, samples.size() * bits_symbol);
    }
    bits->buffer = new u_char[bits->size]{0};
    u_char *step = bits->buffer;
    u_char mask = 0b11000000;
    u_char shift = 6;
    int if_decode_symbol;
    for(i = 0, k = 0; i < (int)samples.size(); ++i){
        if_decode_symbol = 0;
        float min_dist = 999.f;
        int pos = 0;
        for(j = 0; j < (int)ARRAY_SIZE(table_QPSK); ++j){

            
            float dist = calc_dist(table_QPSK[j], samples[i]);
            if(dist <= min_dist) {
                // conD = 1;
                min_dist = dist;
                pos = j;
            }
            // if(mod_symbol_cmp(samples[i], table_QPSK[j], error_cmp)){
            //     if_decode_symbol = 1;
            //     u_char bits_data = j;
            //     print_log(LOG_DATA, "[%s:%d] i = %d, j = %d, shit = %d, mask = %d\n", __func__, __LINE__, i, j, shift, mask);
            //     *step = (*step | (bits_data << shift));
            //     if(k == 3){
            //         k = 0;
            //         ++step;
            //         mask = 0b11000000;
            //         shift = 6;
            //     }
            //     else{
            //         k++;
            //         mask >>= 2;
            //         shift -= 2;
            //     }
            //     break;
            // }
        }
        u_char bits_data = pos;
        print_log(LOG_DATA, "[%s:%d] i = %d, j = %d, shit = %d, mask = %d\n", __func__, __LINE__, i, j, shift, mask);
        *step = (*step | (bits_data << shift));
        if(k == 3){
            k = 0;
            ++step;
            mask = 0b11000000;
            shift = 6;
        }
        else{
            k++;
            mask >>= 2;
            shift -= 2;
        }
        
    



        if(!if_decode_symbol){
            print_log(LOG_DATA, "[%s:%d] Error decode symbol: %f + %f\n", __func__, __LINE__, samples[i].real(), samples[i].imag());
        }

    }
    
    
    print_log(LOG, "[end %s:%d]\n", __func__, __LINE__);
    time_counting_end(CONSOLE, __func__);
    return bits;
}

/*

6   2,4 4,2 6   - 64
4   4   4   4   - 16
8   8   8   8   - 256
8,2 6,4 4,6 2,8 - 1024


6,0   2,4 4,2 0,6 

*/


VecSymbolMod modulation_mapper(bit_sequence bits, TypeModulation m){
    // print_log(LOG, "[%s:%d] start\n", __func__, __LINE__);
    VecSymbolMod samples_mod;
    switch(m){
    case TypeModulation::QAM16:
        samples_mod = modulation_QAM16(bits);
        break;
    case TypeModulation::QAM64:
        samples_mod = modulation_QAM64(bits);
        break;
    case TypeModulation::QPSK:
        samples_mod = modulation_QPSK(bits);
        break;
    default:
        print_log(CONSOLE, "[%s:%d] Error: not found modulation\n");
        break;
    }
    // print_log(LOG, "[%s:%d] TEST samples_mod.size = %d\n", __func__, __LINE__, samples_mod.size());
    // samples_mod = modulation_QAM64(bits);
    // write_file_bin_data(
    //             FILE_NAME_SAVE_MODULATION, &samples_mod[0], 
    //             samples_mod.size() * sizeof(VecSymbolMod::value_type) );
    // print_log(LOG, "[%s:%d] end \n", __func__, __LINE__);
    return samples_mod;
}

bit_sequence *demodulation_mapper(VecSymbolMod &samples, TypeModulation m){
    // print_log(LOG, "[%s:%d] start\n", __func__, __LINE__);

    bit_sequence *bits = NULL;
    switch(m){
    case TypeModulation::QAM16:
        bits = demodulation_QAM16(samples);
        break;
    case TypeModulation::QAM64:
        bits = demodulation_QAM64(samples);
        break;
    case TypeModulation::QPSK:
        bits = demodulation_QPSK(samples);
        break;
    default:
        print_log(CONSOLE, "[%s:%d] Error: not found modulation\n");
        break;
    }
    // write_file_bin_data(
    //             FILE_NAME_SAVE_MODULATION, &samples_mod[0], 
    //             samples_mod.size() * sizeof(VecSymbolMod::value_type) );
    // print_log(LOG, "[%s:%d] end \n", __func__, __LINE__);
    return bits;
}

};


