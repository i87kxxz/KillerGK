# Tests CMakeLists.txt
# Configures Google Test for unit testing and RapidCheck for property-based testing
# Requirements: Testing Strategy from design.md

include(FetchContent)

# =============================================================================
# Google Test Configuration
# =============================================================================
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# =============================================================================
# RapidCheck Configuration (Property-Based Testing)
# =============================================================================
FetchContent_Declare(
    rapidcheck
    GIT_REPOSITORY https://github.com/emil-e/rapidcheck.git
    GIT_TAG master
)

# Enable RapidCheck integration with Google Test BEFORE making available
set(RC_ENABLE_GTEST ON CACHE BOOL "" FORCE)

# Make dependencies available
FetchContent_MakeAvailable(googletest rapidcheck)

# =============================================================================
# RapidCheck GTest Integration Library
# =============================================================================
# Note: rapidcheck_gtest is automatically created by RapidCheck when RC_ENABLE_GTEST=ON
# We just need to ensure the include directories are available

# =============================================================================
# Property-Based Testing Configuration
# =============================================================================
# Minimum iterations for property tests (as per design document)
set(KGK_PBT_MIN_ITERATIONS 100 CACHE STRING "Minimum iterations for property-based tests")

# Configure RapidCheck default settings
add_compile_definitions(
    RC_PARAMS="max_success=${KGK_PBT_MIN_ITERATIONS}"
)

# =============================================================================
# Include Directories
# =============================================================================
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${rapidcheck_SOURCE_DIR}/include
    ${rapidcheck_SOURCE_DIR}/extras/gtest/include
)

# Test helper library
add_library(test_helpers INTERFACE)
target_include_directories(test_helpers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Macro to create test executables
macro(add_kgk_test TEST_NAME)
    add_executable(${TEST_NAME} ${ARGN})
    target_link_libraries(${TEST_NAME} PRIVATE
        KillerGK
        GTest::gtest
        GTest::gtest_main
        rapidcheck
        rapidcheck_gtest
        test_helpers
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    
    # Set test properties
    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 60
        ENVIRONMENT "GTEST_COLOR=1"
    )
    
    # Enable /bigobj for MSVC to handle large object files (many template instantiations)
    if(MSVC)
        target_compile_options(${TEST_NAME} PRIVATE /bigobj)
    endif()
endmacro()

# Core module tests
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/core/test_types.cpp")
    add_kgk_test(test_core_types core/test_types.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/core/test_error.cpp")
    add_kgk_test(test_core_error core/test_error.cpp)
endif()

# Widget tests
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/widgets/test_widget.cpp")
    add_kgk_test(test_widgets widgets/test_widget.cpp)
endif()

# Layout tests
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/layout/test_layout.cpp")
    add_kgk_test(test_layout layout/test_layout.cpp)
endif()

# Animation tests
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/animation/test_animation.cpp")
    add_kgk_test(test_animation animation/test_animation.cpp)
endif()

# Theme tests
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/theme/test_theme.cpp")
    add_kgk_test(test_theme theme/test_theme.cpp)
endif()

# KGK2D tests
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/kgk2d/test_kgk2d.cpp")
    add_kgk_test(test_kgk2d kgk2d/test_kgk2d.cpp)
    target_link_libraries(test_kgk2d PRIVATE KGK2D)
endif()

# KGK3D tests
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/kgk3d/test_kgk3d.cpp")
    add_kgk_test(test_kgk3d kgk3d/test_kgk3d.cpp)
    target_link_libraries(test_kgk3d PRIVATE KGK3D)
endif()

# KGKAudio tests
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/kgkaudio/test_kgkaudio.cpp")
    add_kgk_test(test_kgkaudio kgkaudio/test_kgkaudio.cpp)
    target_link_libraries(test_kgkaudio PRIVATE KGKAudio)
endif()

# KGKNet tests
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/kgknet/test_kgknet.cpp")
    add_kgk_test(test_kgknet kgknet/test_kgknet.cpp)
    target_link_libraries(test_kgknet PRIVATE KGKNet)
endif()

# KGKMedia tests
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/kgkmedia/test_kgkmedia.cpp")
    add_kgk_test(test_kgkmedia kgkmedia/test_kgkmedia.cpp)
    target_link_libraries(test_kgkmedia PRIVATE KGKMedia)
endif()

# Property-based tests (separate target for clarity)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/properties/test_properties.cpp")
    add_kgk_test(test_properties properties/test_properties.cpp)
endif()

# =============================================================================
# Custom Test Targets
# =============================================================================

# Custom target to run all tests
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all KillerGK tests"
)

# Custom target to run only property-based tests
add_custom_target(run_property_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R "test_properties"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running property-based tests"
)

# Custom target to run only unit tests (exclude property tests)
add_custom_target(run_unit_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -E "test_properties"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running unit tests"
)

# =============================================================================
# Test Summary
# =============================================================================
message(STATUS "")
message(STATUS "KillerGK Test Configuration:")
message(STATUS "  Google Test:     v1.14.0")
message(STATUS "  RapidCheck:      latest (property-based testing)")
message(STATUS "  Min PBT Iterations: ${KGK_PBT_MIN_ITERATIONS}")
message(STATUS "")
