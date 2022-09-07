CMAKE_MINIMUM_REQUIRED(VERSION 3.13.4)
PROJECT(sattrack)

set(SRCS
    sattrack.cc)

find_package (Eigen3 3.3 REQUIRED NO_MODULE)


add_executable(sattrack
    ${SRCS}
    src/getFileData.cc
    src/rectifyAzimuth.cc
    src/satellite.cc
    src/mainFunction.cc)

target_include_directories(sattrack PUBLIC include)

target_link_libraries(sattrack
    sgp4
    Eigen3::Eigen)
