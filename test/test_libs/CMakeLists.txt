

cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 11)

project(CE_OFDM_signal_correction)



set(SOURCE
    main.cpp
)

include_directories(lib/include)
add_subdirectory(lib/src/output)
add_subdirectory(lib/src/complex_container)
add_subdirectory(lib/src/signal_processing)
add_subdirectory(lib/src/configure)


add_executable(program_test_lib ${SOURCE})

target_link_libraries(program_test_lib
  output signal_processing complex_container configure
  fftw3f
)



