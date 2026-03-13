cmake_minimum_required(VERSION 3.12)
project(AlgorithmTrainer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Находим Python (предполагается, что виртуальное окружение .venv активировано)
find_package(Python COMPONENTS Interpreter Development REQUIRED)

# Получаем путь к pybind11 из самого Python
execute_process(
    COMMAND "${Python_EXECUTABLE}" -m pybind11 --cmakedir
    OUTPUT_VARIABLE PYBIND11_CMAKE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_VARIABLE PYBIND11_ERROR
    RESULT_VARIABLE PYBIND11_RESULT
)

if(NOT PYBIND11_RESULT EQUAL 0)
    message(FATAL_ERROR "pybind11 не найден. Убедитесь, что он установлен в текущем окружении: pip install pybind11")
endif()

message(STATUS "pybind11 cmake dir: ${PYBIND11_CMAKE_DIR}")

# Добавляем путь к pybind11 в список поиска пакетов
list(APPEND CMAKE_PREFIX_PATH "${PYBIND11_CMAKE_DIR}")

# Теперь ищем сам пакет pybind11
find_package(pybind11 REQUIRED)

# Подключаем nlohmann/json через FetchContent
include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Список исходных файлов ядра
set(CORE_SOURCES
    src/Task.cpp
    src/TaskDatabase.cpp
    src/UserProfile.cpp
    src/RatingCalculator.cpp
    src/TaskSelector.cpp
    src/CoreAPI.cpp
)

# Создаём модуль Python
pybind11_add_module(core bindings/pybind_wrapper.cpp ${CORE_SOURCES})

# Подключаем директории include и библиотеки
target_include_directories(core PRIVATE include)
target_link_libraries(core PRIVATE nlohmann_json::nlohmann_json)

# Устанавливаем модуль в корневую папку проекта (рядом с main.py)
install(TARGETS core DESTINATION ..)