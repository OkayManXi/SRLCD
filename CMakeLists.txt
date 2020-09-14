cmake_minimum_required(VERSION 2.8.3)
project(SRLCD)

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS "-std=c++11")
#-DEIGEN_USE_MKL_ALL")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -g")

find_package(Eigen3)
find_package(OpenCV 3 REQUIRED)



include_directories(
${EIGEN3_INCLUDE_DIR}
  ${OPENCV3_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/thirdpart/DBow3/src
  src/
  ${PROJECT_SOURCE_DIR}/thirdpart/fftw/include
)


add_executable(SRLCD
    src/Util.cpp
    src/kcc.cpp
    src/LoopClass.cpp
    src/main.cpp
    src/saliency_detection.cpp
    src/saliency_region.cpp
    )


target_link_libraries(SRLCD ${catkin_LIBRARIES} ${OpenCV_LIBS} ${PROJECT_SOURCE_DIR}/thirdpart/DBow3/build/src/libDBoW3.a ${PROJECT_SOURCE_DIR}/thirdpart/fftw/lib/libfftw3f.a) 

