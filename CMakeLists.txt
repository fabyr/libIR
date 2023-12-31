cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(libIR
    VERSION 1.0
    DESCRIPTION "libIR"
    LANGUAGES C CXX)

find_package(PkgConfig REQUIRED)

if(DEFINED ENV{SINGLE_PRECISION})
    pkg_search_module(FFTW REQUIRED fftw3f IMPORTED_TARGET)
else()
    pkg_search_module(FFTW REQUIRED fftw3 IMPORTED_TARGET)
endif()

file(GLOB_RECURSE libIR_SRC
     "src/*.h"
     "src/*.c"
     "src/*.hpp"
     "src/*.cpp"
)

add_library(IR SHARED ${libIR_SRC})
target_link_libraries(IR
    -lm
    PkgConfig::FFTW
)

if(DEFINED ENV{SINGLE_PRECISION})
    target_compile_definitions(IR PRIVATE SINGLE_PRECISION)
endif()