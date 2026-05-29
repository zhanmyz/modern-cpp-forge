# ============================================================================
# Common Library - Logger, Timer, and Utilities
# ============================================================================
add_library(common STATIC
    src/logger.cpp
    src/timer.cpp
)

target_include_directories(common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(common PUBLIC
    Threads::Threads
)
