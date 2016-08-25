cmake_minimum_required(VERSION 3.0)
project(object-detection)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(OpenCV 3 REQUIRED)
find_package(Boost REQUIRED system filesystem)

include_directories(common/include)
aux_source_directory(common COMMON_SOURCE_FILES)
add_library(common STATIC ${COMMON_SOURCE_FILES})
target_link_libraries(common
        ${OpenCV_LIBS}
        ${Boost_LIBRARIES})

aux_source_directory(objectdetect objectdetect_SOURCE_FILES)
add_executable(objectdetect ${objectdetect_SOURCE_FILES})
target_link_libraries(objectdetect common pthread)
