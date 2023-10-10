cmake_minimum_required(VERSION 3.0)
project(SportVideoAnalysis)

find_package(OpenCV REQUIRED)

add_executable(SportVideoAnalysis main.cpp
                                    im1.cpp im1.h im2.cpp im2.h im3.cpp im3.h im4.cpp im4.h im5.cpp im5.h
                                    im6.cpp im6.h im7.cpp im7.h im8.cpp im8.h im9.cpp im9.h im10.cpp im10.h
                                    im11.cpp im11.h im12.cpp im12.h im13.cpp im13.h im14.cpp im14.h im15.cpp im15.h)

# add_executable(SportVideoAnalysis testhsv.cpp)

target_link_libraries(SportVideoAnalysis ${OpenCV_LIBS})

# Set the Boost include directory
include_directories(/opt/homebrew/Cellar/boost/1.81.0_1/include)

# Set the Boost library directory
link_directories(/opt/homebrew/Cellar/boost/1.81.0_1/lib)