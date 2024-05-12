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
    Audio/Ports/Port.cpp
    Audio/Ports/PortManager.hpp
    Audio/Ports/PortManager.cpp
    Audio/Sources/AudioSource.hpp
    Audio/Sources/BufferAudioSource.hpp
    Audio/Sources/BufferAudioSource.cpp
    Audio/Sources/Source.hpp
    Audio/Tracks/AudioClip.hpp
    Audio/Tracks/Playlist.hpp
    Audio/Tracks/Playlist.cpp
    Audio/Tracks/Fader.hpp
    Audio/Tracks/Track.hpp
    Audio/Tracks/TrackList.hpp
    Audio/Base.hpp
    Audio/AudioEngineEvents.hpp
    Audio/Engine.hpp
    Audio/Engine.cpp
    Audio/Session.hpp
    Audio/Session.cpp
    Audio/Transport.hpp

    Core/Memory/LinearAllocator.hpp
    Core/Memory/LinearAllocator.cpp
    Core/Memory/Memory.hpp
    Core/Memory/Memory.cpp
    Core/Memory/MemoryPool.hpp
    Core/Memory/MemoryPool.cpp
    Core/Memory/RefCount.hpp
    Core/Memory/TempAllocator.hpp
    Core/Memory/TempAllocator.cpp
    Core/Platform/${QUINTE_PLATFORM_NAME}/Utils.hpp
    Core/Platform/${QUINTE_PLATFORM_NAME}/Threading.cpp
    Core/Platform/${QUINTE_PLATFORM_NAME}/Memory.cpp
    Core/Base.hpp
    Core/Core.hpp
    Core/CoreMath.hpp
    Core/CoreTypes.hpp
    Core/EventBus.hpp
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

    Graph/ExecutionGraph.hpp
    Graph/ExecutionGraph.cpp
    Graph/ExecutionGraphNode.hpp

    UI/Platform/${QUINTE_PLATFORM_NAME}/Alerts.cpp
    UI/Widgets/Tracks/TrackEditView.hpp
    UI/Widgets/Tracks/TrackEditView.cpp
    UI/Widgets/Tracks/TrackMixerView.hpp
    UI/Widgets/Tracks/TrackMixerView.cpp
    UI/Widgets/Tracks/TracksCommon.hpp
    UI/Widgets/Tracks/TracksCommon.cpp
    UI/Widgets/Common.hpp
    UI/Widgets/Common.cpp
    UI/Windows/BaseWindow.hpp
    UI/Windows/EditWindow.hpp
    UI/Windows/EditWindow.cpp
    UI/Windows/WorkArea.hpp
    UI/Windows/WorkArea.cpp
    UI/Alerts.hpp
    UI/Colors.hpp
    UI/Icons.hpp
    UI/Utils.hpp
    UI/Utils.cpp
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
