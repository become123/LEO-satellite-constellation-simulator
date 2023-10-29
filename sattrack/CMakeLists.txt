CMAKE_MINIMUM_REQUIRED(VERSION 3.13.4)
PROJECT(sattrack)

set(SRCS
    sattrack.cc)

find_package (Eigen3 3.3 REQUIRED NO_MODULE)
find_package (nlohmann_json 3.2.0 REQUIRED)

add_executable(sattrack
    ${SRCS}
    src/getFileData.cc
    src/rectifyAzimuth.cc
    src/satellite.cc
    src/mainFunction.cc
    src/groundStation.cc
    src/util.cc)

target_include_directories(sattrack PUBLIC include)

target_link_libraries(sattrack
    sgp4
    Eigen3::Eigen
    nlohmann_json::nlohmann_json)
