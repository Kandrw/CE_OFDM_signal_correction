cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 11)

project(CE_OFDM_signal_correction)



set(COMPILE_FLAGS 
"-Bstatic -static-libgcc -static-libstdc++ -lm -g -Wall -pthread"
)

add_definitions(${COMPILE_FLAGS})


set (CONDITION_SDR no_sdr) #sdr or no_sdr






if(CONDITION_SDR STREQUAL "sdr")

# set(M_FOUND_LIBIIO 1)

# configure_file(src/config.hpp.in
#                   src/config.hpp)
# include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(src)
add_definitions(-DCONDITION_TARGET_PROGRAM=2 -DM_FOUND_LIBIIO)

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
else()

# set (CONDITION_TARGET_PROGRAM 4)
# configure_file(src/config.hpp.in
#                   src/config.hpp)
# include_directories(src)
add_definitions(-DCONDITION_TARGET_PROGRAM=4)

set(SOURCE 
    src/main.cpp
    
    src/phy/phy.cpp
    src/phy/modulation.cpp
    src/phy/modulation.cpp
    src/phy/ofdm_modulation.cpp
    src/phy/types.cpp

    src/model/modelling.cpp

    src/loaders/load_data.cpp
)



endif()




include_directories(lib/include)


add_subdirectory(lib/src/output)


add_executable(program ${SOURCE})


if(CONDITION_SDR STREQUAL "sdr")
target_link_libraries(program iio)
endif()

target_link_libraries(program output)




