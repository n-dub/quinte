if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(QUINTE_COMPILER_CLANG ON)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(QUINTE_COMPILER_GCC ON)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(QUINTE_COMPILER_MSVC ON)
endif ()


if (WIN32)
    set(QUINTE_WINDOWS ON)
elseif (UNIX)
    set(QUINE_UNIX ON)
    if (APPLE)
        set(QUINTE_APPLE ON)
    else ()
        set(QUINTE_LINUX ON)
    endif ()
endif ()


if (QUINTE_WINDOWS)
    set(QUINTE_PLATFORM_NAME "Windows")
elseif (QUINTE_LINUX)
    set(QUINTE_PLATFORM_NAME "Linux")
else ()
    message(FATAL_ERROR "platform not supported")
endif ()


set(CMAKE_DEBUG_POSTFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")


if (QUINTE_WINDOWS)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif ()


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(QUINTE_RESOURCE_DIR ${CMAKE_BINARY_DIR}/resources)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

function (quinte_configure_target TARGET)
    if (QUINTE_COMPILER_MSVC)
        target_compile_options(${TARGET} PUBLIC /utf-8 /W4 /WX /ZI /INCREMENTAL)
    else ()
        target_compile_options(${TARGET} PUBLIC -Wall -Wextra -pedantic -Werror)
    endif ()
endfunction ()
