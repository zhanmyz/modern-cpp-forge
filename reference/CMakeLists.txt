# ============================================================================
# Reference Examples - Standard reference implementations
# ============================================================================

# Collect all .cpp files in reference subdirectories; each compiles to a standalone executable
file(GLOB_RECURSE REFERENCE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/concurrency/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/design_patterns/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/memory_management/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/modern_cpp/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metaprogramming/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/performance/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/networking/*.cpp
)

foreach(SOURCE_FILE ${REFERENCE_SOURCES})
    # Extract filename (without extension) as target name
    get_filename_component(TARGET_NAME ${SOURCE_FILE} NAME_WE)

    # Get path relative to reference/ directory for grouping
    file(RELATIVE_PATH REL_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${SOURCE_FILE})
    get_filename_component(REL_DIR ${REL_PATH} DIRECTORY)

    # Add prefix to avoid name collisions: ref_concurrency_thread_basics
    string(REPLACE "/" "_" TARGET_PREFIX ${REL_DIR})
    set(FULL_TARGET_NAME "ref_${TARGET_PREFIX}_${TARGET_NAME}")

    add_executable(${FULL_TARGET_NAME} ${SOURCE_FILE})
    target_link_libraries(${FULL_TARGET_NAME} PRIVATE common Threads::Threads)

    # Output to project root bin/reference/<category>/ for F5 debug compatibility
    set_target_properties(${FULL_TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/reference/${REL_DIR}
        OUTPUT_NAME ${TARGET_NAME}
    )
endforeach()
