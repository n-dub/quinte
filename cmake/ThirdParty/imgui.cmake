set(QUINTE_IMGUI_PATH "${QUINTE_PROJECT_ROOT}/ThirdParty/imgui")


add_library(imgui STATIC
    ${QUINTE_IMGUI_PATH}/imconfig.h

    ${QUINTE_IMGUI_PATH}/imgui.h
    ${QUINTE_IMGUI_PATH}/imgui_internal.h

    ${QUINTE_IMGUI_PATH}/imgui.cpp
    ${QUINTE_IMGUI_PATH}/imgui_draw.cpp
    ${QUINTE_IMGUI_PATH}/imgui_widgets.cpp
    ${QUINTE_IMGUI_PATH}/imgui_tables.cpp

    ${QUINTE_IMGUI_PATH}/imgui_impl_glfw.h
    ${QUINTE_IMGUI_PATH}/imgui_impl_glfw.cpp

    ${QUINTE_IMGUI_PATH}/imgui_impl_vulkan.h
    ${QUINTE_IMGUI_PATH}/imgui_impl_vulkan.cpp

    ${QUINTE_IMGUI_PATH}/imgui_demo.cpp
)


find_package(Vulkan QUIET)
if(VULKAN_HEADERS_INSTALL_DIR)
	set(VK_INCLUDES "${VULKAN_HEADERS_INSTALL_DIR}/include")
elseif(Vulkan_INCLUDE_DIRS)
	set(VK_INCLUDES "${Vulkan_INCLUDE_DIRS}")
elseif(DEFINED ENV{VULKAN_SDK})
	set(VK_INCLUDES "$ENV{VULKAN_SDK}/include")
endif()


target_include_directories(imgui PUBLIC ${QUINTE_IMGUI_PATH})
set_target_properties(imgui PROPERTIES FOLDER "ThirdParty")
target_link_libraries(imgui glfw Vulkan::Vulkan)
target_include_directories(imgui PUBLIC "${VK_INCLUDES}")
target_compile_definitions(imgui PUBLIC -DImTextureID=ImU64)
