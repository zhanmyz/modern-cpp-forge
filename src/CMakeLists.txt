# ============================================================================
# Source Examples - Your practice code
# ============================================================================

# Collect all .cpp files in src subdirectories; each compiles to a standalone executable
file(GLOB_RECURSE SRC_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/concurrency/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/design_patterns/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/memory_management/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/modern_cpp/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metaprogramming/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/performance/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/networking/*.cpp
)

foreach(SOURCE_FILE ${SRC_SOURCES})
    get_filename_component(TARGET_NAME ${SOURCE_FILE} NAME_WE)
    file(RELATIVE_PATH REL_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${SOURCE_FILE})
    get_filename_component(REL_DIR ${REL_PATH} DIRECTORY)
    string(REPLACE "/" "_" TARGET_PREFIX ${REL_DIR})
    set(FULL_TARGET_NAME "src_${TARGET_PREFIX}_${TARGET_NAME}")

    add_executable(${FULL_TARGET_NAME} ${SOURCE_FILE})
    target_link_libraries(${FULL_TARGET_NAME} PRIVATE common Threads::Threads)

    # Output to project root bin/src/<category>/ for F5 debug compatibility
    set_target_properties(${FULL_TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/src/${REL_DIR}
        OUTPUT_NAME ${TARGET_NAME}
    )
endforeach()
