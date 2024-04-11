set(QUINTE_SRC
    Application/Application.hpp
    Application/Application.cpp
    Application/BaseApplication.hpp
    Application/BaseApplication.cpp
    Application/VulkanApplication.hpp
    Application/VulkanApplication.cpp

    Core/Base.hpp
    Core/Core.hpp
    Core/FixedString.hpp
    Core/Memory.hpp
    Core/String.hpp
    Core/StringBase.hpp
    Core/StringSlice.hpp
    Core/Unicode.hpp
	Core/${QUINTE_PLATFORM_NAME}/Utils.hpp

    UI/Alerts.hpp
    UI/${QUINTE_PLATFORM_NAME}/Alerts.cpp

    main.cpp
)


add_executable(quinte ${QUINTE_SRC})
target_link_libraries(quinte imgui)
target_include_directories(quinte PRIVATE "${QUINTE_PROJECT_ROOT}/code")