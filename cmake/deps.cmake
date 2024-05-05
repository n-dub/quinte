include(ThirdParty/get_cpm)
include(ThirdParty/glfw)
include(ThirdParty/gcem)
include(ThirdParty/imgui)
include(ThirdParty/mimalloc)
include(ThirdParty/small_vector)


if (QUINTE_BUILD_TESTS)
    include(ThirdParty/gtest)
endif ()
