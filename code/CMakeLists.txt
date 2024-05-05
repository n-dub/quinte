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
    Audio/Buffers/AudioBuffer.hpp
    Audio/Buffers/AudioBuffer.cpp
    Audio/Buffers/AudioBufferCommon.hpp
    Audio/Buffers/AudioBufferView.hpp
    Audio/Buffers/AudioBufferView.cpp
    Audio/Buffers/Buffer.hpp
    Audio/Buffers/BufferView.hpp
    Audio/Ports/AudioPort.hpp
    Audio/Ports/Port.hpp
    Audio/Ports/PortManager.hpp
    Audio/Ports/PortManager.cpp
    Audio/Base.hpp
    Audio/Engine.hpp
    Audio/Engine.cpp
    Audio/Session.hpp
    Audio/Session.cpp

    Core/Memory/Memory.hpp
    Core/Memory/Memory.cpp
    Core/Memory/MemoryPool.hpp
    Core/Memory/MemoryPool.cpp
    Core/Memory/RefCount.hpp
    Core/Memory/TempAllocator.hpp
    Core/Memory/TempAllocator.cpp
    Core/Base.hpp
    Core/Core.hpp
    Core/CoreMath.hpp
    Core/CoreTypes.hpp
    Core/FixedString.hpp
    Core/FixedVector.hpp
    Core/Hash.hpp
    Core/Interface.hpp
    Core/Interface.cpp
    Core/LockFreeHashTable.hpp
    Core/String.hpp
    Core/StringBase.hpp
    Core/StringSlice.hpp
    Core/Threading.hpp
    Core/Unicode.hpp
    Core/${QUINTE_PLATFORM_NAME}/Utils.hpp
    Core/${QUINTE_PLATFORM_NAME}/Threading.cpp
    Core/${QUINTE_PLATFORM_NAME}/Memory.cpp

    UI/Widgets/Track.hpp
    UI/Widgets/Track.cpp
    UI/Alerts.hpp
    UI/Colors.hpp
    UI/Icons.hpp
    UI/Utils.hpp
    UI/Utils.cpp
    UI/${QUINTE_PLATFORM_NAME}/Alerts.cpp
)


add_library(quinte-lib STATIC ${QUINTE_SRC})
target_link_libraries(quinte-lib imgui gcem mimalloc-static gch::small_vector)
target_include_directories(quinte-lib PUBLIC "${QUINTE_PROJECT_ROOT}/code")
quinte_configure_target(quinte-lib)


add_executable(quinte main.cpp)
target_link_libraries(quinte quinte-lib)
quinte_configure_target(quinte)


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
