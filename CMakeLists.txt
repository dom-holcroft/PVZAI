cmake_minimum_required(VERSION 3.5.0)
project(pvz VERSION 0.1.0 LANGUAGES CXX ASM)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

add_library(pvz 
            code/c++/pvz.cpp
            code/c++/pvzinterface.cpp
            code/c++/machinecode.cpp
            code/c++/memoryconfig.cpp
            code/c++/memorylocator.cpp
            code/c++/util.cpp
            )


find_package(Python3 COMPONENTS Interpreter Development NumPy REQUIRED)


target_include_directories(pvz PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/code/c++
    ${Python3_INCLUDE_DIRS}
    ${Python3_NumPy_INCLUDE_DIRS}
)


target_link_libraries(pvz PUBLIC_HEADER
    ${Python3_LIBRARIES}
    ${Python3_NumPy_LIBRARIES}
    tomlplusplus::tomlplusplus
)




