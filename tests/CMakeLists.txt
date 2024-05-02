set(SRC
    Common.hpp
    main.cpp

    FixedString.cpp
    String.cpp
)

add_executable(quinte-tests ${SRC})
quinte_configure_target(quinte-tests)

set_target_properties(quinte-tests PROPERTIES FOLDER "Runtime")
target_link_libraries(quinte-tests gtest gmock quinte-lib)

get_property("TARGET_SOURCE_FILES" TARGET quinte-tests PROPERTY SOURCES)
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${TARGET_SOURCE_FILES})

include(GoogleTest)
gtest_discover_tests(quinte-tests)
