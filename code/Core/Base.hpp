#pragma once
#include <cstdint>

#if defined _WIN32 || defined _WIN64 || defined _WINDOWS
#    define QUINTE_WINDOWS 1

#    define QUINTE_EXPORT __declspec(dllexport)
#    define QUINTE_IMPORT __declspec(dllimport)

#elif defined __linux__
#    define QUINTE_LINUX 1

#    define QUINTE_EXPORT __attribute__((visibility("default")))
#    define QUINTE_IMPORT __attribute__((visibility("default")))

#else
#    error Unsupported platform
#endif


#define QUINTE_Stringify(txt) #txt


#if defined __clang__
#    define QUINTE_COMPILER_CLANG 1

#    define QUINTE_NO_BUILTIN(value) __attribute__((no_builtin(#value)))

#    if defined _MSC_VER
#        define QUINTE_COMPILER_MS_CLANG 1
#    endif

#    define QUINTE_PUSH_MSVC_WARNING(...)
#    define QUINTE_POP_MSVC_WARNING

#    define QUINTE_PUSH_CLANG_WARNING(warn)                                                                                      \
        _Pragma("clang diagnostic push") _Pragma(QUINTE_Stringify(clang diagnostic ignored warn))
#    define QUINTE_POP_CLANG_WARNING _Pragma("clang diagnostic pop")

#    define QUINTE_PRETTY_FUNCTION __PRETTY_FUNCTION__

#    define QUINTE_BUILTIN_ASSUME(expr) __builtin_assume(expr)

#    ifndef QUINTE_FORCE_INLINE
#        define QUINTE_FORCE_INLINE inline
#    endif
#elif defined _MSC_VER
#    define QUINTE_COMPILER_MSVC 1

#    define QUINTE_NO_BUILTIN(value)

#    define QUINTE_PUSH_MSVC_WARNING(warn) __pragma(warning(push)) __pragma(warning(disable : warn))
#    define QUINTE_POP_MSVC_WARNING __pragma(warning(pop))

#    define QUINTE_PUSH_CLANG_WARNING(...)
#    define QUINTE_POP_CLANG_WARNING

#    define QUINTE_PRETTY_FUNCTION __FUNCSIG__

#    define QUINTE_BUILTIN_ASSUME(expr) __assume(expr)

#    ifndef QUINTE_FORCE_INLINE
#        define QUINTE_FORCE_INLINE __forceinline
#    endif
#endif
