cmake_minimum_required(VERSION 3.19)
project(quinte VERSION 0.1.0
    LANGUAGES CXX C
)


enable_testing()
set(QUINTE_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
list(PREPEND CMAKE_MODULE_PATH "${QUINTE_PROJECT_ROOT}/cmake")


include(Config)
include(deps)

add_subdirectory(code)
add_subdirectory(tests)
