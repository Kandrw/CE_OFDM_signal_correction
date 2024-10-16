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
        print_log(CONSOLE, "%f + %fi  ", vec[i].real(), vec[i].q);
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


OFDM_symbol OFDM_modulator(VecSymbolMod &samples, OFDM_params &param) {
    
    OFDM_symbol ofdms;
    VecSymbolMod def_interval((int)param.def_interval, {VALUE_DEF_INTERVAL, VALUE_DEF_INTERVAL});
    u_int16_t count_sub_no_defi = param.count_subcarriers - param.def_interval;
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
    bool fill_symbol_and_end = false;
    while(running) {
        switch (con)
        {
        case TO_INT(OFDM_STATE::EXIT):
            running = false;
            break;
        case TO_INT(OFDM_STATE::NEXT_OFDM_SYMBOL):
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
            if(ofdm.size() > count_sub_no_defi) {
                con = TO_INT(OFDM_STATE::ADD_DEF_INTERVAL);
            } else {
                con = TO_INT(OFDM_STATE::ADD_DATA);
            }
            break;
        case TO_INT(OFDM_STATE::ADD_DATA):
            if(ofdm.size() > count_sub_no_defi) {
                con = TO_INT(OFDM_STATE::ADD_DEF_INTERVAL);
            } else {
                int size_remained = count_sub_no_defi - ofdm.size();
                if(size_remained > param.step_RS) {
                    size_remained = param.step_RS;
                    con = TO_INT(OFDM_STATE::ADD_PILOT);
                } else {
                    con = TO_INT(OFDM_STATE::ADD_DEF_INTERVAL);
                }

                switch (fill_symbol_and_end)
                {
                case true:
                    ofdm.insert(ofdm.end(), 
                        interval_no_data.begin(), 
                        interval_no_data.begin() + size_remained);
                    break;
                case false:
                    int size_fill = 
    (size_remained <= samples.size() - index_step_data) ? 
    size_remained : (samples.size() - index_step_data);
                // bool 

                    int size_fill_no_data = 0;
                    if(size_fill < param.step_RS) {
                        fill_symbol_and_end = true;
                        size_fill_no_data = param.step_RS - size_fill;
                    }

                    break;
                default:
                    break;
                }



                
            }


            // if(ofdm.size() > count_sub_no_defi) {
            //     con = TO_INT(OFDM_STATE::ADD_DEF_INTERVAL);
            // } else if() {
                
            // }
            break;
        default:
            break;
        }
    }

    print_VecSymbolMod(ofdm);
    // for()
    return ofdms;
}






















