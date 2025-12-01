cmake_minimum_required(VERSION 3.20)
project(KillerGK VERSION 1.0.0 LANGUAGES CXX)

# =============================================================================
# C++ Standard Configuration
# =============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =============================================================================
# Build Options
# =============================================================================
option(KGK_BUILD_TESTS "Build tests" ON)
option(KGK_BUILD_EXAMPLES "Build examples" ON)
option(KGK_BUILD_TOOLS "Build CLI tools" ON)
option(KGK_BUILD_DOCS "Build documentation" OFF)
option(KGK_BUILD_SHARED "Build shared library instead of static" OFF)
option(KGK_ENABLE_ASSIMP "Enable Assimp for 3D model loading" ON)
option(KGK_ENABLE_MINIAUDIO "Enable miniaudio for audio support" ON)

# =============================================================================
# Output Directories
# =============================================================================
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Multi-config generators (Visual Studio, Xcode)
foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG_TYPE} CONFIG_TYPE_UPPER)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG_TYPE})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG_TYPE})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_BINARY_DIR}/bin/${CONFIG_TYPE})
endforeach()

# =============================================================================
# FetchContent for External Dependencies
# =============================================================================
include(FetchContent)

# =============================================================================
# Required Dependencies
# =============================================================================

# Vulkan - Required for rendering
find_package(Vulkan REQUIRED)
message(STATUS "Found Vulkan: ${Vulkan_INCLUDE_DIRS}")

# GLFW - Required for windowing
find_package(glfw3 CONFIG QUIET)
if(NOT glfw3_FOUND)
    find_package(glfw3 REQUIRED)
endif()
message(STATUS "Found GLFW3")

# GLM - Required for math operations
find_package(glm CONFIG QUIET)
if(NOT glm_FOUND)
    find_package(glm REQUIRED)
endif()
message(STATUS "Found GLM")

# =============================================================================
# Optional Dependencies
# =============================================================================

# FreeType - Optional for text rendering
find_package(Freetype QUIET)
if(Freetype_FOUND)
    message(STATUS "Found FreeType: ${FREETYPE_INCLUDE_DIRS}")
else()
    message(STATUS "FreeType not found - text rendering will be limited")
endif()

# Assimp - Optional for 3D model loading
if(KGK_ENABLE_ASSIMP)
    find_package(assimp CONFIG QUIET)
    if(NOT assimp_FOUND)
        find_package(assimp QUIET)
    endif()
    if(assimp_FOUND)
        message(STATUS "Found Assimp")
    else()
        message(STATUS "Assimp not found - 3D model loading will be limited")
    endif()
endif()

# =============================================================================
# Header-Only Dependencies (stb_image, miniaudio)
# =============================================================================

# Create external directory if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/external)

# stb_image - Header-only image loading library
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/stb/stb_image.h")
    message(STATUS "Fetching stb libraries...")
    FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG master
    )
    FetchContent_GetProperties(stb)
    if(NOT stb_POPULATED)
        FetchContent_Populate(stb)
        # Copy only the headers we need
        file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/external/stb)
        file(COPY ${stb_SOURCE_DIR}/stb_image.h DESTINATION ${CMAKE_SOURCE_DIR}/external/stb)
        file(COPY ${stb_SOURCE_DIR}/stb_image_write.h DESTINATION ${CMAKE_SOURCE_DIR}/external/stb)
    endif()
endif()
message(STATUS "stb_image available at: ${CMAKE_SOURCE_DIR}/external/stb")

# miniaudio - Header-only audio library
if(KGK_ENABLE_MINIAUDIO)
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/miniaudio/miniaudio.h")
        message(STATUS "Fetching miniaudio...")
        FetchContent_Declare(
            miniaudio
            GIT_REPOSITORY https://github.com/mackron/miniaudio.git
            GIT_TAG master
        )
        FetchContent_GetProperties(miniaudio)
        if(NOT miniaudio_POPULATED)
            FetchContent_Populate(miniaudio)
            file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/external/miniaudio)
            file(COPY ${miniaudio_SOURCE_DIR}/miniaudio.h DESTINATION ${CMAKE_SOURCE_DIR}/external/miniaudio)
        endif()
    endif()
    message(STATUS "miniaudio available at: ${CMAKE_SOURCE_DIR}/external/miniaudio")
endif()

# =============================================================================
# Source Files Collection
# =============================================================================
file(GLOB_RECURSE KGK_CORE_SOURCES "src/core/*.cpp")
file(GLOB_RECURSE KGK_PLATFORM_SOURCES "src/platform/*.cpp")
file(GLOB_RECURSE KGK_RENDERING_SOURCES "src/rendering/*.cpp")
file(GLOB_RECURSE KGK_WIDGETS_SOURCES "src/widgets/*.cpp")
file(GLOB_RECURSE KGK_LAYOUT_SOURCES "src/layout/*.cpp")
file(GLOB_RECURSE KGK_ANIMATION_SOURCES "src/animation/*.cpp")
file(GLOB_RECURSE KGK_THEME_SOURCES "src/theme/*.cpp")
file(GLOB_RECURSE KGK_TEXT_SOURCES "src/text/*.cpp")
file(GLOB_RECURSE KGK_RESOURCES_SOURCES "src/resources/*.cpp")
file(GLOB_RECURSE KGK_2D_SOURCES "src/kgk2d/*.cpp")
file(GLOB_RECURSE KGK_3D_SOURCES "src/kgk3d/*.cpp")
file(GLOB_RECURSE KGK_AUDIO_SOURCES "src/kgkaudio/*.cpp")
file(GLOB_RECURSE KGK_NET_SOURCES "src/kgknet/*.cpp")
file(GLOB_RECURSE KGK_MEDIA_SOURCES "src/kgkmedia/*.cpp")

# =============================================================================
# Library Type Selection
# =============================================================================
if(KGK_BUILD_SHARED)
    set(KGK_LIBRARY_TYPE SHARED)
else()
    set(KGK_LIBRARY_TYPE STATIC)
endif()

# =============================================================================
# Main KillerGK Library
# =============================================================================
add_library(KillerGK ${KGK_LIBRARY_TYPE}
    ${KGK_CORE_SOURCES}
    ${KGK_PLATFORM_SOURCES}
    ${KGK_RENDERING_SOURCES}
    ${KGK_WIDGETS_SOURCES}
    ${KGK_LAYOUT_SOURCES}
    ${KGK_ANIMATION_SOURCES}
    ${KGK_THEME_SOURCES}
    ${KGK_TEXT_SOURCES}
    ${KGK_RESOURCES_SOURCES}
)

# Set library properties
set_target_properties(KillerGK PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

# Include directories
target_include_directories(KillerGK PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/external>
    $<INSTALL_INTERFACE:include>
)

# Link required dependencies
target_link_libraries(KillerGK PUBLIC
    Vulkan::Vulkan
    glfw
    glm::glm
)

# Link optional FreeType
if(Freetype_FOUND)
    target_link_libraries(KillerGK PUBLIC Freetype::Freetype)
    target_compile_definitions(KillerGK PUBLIC KGK_HAS_FREETYPE)
endif()

# Platform-specific libraries for OS integration
if(WIN32)
    target_link_libraries(KillerGK PRIVATE
        dwmapi
        shcore
        shell32
        ole32
        dxgi
    )
endif()

# =============================================================================
# KGK2D Module - 2D Graphics
# =============================================================================
add_library(KGK2D ${KGK_LIBRARY_TYPE} ${KGK_2D_SOURCES})
set_target_properties(KGK2D PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
target_link_libraries(KGK2D PUBLIC KillerGK)

# =============================================================================
# KGK3D Module - 3D Graphics
# =============================================================================
add_library(KGK3D ${KGK_LIBRARY_TYPE} ${KGK_3D_SOURCES})
set_target_properties(KGK3D PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
target_link_libraries(KGK3D PUBLIC KillerGK)

# Link Assimp if available
if(KGK_ENABLE_ASSIMP AND assimp_FOUND)
    target_link_libraries(KGK3D PUBLIC assimp::assimp)
    target_compile_definitions(KGK3D PUBLIC KGK_HAS_ASSIMP)
endif()

# =============================================================================
# KGKAudio Module - Audio
# =============================================================================
add_library(KGKAudio ${KGK_LIBRARY_TYPE} ${KGK_AUDIO_SOURCES})
set_target_properties(KGKAudio PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
target_link_libraries(KGKAudio PUBLIC KillerGK)

# Add miniaudio support
if(KGK_ENABLE_MINIAUDIO AND EXISTS "${CMAKE_SOURCE_DIR}/external/miniaudio/miniaudio.h")
    target_include_directories(KGKAudio PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/external/miniaudio>
    )
    target_compile_definitions(KGKAudio PUBLIC KGK_HAS_MINIAUDIO)
endif()

# =============================================================================
# KGKNet Module - Networking
# =============================================================================
add_library(KGKNet ${KGK_LIBRARY_TYPE} ${KGK_NET_SOURCES})
set_target_properties(KGKNet PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
target_link_libraries(KGKNet PUBLIC KillerGK)

# Platform-specific networking libraries
if(WIN32)
    target_link_libraries(KGKNet PUBLIC ws2_32 winhttp)
elseif(UNIX)
    find_package(CURL QUIET)
    if(CURL_FOUND)
        target_link_libraries(KGKNet PUBLIC CURL::libcurl)
        target_compile_definitions(KGKNet PUBLIC KGK_HAS_CURL)
    endif()
endif()

# =============================================================================
# KGKMedia Module - Media Processing
# =============================================================================
add_library(KGKMedia ${KGK_LIBRARY_TYPE} ${KGK_MEDIA_SOURCES})
set_target_properties(KGKMedia PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
target_link_libraries(KGKMedia PUBLIC KillerGK)

# =============================================================================
# Compiler Configuration
# =============================================================================

# All library targets
set(KGK_ALL_TARGETS KillerGK KGK2D KGK3D KGKAudio KGKNet KGKMedia)

# Compiler warnings for all targets
foreach(TARGET ${KGK_ALL_TARGETS})
    if(MSVC)
        target_compile_options(${TARGET} PRIVATE
            /W4                 # Warning level 4
            /WX-                # Don't treat warnings as errors (for now)
            /permissive-        # Strict conformance mode
            /Zc:__cplusplus     # Report correct __cplusplus value
        )
    else()
        target_compile_options(${TARGET} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wno-unused-parameter   # Allow unused parameters for interface methods
        )
    endif()
endforeach()

# =============================================================================
# Debug/Release Build Configuration
# =============================================================================

# Use generator expressions for multi-config generators (Visual Studio, Xcode)
foreach(TARGET ${KGK_ALL_TARGETS})
    # Debug configuration
    target_compile_definitions(${TARGET} PRIVATE
        $<$<CONFIG:Debug>:KGK_DEBUG>
        $<$<CONFIG:Debug>:_DEBUG>
    )
    
    # Release configuration
    target_compile_definitions(${TARGET} PRIVATE
        $<$<CONFIG:Release>:KGK_RELEASE>
        $<$<CONFIG:Release>:NDEBUG>
    )
    
    # RelWithDebInfo configuration
    target_compile_definitions(${TARGET} PRIVATE
        $<$<CONFIG:RelWithDebInfo>:KGK_RELEASE>
        $<$<CONFIG:RelWithDebInfo>:NDEBUG>
    )
    
    # MinSizeRel configuration
    target_compile_definitions(${TARGET} PRIVATE
        $<$<CONFIG:MinSizeRel>:KGK_RELEASE>
        $<$<CONFIG:MinSizeRel>:NDEBUG>
    )
    
    # Platform-specific debug/release options
    if(MSVC)
        target_compile_options(${TARGET} PRIVATE
            $<$<CONFIG:Debug>:/Zi /Od /RTC1>
            $<$<CONFIG:Release>:/O2 /Oi /GL>
            $<$<CONFIG:RelWithDebInfo>:/O2 /Zi>
            $<$<CONFIG:MinSizeRel>:/O1>
        )
        target_link_options(${TARGET} PRIVATE
            $<$<CONFIG:Release>:/LTCG>
        )
    else()
        target_compile_options(${TARGET} PRIVATE
            $<$<CONFIG:Debug>:-g -O0>
            $<$<CONFIG:Release>:-O3 -DNDEBUG>
            $<$<CONFIG:RelWithDebInfo>:-O2 -g>
            $<$<CONFIG:MinSizeRel>:-Os>
        )
    endif()
endforeach()

# Single-config generator fallback (Makefiles, Ninja)
if(NOT CMAKE_CONFIGURATION_TYPES)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    endif()
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

# =============================================================================
# Tests
# =============================================================================
if(KGK_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# =============================================================================
# Examples
# =============================================================================
if(KGK_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# =============================================================================
# CLI Tools
# =============================================================================
if(KGK_BUILD_TOOLS)
    add_subdirectory(tools/kgk-cli)
endif()

# =============================================================================
# Installation Configuration
# =============================================================================
include(GNUInstallDirs)

# Install all library targets
install(TARGETS ${KGK_ALL_TARGETS}
    EXPORT KillerGKTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT Runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT Development
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT Runtime
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install header files
install(DIRECTORY include/KillerGK
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Development
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

# Install external headers (stb, miniaudio)
install(DIRECTORY external/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KillerGK/external
    COMPONENT Development
    FILES_MATCHING PATTERN "*.h"
)

# =============================================================================
# CMake Package Configuration (find_package support)
# =============================================================================

# Export targets for find_package
install(EXPORT KillerGKTargets
    FILE KillerGKTargets.cmake
    NAMESPACE KillerGK::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KillerGK
    COMPONENT Development
)

# Create and install config file
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/KillerGKConfig.cmake.in
    ${CMAKE_BINARY_DIR}/KillerGKConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KillerGK
)

write_basic_package_version_file(
    ${CMAKE_BINARY_DIR}/KillerGKConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_BINARY_DIR}/KillerGKConfig.cmake
    ${CMAKE_BINARY_DIR}/KillerGKConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KillerGK
    COMPONENT Development
)

# =============================================================================
# Export Build Tree (for use without installing)
# =============================================================================
export(EXPORT KillerGKTargets
    FILE ${CMAKE_BINARY_DIR}/KillerGKTargets.cmake
    NAMESPACE KillerGK::
)

# Register package in user's CMake package registry
export(PACKAGE KillerGK)

# =============================================================================
# Summary
# =============================================================================
message(STATUS "")
message(STATUS "KillerGK Configuration Summary:")
message(STATUS "  Version:        ${PROJECT_VERSION}")
message(STATUS "  C++ Standard:   ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  Library Type:   ${KGK_LIBRARY_TYPE}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  Vulkan:         Found")
message(STATUS "  GLFW:           Found")
message(STATUS "  GLM:            Found")
message(STATUS "  FreeType:       ${Freetype_FOUND}")
message(STATUS "  Assimp:         ${assimp_FOUND}")
message(STATUS "  stb_image:      Available")
if(KGK_ENABLE_MINIAUDIO)
    message(STATUS "  miniaudio:      Available")
endif()
message(STATUS "")
message(STATUS "Build Options:")
message(STATUS "  Build Tests:    ${KGK_BUILD_TESTS}")
message(STATUS "  Build Examples: ${KGK_BUILD_EXAMPLES}")
message(STATUS "  Build Tools:    ${KGK_BUILD_TOOLS}")
message(STATUS "  Build Docs:     ${KGK_BUILD_DOCS}")
message(STATUS "")
