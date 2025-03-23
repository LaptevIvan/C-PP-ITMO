cmake_minimum_required(VERSION 3.27)
project(lab2 C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_GENERATOR_PLATFORM x64)

set(FFMPEG_INCLUDE_DIR "ffmpeg-6.1/include")
set(FFMPEG_LIB_DIR "ffmpeg-6.1/lib")

set(FFTW3_INCLUDE_DIR "fftw/include")
set(FFTW3_LIB_DIR "fftw/lib")

include_directories(${FFMPEG_INCLUDE_DIR})
include_directories(${FFTW3_INCLUDE_DIR})
link_directories(${FFMPEG_LIB_DIR})
link_directories(${FFTW3_LIB_DIR})

add_executable(${PROJECT_NAME} main.c Decode.c CrossCorrelation.c BufferSamples.c)
target_link_libraries(lab2 "avcodec.lib")
target_link_libraries(lab2 "avformat.lib")
target_link_libraries(lab2 "avutil.lib")
target_link_libraries(lab2 "swresample.lib")
target_link_libraries(lab2 "libfftw3-3.lib")

add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-Wpedantic)
