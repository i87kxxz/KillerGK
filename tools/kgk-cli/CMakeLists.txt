# =============================================================================
# KGK-CLI - KillerGK Command Line Interface Tool
# =============================================================================
cmake_minimum_required(VERSION 3.20)

# CLI executable
add_executable(kgk-cli main.cpp)

# Set C++ standard
target_compile_features(kgk-cli PRIVATE cxx_std_20)

# Set output name
set_target_properties(kgk-cli PROPERTIES
    OUTPUT_NAME "kgk-cli"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Compiler warnings
if(MSVC)
    target_compile_options(kgk-cli PRIVATE
        /W4
        /WX-
        /permissive-
        /Zc:__cplusplus
    )
else()
    target_compile_options(kgk-cli PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# Platform-specific libraries
if(WIN32)
    # Windows-specific libraries for process execution
    target_link_libraries(kgk-cli PRIVATE shell32)
endif()

# Installation
install(TARGETS kgk-cli
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT Tools
)
