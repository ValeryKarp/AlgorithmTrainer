cmake_minimum_required(VERSION 3.12)
project(AlgorithmTrainer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Находим Python из виртуального окружения
if(DEFINED ENV{VIRTUAL_ENV})
    set(Python_EXECUTABLE "$ENV{VIRTUAL_ENV}/Scripts/python.exe" CACHE FILEPATH "Python from virtualenv")
endif()
find_package(Python COMPONENTS Interpreter Development REQUIRED)

# Получаем путь к pybind11 из Python
execute_process(
    COMMAND "${Python_EXECUTABLE}" -m pybind11 --cmakedir
    OUTPUT_VARIABLE PYBIND11_CMAKE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_VARIABLE PYBIND11_ERROR
    RESULT_VARIABLE PYBIND11_RESULT
)

if(NOT PYBIND11_RESULT EQUAL 0)
    message(FATAL_ERROR "pybind11 не найден. Установите: pip install pybind11")
endif()

list(APPEND CMAKE_PREFIX_PATH "${PYBIND11_CMAKE_DIR}")
find_package(pybind11 REQUIRED)

# Подключаем nlohmann/json
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