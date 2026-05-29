# ============================================================================
# Benchmarks - Performance Benchmarks
# ============================================================================

file(GLOB BENCH_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

foreach(SOURCE_FILE ${BENCH_SOURCES})
    get_filename_component(TARGET_NAME ${SOURCE_FILE} NAME_WE)
    add_executable(${TARGET_NAME} ${SOURCE_FILE})
    target_link_libraries(${TARGET_NAME} PRIVATE common Threads::Threads)
    set_target_properties(${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/benchmarks
    )

    # Release optimization level
    target_compile_options(${TARGET_NAME} PRIVATE -O2)
endforeach()
