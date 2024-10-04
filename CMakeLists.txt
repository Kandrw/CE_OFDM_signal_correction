cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 11)

project(CE_OFDM_signal_correction)



set(COMPILE_FLAGS 
"-Bstatic -static-libgcc -static-libstdc++ -lm -g -Wall -pthread"
)

add_definitions(${COMPILE_FLAGS})

set(SOURCE 
    src/main.cpp
    
    src/trx/device_api.cpp
    src/trx/ad9361/device_ad9361.cpp

    src/phy/phy.cpp
    src/phy/modulation.cpp
    src/phy/modulation.cpp
    src/phy/ofdm_modulation.cpp
    src/phy/types.cpp

    src/model/modelling.cpp

    src/loaders/load_data.cpp
)

include_directories(lib/include)


add_subdirectory(lib/src/output)


add_executable(program ${SOURCE})

target_link_libraries(program output iio)







