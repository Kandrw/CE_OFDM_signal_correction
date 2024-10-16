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
        print_log(CONSOLE, "%f + %fi  ", vec[i].i, vec[i].q);
    }
    print_log(CONSOLE, "\n");
}

#define TO_INT(x) static_cast<int>(x)

enum class OFDM_STATE{
    EXIT,
    ADD_PILOT,
    ADD_DATA,
    ADD_DEF_INTERVAL,
    ADD_OFDM_SYMBOL_TO_LIST_AND_CLEAR,
    NEXT_OFDM_SYMBOL,
};

#define BEGIN 0
#define END 1
#define PRINT_DEBUG_LINE(str) print_log(LOG, "%s", str)

OFDM_symbol OFDM_modulator(VecSymbolMod samples, OFDM_params &param) {
    
    OFDM_symbol ofdms;
    VecSymbolMod def_interval((int)param.def_interval, {VALUE_DEF_INTERVAL, VALUE_DEF_INTERVAL});
    int count_sub_no_defi = param.count_subcarriers - param.def_interval * 2;
    VecSymbolMod interval_no_data((int)param.step_RS, {NO_DATA, NO_DATA});
#if 0
    for(int i = 0; i < samples.size();) {
        VecSymbolMod::iterator it = samples.begin() + i;
        VecSymbolMod::iterator it_block = 
            samples.begin() + i + param.step_RS;
        ofdm.push_back(param.pilot);
        ofdm.insert(ofdm.end(), it, it_block);
        i += param.step_RS;
    }
#endif
#if 0
    
    u_int16_t iter_sub = 0;
    for(int i = 0; i < samples.size();) {
        VecSymbolMod::iterator it = samples.begin() + i;
        VecSymbolMod::iterator it_block = 
            samples.begin() + i + param.step_RS;
        ofdm.push_back(param.pilot);
        ofdm.insert(ofdm.end(), it, it_block);
        i += param.step_RS;
        iter_sub += 1 + param.step_RS;
        // if()
    }
#endif

    int con = TO_INT(OFDM_STATE::NEXT_OFDM_SYMBOL);
    bool running = true;
    VecSymbolMod ofdm;
    int pos_def_interval = BEGIN;
    int index_step_data = 0;
    int count_rs = count_sub_no_defi / param.step_RS;
    // int size_block_data = (count_sub_no_defi - count_rs) + 
    //     (count_sub_no_defi % param.step_RS);
    int size_block_data = count_sub_no_defi - count_rs;
    print_log(CONSOLE, "count_rs = %d, count_sub_no_defi = %d"
        ", size_block_data = %d\n", 
        count_rs, count_sub_no_defi, size_block_data);
    return ofdms;
    bool fill_symbol_and_end = false;
    while(running) {
        print_log(LOG, "[%s:%d] con state = %d\n", con);
        switch (con)
        {
        case TO_INT(OFDM_STATE::EXIT):
            running = false;
            break;
        case TO_INT(OFDM_STATE::NEXT_OFDM_SYMBOL):
            if(fill_symbol_and_end) {
                con = TO_INT(OFDM_STATE::EXIT);
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
                ofdm.insert(ofdm.end(), 
                    interval_no_data.begin(), interval_no_data.end());
                PRINT_DEBUG_LINE("fill zero\n");
            }
            con = TO_INT(OFDM_STATE::ADD_DEF_INTERVAL);
            break;
        case TO_INT(OFDM_STATE::ADD_DEF_INTERVAL):
            if(ofdm.size() > count_sub_no_defi) {
                print_log(ERROR_OUT, "[%s:%d] Error logic\n",
                    __func__, __LINE__);
                FAILED;
            }
            ofdm.insert(ofdm.end(), 
                def_interval.begin(), def_interval.end());
            switch (pos_def_interval)
            {
            case BEGIN:
                con = TO_INT(OFDM_STATE::ADD_PILOT);
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
        case TO_INT(OFDM_STATE::ADD_PILOT):
            if(ofdm.size() >= count_sub_no_defi) {
                con = TO_INT(OFDM_STATE::ADD_DEF_INTERVAL);
            } else {
                con = TO_INT(OFDM_STATE::ADD_DATA);
            }
            break;
        case TO_INT(OFDM_STATE::ADD_DATA):
            if(ofdm.size() >= count_sub_no_defi) {
                con = TO_INT(OFDM_STATE::ADD_DEF_INTERVAL);
            } else {
                con = TO_INT(OFDM_STATE::ADD_PILOT);
            }

            break;
        default:
            break;
        }
    }

    print_VecSymbolMod(ofdm);
    // for()
    return ofdms;
}






















