cmake_minimum_required(VERSION 3.12)
project(AlgorithmTrainer)

list(APPEND CMAKE_PREFIX_PATH "C:/Users/valer/CLionProjects/TaskSelectionSystem/.venv/Lib/site-packages/pybind11/share/cmake/pybind11")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(pybind11 REQUIRED)

include(FetchContent)
FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

set(CORE_SOURCES
        src/Task.cpp
        src/TaskDatabase.cpp
        src/UserProfile.cpp
        src/RatingCalculator.cpp
        src/TaskSelector.cpp
        src/CoreAPI.cpp
)

pybind11_add_module(core bindings/pybind_wrapper.cpp ${CORE_SOURCES})

target_include_directories(core PRIVATE include)
target_link_libraries(core PRIVATE nlohmann_json::nlohmann_json)

install(TARGETS core DESTINATION ..)