# Standalone CMakeLists.txt for CLI property tests
# This can be built independently without the full KillerGK library dependencies

cmake_minimum_required(VERSION 3.20)
project(KillerGK_CLI_Tests VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =============================================================================
# FetchContent for Test Dependencies
# =============================================================================
include(FetchContent)

# Google Test
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# RapidCheck
FetchContent_Declare(
    rapidcheck
    GIT_REPOSITORY https://github.com/emil-e/rapidcheck.git
    GIT_TAG master
)
set(RC_ENABLE_GTEST ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest rapidcheck)

# =============================================================================
# CLI Property Test Executable
# =============================================================================
add_executable(test_cli_properties test_cli_properties.cpp)

target_link_libraries(test_cli_properties PRIVATE
    GTest::gtest
    GTest::gtest_main
    rapidcheck
    rapidcheck_gtest
)

target_include_directories(test_cli_properties PRIVATE
    ${rapidcheck_SOURCE_DIR}/include
    ${rapidcheck_SOURCE_DIR}/extras/gtest/include
)

# Enable /bigobj for MSVC
if(MSVC)
    target_compile_options(test_cli_properties PRIVATE /bigobj)
endif()

# =============================================================================
# Testing
# =============================================================================
enable_testing()
add_test(NAME test_cli_properties COMMAND test_cli_properties)

# Set minimum iterations for property tests
target_compile_definitions(test_cli_properties PRIVATE
    RC_PARAMS="max_success=100"
)
