# ============================================================================
# Tests - Unit Tests
# ============================================================================

# Uncomment the following if Google Test is available
# find_package(GTest QUIET)
# if(GTest_FOUND)
#     enable_testing()
#     # Add test files...
# endif()

# Simple test framework (no external dependencies)
file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

foreach(SOURCE_FILE ${TEST_SOURCES})
    get_filename_component(TARGET_NAME ${SOURCE_FILE} NAME_WE)
    add_executable(${TARGET_NAME} ${SOURCE_FILE})
    target_link_libraries(${TARGET_NAME} PRIVATE common Threads::Threads)
    set_target_properties(${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/tests
    )
endforeach()
