cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(sjconv-cpp)

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(sjconv-cpp src/main.cpp)
target_include_directories(sjconv-cpp PRIVATE thirdparty)

add_library(FFTConvolver STATIC
    thirdparty/FFTConvolver/FFTConvolver.cpp
    thirdparty/FFTConvolver/AudioFFT.cpp
    thirdparty/FFTConvolver/TwoStageFFTConvolver.cpp
    thirdparty/FFTConvolver/Utilities.cpp
)
target_link_libraries(sjconv-cpp -lsndfile -ljack FFTConvolver)
