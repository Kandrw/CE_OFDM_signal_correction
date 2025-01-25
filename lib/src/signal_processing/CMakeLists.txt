


set(SOURCES 
    modulation.cpp
    cfo_correct.cpp
    ofdm_modulation.cpp
    
)

add_library(signal_processing SHARED ${SOURCES})




