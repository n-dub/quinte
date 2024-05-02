#pragma once
#include <cstdint>

#if defined _WIN32 || defined _WIN64 || defined _WINDOWS
#    define QU_WINDOWS 1

#    define QU_EXPORT __declspec(dllexport)
#    define QU_IMPORT __declspec(dllimport)

#elif defined __linux__
#    define QU_LINUX 1

#    define QU_EXPORT __attribute__((visibility("default")))
#    define QU_IMPORT __attribute__((visibility("default")))

#else
#    error Unsupported platform
#endif

#if defined __clang__
#    define QU_COMPILER_CLANG 1

#    define QU_NO_BUILTIN(value) __attribute__((no_builtin(#value)))

#    if defined _MSC_VER
#        define QU_COMPILER_MS_CLANG 1
#    endif

#    define QU_PUSH_MSVC_WARNING(...)
#    define QU_POP_MSVC_WARNING

#    define QU_PUSH_CLANG_WARNING(warn) _Pragma("clang diagnostic push") _Pragma(QU_Stringify(clang diagnostic ignored warn))
#    define QU_POP_CLANG_WARNING _Pragma("clang diagnostic pop")

#    define QU_PRETTY_FUNCTION __PRETTY_FUNCTION__

#    define QU_BUILTIN_ASSUME(expr) __builtin_assume(expr)

#    ifndef QU_FORCE_INLINE
#        define QU_FORCE_INLINE inline
#    endif
#elif defined _MSC_VER
#    define QU_COMPILER_MSVC 1

#    define QU_NO_BUILTIN(value)

#    define QU_PUSH_MSVC_WARNING(warn) __pragma(warning(push)) __pragma(warning(disable : warn))
#    define QU_POP_MSVC_WARNING __pragma(warning(pop))

#    define QU_PUSH_CLANG_WARNING(...)
#    define QU_POP_CLANG_WARNING

#    define QU_PRETTY_FUNCTION __FUNCSIG__

#    define QU_BUILTIN_ASSUME(expr) __assume(expr)

#    ifndef QU_FORCE_INLINE
#        define QU_FORCE_INLINE __forceinline
#    endif
#endif

#if NDEBUG
#    define QU_RELEASE 1
#else
#    define QU_DEBUG 1
#endif


#define QU_Stringify(txt) #txt

#define QU_CONCAT(a, b) QU_CONCAT_INNER(a, b)
#define QU_CONCAT_INNER(a, b) a##b

#define QU_UNIQUE_NAME(base) QU_CONCAT(base, __COUNTER__)

#define QU_RESTRICT __restrict
