#include "ofdm_modulation.hpp"

#define VALUE_DEF_INTERVAL 0.f 
#define NO_DATA 0.f

#define FAILED exit(-1)

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

static void print_VecSymbolMod( VecSymbolMod &vec) {
    for(int i = 0; i < (int)vec.size(); ++i) {
        if(i != 0 && i % 8 == 0) {
            print_log(CONSOLE,"\n");
        }
        print_log(CONSOLE, "%f + %fi  ", vec[i].i, vec[i].q);
        
    }
    print_log(CONSOLE, "\n");
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
    int count_rs = count_sub_no_defi / (param.step_RS + 1);
    // int size_block_data = (count_sub_no_defi - count_rs) + 
    //     (count_sub_no_defi % param.step_RS);
    int size_block_data = count_sub_no_defi - count_rs;
    print_log(CONSOLE, "count_rs = %d, count_sub_no_defi = %d"
        ", size_block_data = %d\n", 
        count_rs, count_sub_no_defi, size_block_data);
    // return ofdms;
    bool fill_symbol_and_end = false;
    int step;
    int block;
    while(running) {
        print_log(LOG, "[%s:%d] con state = %d\n", __func__, __LINE__, con);
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
                con = TO_INT(OFDM_STATE::EXIT);
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
            con = TO_INT(OFDM_STATE::ADD_DEF_INTERVAL);
            break;
        case TO_INT(OFDM_STATE::ADD_DEF_INTERVAL):
            ofdm.insert(ofdm.end(), 
                def_interval.begin(), def_interval.end());
            switch (pos_def_interval)
            {
            case BEGIN:
                con = TO_INT(OFDM_STATE::FILL_SYMBOL);
                pos_def_interval = END;
                break;
            case END:
                con = TO_INT(OFDM_STATE::ADD_OFDM_SYMBOL_TO_LIST_AND_CLEAR);
                pos_def_interval = BEGIN;
                break;
            default:
                break;
            }
            break;
        case TO_INT(OFDM_STATE::FILL_SYMBOL):
            block = size_block_data;
            for(int i = 0; i < count_rs; ++i) {
                print_log(LOG, "\tblock = %d, step = %d, ofdm.size() = %d\n",
                block, step, ofdm.size());
                ofdm.push_back(param.pilot);
                if(block < param.step_RS) {
                    step = block;
                } else {
                    step = param.step_RS;
                }
                if(step > 0) {
                    ofdm.insert(ofdm.end(), 
                        samples.begin() + index_step_data, 
                        samples.begin() + index_step_data + step);
                }
                print_log(LOG, "\t[%d]step = %d, ofdm.size() = %d\n", i, step, ofdm.size());
                index_step_data += step;
                block -= (step );
            }
            print_log(LOG, "new size = %d\n", ofdm.size());
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
            print_log(CONSOLE, "=========ofdm.size() = %d\n", ofdm.size());
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

VecSymbolMod OFDM_demodulator(OFDM_symbol samples, OFDM_params &param) {
    // delete_cyclic_prefix(samples)
    int count_sub_no_defi = 
        param.count_subcarriers - param.def_interval * 2;
    int count_rs = count_sub_no_defi / (param.step_RS + 1);
    for(int i = 0; i < samples.size; ++i) {
        VecSymbolMod &ofdm = samples.symbol[i];
        delete_cyclic_prefix(ofdm, param.cyclic_prefix);
        print_log(CONSOLE, "ofdm.size() = %d\n", ofdm.size());
        ofdm.erase(ofdm.begin(), ofdm.begin() + param.def_interval);
        ofdm.erase(ofdm.end() - param.def_interval, ofdm.end());
        VecSymbolMod pilots;
        int step_pilot = 1 + param.step_RS;
        for(int j = 0; j < ofdm.size(); j += step_pilot) {
            pilots.push_back(ofdm[j]);
        }
        print_log(CONSOLE, "ofdm.size() = %d, count pilots = %d\n", ofdm.size(), pilots.size());
        print_VecSymbolMod(pilots);
        
        //Добавить коррекцию канала
        
    }
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);
    VecSymbolMod d;
    return d;


}











