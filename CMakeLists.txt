cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 11)

project(CE_OFDM_signal_correction)




set(COMPILE_FLAGS 
"-Bstatic -static-libgcc -static-libstdc++ -lm -g -Wall -pthread"
)

add_definitions(${COMPILE_FLAGS})


set (CONDITION_SDR sdr) #sdr or no_sdr


if(CONDITION_SDR STREQUAL "sdr")

add_definitions(-DCONDITION_TARGET_PROGRAM=4 -DM_FOUND_LIBIIO)

set(SOURCE 
    src/main.cpp
    src/target.cpp
    src/trx/device_api.cpp
    src/trx/ad9361/device_ad9361.cpp
    src/trx/receiver_ofdm.cpp
    src/loaders/load_data.cpp
    src/ipc/managment_ipc.cpp
    src/model/modelling.cpp
    src/trx/debug_trx.cpp
    src/configure/config_parse.cpp
)
endif()




include_directories(lib/include)
add_subdirectory(lib/src/output)
add_subdirectory(lib/src/complex_container)
add_subdirectory(lib/src/signal_processing)

add_executable(program ${SOURCE})


if(CONDITION_SDR STREQUAL "sdr")

add_library(iio_26 SHARED IMPORTED) # or STATIC instead of SHARED
set_target_properties(iio_26 PROPERTIES
  IMPORTED_LOCATION "/home/andrey/Документы/libiio/build/libiio.so"
  INTERFACE_INCLUDE_DIRECTORIES "/home/andrey/Документы/libiio/include"
)
target_link_libraries(program iio_26)
endif()

target_link_libraries(program output complex_container signal_processing fftw3f)




