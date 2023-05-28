cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(libIR
    VERSION 1.0
    DESCRIPTION "libIR"
    LANGUAGES C CXX)

if(DEFINED ENV{NN})
    find_package(Torch REQUIRED)
endif()

file(GLOB_RECURSE libIR_SRC
     "src/*.h"
     "src/*.c"
     "src/*.hpp"
     "src/*.cpp"
)

add_library(IR SHARED ${libIR_SRC})

if(DEFINED ENV{NN})
    target_compile_definitions(IR PRIVATE NN)
endif()

target_link_libraries(IR m stdc++)
target_link_libraries(IR "${TORCH_LIBRARIES}")
set_property(TARGET IR PROPERTY CXX_STANDARD 14)