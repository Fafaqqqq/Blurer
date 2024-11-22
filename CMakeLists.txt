cmake_minimum_required(VERSION 3.21)

set(target opencv_test)

project(${target})

add_subdirectory(submodules/crow)


file(GLOB_RECURSE sources 
    *.cpp
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(${target} main.cpp)

target_link_libraries(${target} 
    ${OpenCV_LIBS}
    Crow::Crow
)

set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)



