set(QUINTE_SRC
    Application/Application.hpp
    Application/Application.cpp
    Application/BaseApplication.hpp
    Application/BaseApplication.cpp
    Application/VulkanApplication.hpp
    Application/VulkanApplication.cpp

    Audio/Backend/BackendBase.hpp
    Audio/Backend/BackendBase.cpp
    Audio/Backend/RingBuffer.hpp
    Audio/Backend/WASAPI.hpp
    Audio/Backend/WASAPI.cpp
    Audio/Engine.hpp
    Audio/Engine.cpp

    Core/Base.hpp
    Core/Core.hpp
    Core/CoreMath.hpp
    Core/CoreTypes.hpp
    Core/FixedString.hpp
    Core/FixedVector.hpp
    Core/Memory.hpp
    Core/String.hpp
    Core/StringBase.hpp
    Core/StringSlice.hpp
    Core/TempAllocator.hpp
    Core/TempAllocator.cpp
    Core/Threading.hpp
    Core/Unicode.hpp
    Core/${QUINTE_PLATFORM_NAME}/Utils.hpp
    Core/${QUINTE_PLATFORM_NAME}/Threading.cpp
    Core/${QUINTE_PLATFORM_NAME}/Memory.cpp

    UI/${QUINTE_PLATFORM_NAME}/Alerts.cpp
    UI/Widgets/Track.hpp
    UI/Widgets/Track.cpp
    UI/Alerts.hpp
    UI/Colors.hpp
    UI/Icons.hpp
    UI/Utils.hpp
    UI/Utils.cpp

    main.cpp
)


add_executable(quinte ${QUINTE_SRC})
target_link_libraries(quinte imgui gcem mimalloc-static)
target_include_directories(quinte PRIVATE "${QUINTE_PROJECT_ROOT}/code")

if (QUINTE_COMPILER_MSVC)
    target_compile_options(quinte PRIVATE /utf-8 /W4 /WX /ZI /INCREMENTAL)
else ()
    target_compile_options(quinte PRIVATE -Wall -Wextra -pedantic -Werror)
endif ()


file(GLOB_RECURSE
    QUINTE_RESOURCE_FILES
    RELATIVE ${QUINTE_PROJECT_ROOT}/ThirdParty/fonts
    CONFIGURE_DEPENDS
    ${QUINTE_PROJECT_ROOT}/ThirdParty/fonts/**.ttf
)

foreach(RESOURCE_FILE ${QUINTE_RESOURCE_FILES})
    configure_file(
        "${QUINTE_PROJECT_ROOT}/ThirdParty/fonts/${RESOURCE_FILE}"
        "${QUINTE_RESOURCE_DIR}/fonts/${RESOURCE_FILE}"
        COPYONLY
    )
endforeach()
