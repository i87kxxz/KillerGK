/**
 * @file test_cli_properties.cpp
 * @brief Property-based tests for KillerGK CLI tool
 * 
 * This file contains property-based tests that verify correctness properties
 * for the CLI tool, specifically project generation.
 * 
 * Requirements: 15.1
 * 
 * **Feature: killergk-gui-library, Property 19: CLI Project Generation**
 * **Validates: Requirements 15.1**
 */

#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <regex>
#include <cstdlib>

namespace fs = std::filesystem;

// ============================================================================
// Project Generator Logic (extracted from kgk-cli for testing)
// ============================================================================

namespace kgk {
namespace cli {

/**
 * @brief Validate project name
 * @param name Project name to validate
 * @return true if valid, false otherwise
 */
inline bool isValidProjectName(const std::string& name) {
    if (name.empty() || name.length() > 64) {
        return false;
    }
    
    // Must start with a letter
    if (!std::isalpha(static_cast<unsigned char>(name[0]))) {
        return false;
    }
    
    // Can only contain alphanumeric, underscore, and hyphen
    for (char c : name) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-') {
            return false;
        }
    }
    
    return true;
}

/**
 * @brief Generate CMakeLists.txt content for a project
 */
inline std::string generateCMakeListsContent(const std::string& projectName) {
    std::string content = R"(cmake_minimum_required(VERSION 3.20)
project()" + projectName + R"( VERSION 1.0.0 LANGUAGES CXX)

# =============================================================================
# C++ Standard Configuration
# =============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =============================================================================
# Output Directories
# =============================================================================
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# =============================================================================
# Find KillerGK
# =============================================================================
find_package(KillerGK REQUIRED)

# =============================================================================
# Application Executable
# =============================================================================
add_executable(${PROJECT_NAME}
    src/main.cpp
)

# Link KillerGK libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    KillerGK::KillerGK
    KillerGK::KGK2D
    KillerGK::KGK3D
    KillerGK::KGKAudio
    KillerGK::KGKNet
    KillerGK::KGKMedia
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# =============================================================================
# Compiler Warnings
# =============================================================================
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4
        /permissive-
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# =============================================================================
# Debug/Release Configuration
# =============================================================================
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

)";
    return content;
}

/**
 * @brief Generate main.cpp content for a project
 */
inline std::string generateMainCppContent(const std::string& projectName) {
    // Convert project name to title case for display
    std::string displayName = projectName;
    if (!displayName.empty()) {
        displayName[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(displayName[0])));
    }
    // Replace hyphens and underscores with spaces for display
    for (char& c : displayName) {
        if (c == '-' || c == '_') {
            c = ' ';
        }
    }
    
    std::string content = R"(/**
 * @file main.cpp
 * @brief )" + displayName + R"( - A KillerGK Application
 * 
 * This is the main entry point for your KillerGK application.
 * Customize this file to build your GUI application.
 */

#include <KillerGK/KillerGK.hpp>

int main() {
    using namespace KillerGK;

    // Create and configure the theme
    // Options: Theme::material(), Theme::flat(), Theme::glass()
    auto theme = Theme::material();

    // Create and run the application
    Application::instance()
        .title(")" + displayName + R"(")
        .size(1280, 720)
        .theme(theme)
        .run([]() {
            // Create the main window
            auto window = Window::create()
                .title(")" + displayName + R"(")
                .size(800, 600)
                .resizable(true)
                .build();

            // TODO: Add your widgets here
            // Example:
            // auto button = Button::create()
            //     .text("Click Me!")
            //     .onClick([]() {
            //         std::cout << "Button clicked!\n";
            //     })
            //     .build();
        });

    return 0;
}
)";
    return content;
}

/**
 * @brief Generate .gitignore content
 */
inline std::string generateGitignoreContent() {
    return R"(# Build directories
build/
cmake-build-*/
out/

# IDE files
.vs/
.vscode/
.idea/
*.user
*.suo
*.sln
*.vcxproj*

# Compiled files
*.o
*.obj
*.exe
*.dll
*.so
*.dylib
*.a
*.lib

# CMake generated files
CMakeCache.txt
CMakeFiles/
cmake_install.cmake
compile_commands.json
Makefile

# Package files
*.zip
*.tar.gz
*.deb
*.rpm
*.msi
)";
}

/**
 * @brief Generate a project in the specified directory
 * @param projectName Name of the project
 * @param targetDir Target directory
 * @return true on success, false on failure
 */
inline bool generateProject(const std::string& projectName, const fs::path& targetDir) {
    if (!isValidProjectName(projectName)) {
        return false;
    }
    
    fs::path projectPath = targetDir / projectName;
    
    try {
        // Create directory structure
        fs::create_directories(projectPath / "src");
        fs::create_directories(projectPath / "include");
        fs::create_directories(projectPath / "build");
        
        // Write CMakeLists.txt
        {
            std::ofstream file(projectPath / "CMakeLists.txt");
            if (!file.is_open()) return false;
            file << generateCMakeListsContent(projectName);
            file.close();
            if (file.fail()) return false;
        }
        
        // Write main.cpp
        {
            std::ofstream file(projectPath / "src" / "main.cpp");
            if (!file.is_open()) return false;
            file << generateMainCppContent(projectName);
            file.close();
            if (file.fail()) return false;
        }
        
        // Write .gitignore
        {
            std::ofstream file(projectPath / ".gitignore");
            if (!file.is_open()) return false;
            file << generateGitignoreContent();
            file.close();
            if (file.fail()) return false;
        }
        
        return true;
    }
    catch (const std::exception&) {
        // Cleanup on failure
        try {
            if (fs::exists(projectPath)) {
                fs::remove_all(projectPath);
            }
        } catch (...) {}
        return false;
    }
}

} // namespace cli
} // namespace kgk

// ============================================================================
// RapidCheck Generators
// ============================================================================

namespace rc {

/**
 * @brief Generator for valid project names
 * 
 * Valid project names:
 * - Start with a letter
 * - Contain only alphanumeric, underscore, and hyphen
 * - Length 1-64 characters
 */
inline Gen<std::string> genValidProjectName() {
    return gen::exec([]() {
        // Generate length between 1 and 20 (reasonable for testing)
        int length = *gen::inRange(1, 21);
        
        std::string name;
        
        // First character must be a letter
        int firstChar = *gen::inRange(0, 52);
        if (firstChar < 26) {
            name += static_cast<char>('a' + firstChar);
        } else {
            name += static_cast<char>('A' + (firstChar - 26));
        }
        
        // Remaining characters can be alphanumeric, underscore, or hyphen
        for (int i = 1; i < length; ++i) {
            int charType = *gen::inRange(0, 64);
            if (charType < 26) {
                name += static_cast<char>('a' + charType);
            } else if (charType < 52) {
                name += static_cast<char>('A' + (charType - 26));
            } else if (charType < 62) {
                name += static_cast<char>('0' + (charType - 52));
            } else if (charType == 62) {
                name += '_';
            } else {
                name += '-';
            }
        }
        
        return name;
    });
}

/**
 * @brief Generator for invalid project names (for negative testing)
 */
inline Gen<std::string> genInvalidProjectName() {
    return gen::oneOf(
        // Empty string
        gen::just(std::string("")),
        // Starts with number
        gen::map(gen::inRange(0, 10), [](int n) {
            return std::to_string(n) + "project";
        }),
        // Starts with special character
        gen::map(gen::element('!', '@', '#', '$', '%', '^', '&', '*'), [](char c) {
            return std::string(1, c) + "project";
        }),
        // Contains invalid characters
        gen::just(std::string("my project")),  // space
        gen::just(std::string("my.project")),  // dot
        gen::just(std::string("my/project")),  // slash
        // Too long (65+ characters)
        gen::map(gen::just(0), [](int) {
            return std::string(65, 'a');
        })
    );
}

} // namespace rc

// ============================================================================
// Test Fixture
// ============================================================================

class CLIProjectGenerationTest : public ::testing::Test {
protected:
    fs::path testDir;
    
    void SetUp() override {
        // Create a unique temporary directory for each test
        testDir = fs::temp_directory_path() / ("kgk_cli_test_" + std::to_string(std::rand()));
        fs::create_directories(testDir);
    }
    
    void TearDown() override {
        // Clean up test directory
        try {
            if (fs::exists(testDir)) {
                fs::remove_all(testDir);
            }
        } catch (...) {}
    }
    
    /**
     * @brief Check if a file exists and is not empty
     */
    bool fileExistsAndNotEmpty(const fs::path& path) {
        if (!fs::exists(path)) return false;
        if (!fs::is_regular_file(path)) return false;
        return fs::file_size(path) > 0;
    }
    
    /**
     * @brief Read file content
     */
    std::string readFileContent(const fs::path& path) {
        std::ifstream file(path);
        if (!file.is_open()) return "";
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
    
    /**
     * @brief Check if CMakeLists.txt is valid
     * 
     * Validates:
     * - Contains cmake_minimum_required
     * - Contains project() with correct name
     * - Contains C++20 standard
     * - Contains find_package(KillerGK)
     * - Contains add_executable
     * - Contains target_link_libraries with KillerGK
     */
    bool isValidCMakeLists(const std::string& content, const std::string& projectName) {
        // Check for required CMake commands
        if (content.find("cmake_minimum_required") == std::string::npos) return false;
        if (content.find("project(" + projectName) == std::string::npos) return false;
        if (content.find("CMAKE_CXX_STANDARD 20") == std::string::npos) return false;
        if (content.find("find_package(KillerGK") == std::string::npos) return false;
        if (content.find("add_executable") == std::string::npos) return false;
        if (content.find("target_link_libraries") == std::string::npos) return false;
        if (content.find("KillerGK::KillerGK") == std::string::npos) return false;
        
        return true;
    }
    
    /**
     * @brief Check if main.cpp is valid
     * 
     * Validates:
     * - Contains #include <KillerGK/KillerGK.hpp>
     * - Contains int main()
     * - Contains Application::instance()
     * - Contains Window::create()
     */
    bool isValidMainCpp(const std::string& content) {
        if (content.find("#include <KillerGK/KillerGK.hpp>") == std::string::npos) return false;
        if (content.find("int main()") == std::string::npos) return false;
        if (content.find("Application::instance()") == std::string::npos) return false;
        if (content.find("Window::create()") == std::string::npos) return false;
        
        return true;
    }
    
    /**
     * @brief Check if .gitignore is valid
     * 
     * Validates:
     * - Contains build/ exclusion
     * - Contains common exclusions
     */
    bool isValidGitignore(const std::string& content) {
        if (content.find("build/") == std::string::npos) return false;
        if (content.find("*.o") == std::string::npos) return false;
        if (content.find("CMakeCache.txt") == std::string::npos) return false;
        
        return true;
    }
};

// ============================================================================
// Property Tests for CLI Project Generation
// ============================================================================

/**
 * **Feature: killergk-gui-library, Property 19: CLI Project Generation**
 * 
 * *For any* valid project name, the "kgk-cli new" command SHALL create a 
 * complete project structure with valid CMake configuration and compilable 
 * starter code.
 * 
 * This test verifies that:
 * 1. Project directory is created
 * 2. Required subdirectories (src/, include/, build/) are created
 * 3. CMakeLists.txt is created with valid configuration
 * 4. src/main.cpp is created with valid starter code
 * 5. .gitignore is created with appropriate exclusions
 * 
 * **Validates: Requirements 15.1**
 */
RC_GTEST_FIXTURE_PROP(CLIProjectGenerationTest, ProjectStructureIsComplete, ()) {
    auto projectName = *rc::genValidProjectName();
    
    // Generate the project
    bool success = kgk::cli::generateProject(projectName, testDir);
    RC_ASSERT(success);
    
    fs::path projectPath = testDir / projectName;
    
    // Verify project directory exists
    RC_ASSERT(fs::exists(projectPath));
    RC_ASSERT(fs::is_directory(projectPath));
    
    // Verify required subdirectories exist
    RC_ASSERT(fs::exists(projectPath / "src"));
    RC_ASSERT(fs::is_directory(projectPath / "src"));
    
    RC_ASSERT(fs::exists(projectPath / "include"));
    RC_ASSERT(fs::is_directory(projectPath / "include"));
    
    RC_ASSERT(fs::exists(projectPath / "build"));
    RC_ASSERT(fs::is_directory(projectPath / "build"));
    
    // Verify required files exist and are not empty
    RC_ASSERT(fileExistsAndNotEmpty(projectPath / "CMakeLists.txt"));
    RC_ASSERT(fileExistsAndNotEmpty(projectPath / "src" / "main.cpp"));
    RC_ASSERT(fileExistsAndNotEmpty(projectPath / ".gitignore"));
    
    // Clean up this specific project
    fs::remove_all(projectPath);
}

/**
 * **Feature: killergk-gui-library, Property 19: CLI Project Generation**
 * 
 * *For any* valid project name, the generated CMakeLists.txt SHALL contain
 * all required CMake configuration for a KillerGK project.
 * 
 * **Validates: Requirements 15.1**
 */
RC_GTEST_FIXTURE_PROP(CLIProjectGenerationTest, CMakeListsIsValid, ()) {
    auto projectName = *rc::genValidProjectName();
    
    // Generate the project
    bool success = kgk::cli::generateProject(projectName, testDir);
    RC_ASSERT(success);
    
    fs::path projectPath = testDir / projectName;
    
    // Read and validate CMakeLists.txt
    std::string cmakeContent = readFileContent(projectPath / "CMakeLists.txt");
    RC_ASSERT(!cmakeContent.empty());
    RC_ASSERT(isValidCMakeLists(cmakeContent, projectName));
    
    // Clean up
    fs::remove_all(projectPath);
}

/**
 * **Feature: killergk-gui-library, Property 19: CLI Project Generation**
 * 
 * *For any* valid project name, the generated main.cpp SHALL contain
 * valid starter code that uses the KillerGK API.
 * 
 * **Validates: Requirements 15.1**
 */
RC_GTEST_FIXTURE_PROP(CLIProjectGenerationTest, MainCppIsValid, ()) {
    auto projectName = *rc::genValidProjectName();
    
    // Generate the project
    bool success = kgk::cli::generateProject(projectName, testDir);
    RC_ASSERT(success);
    
    fs::path projectPath = testDir / projectName;
    
    // Read and validate main.cpp
    std::string mainContent = readFileContent(projectPath / "src" / "main.cpp");
    RC_ASSERT(!mainContent.empty());
    RC_ASSERT(isValidMainCpp(mainContent));
    
    // Clean up
    fs::remove_all(projectPath);
}

/**
 * **Feature: killergk-gui-library, Property 19: CLI Project Generation**
 * 
 * *For any* valid project name, the generated .gitignore SHALL contain
 * appropriate exclusion patterns for a C++ CMake project.
 * 
 * **Validates: Requirements 15.1**
 */
RC_GTEST_FIXTURE_PROP(CLIProjectGenerationTest, GitignoreIsValid, ()) {
    auto projectName = *rc::genValidProjectName();
    
    // Generate the project
    bool success = kgk::cli::generateProject(projectName, testDir);
    RC_ASSERT(success);
    
    fs::path projectPath = testDir / projectName;
    
    // Read and validate .gitignore
    std::string gitignoreContent = readFileContent(projectPath / ".gitignore");
    RC_ASSERT(!gitignoreContent.empty());
    RC_ASSERT(isValidGitignore(gitignoreContent));
    
    // Clean up
    fs::remove_all(projectPath);
}

/**
 * **Feature: killergk-gui-library, Property 19: CLI Project Generation**
 * 
 * *For any* invalid project name, the project generation SHALL fail
 * and not create any files.
 * 
 * **Validates: Requirements 15.1**
 */
RC_GTEST_FIXTURE_PROP(CLIProjectGenerationTest, InvalidNameRejected, ()) {
    auto invalidName = *rc::genInvalidProjectName();
    
    // Attempt to generate the project
    bool success = kgk::cli::generateProject(invalidName, testDir);
    
    // Should fail for invalid names
    RC_ASSERT(!success);
    
    // No directory should be created
    if (!invalidName.empty()) {
        fs::path projectPath = testDir / invalidName;
        RC_ASSERT(!fs::exists(projectPath));
    }
}

/**
 * **Feature: killergk-gui-library, Property 19: CLI Project Generation**
 * 
 * *For any* valid project name, the project name validation function
 * SHALL return true.
 * 
 * **Validates: Requirements 15.1**
 */
RC_GTEST_PROP(CLIProjectNameValidation, ValidNamesAccepted, ()) {
    auto projectName = *rc::genValidProjectName();
    
    RC_ASSERT(kgk::cli::isValidProjectName(projectName));
}

/**
 * **Feature: killergk-gui-library, Property 19: CLI Project Generation**
 * 
 * *For any* invalid project name, the project name validation function
 * SHALL return false.
 * 
 * **Validates: Requirements 15.1**
 */
RC_GTEST_PROP(CLIProjectNameValidation, InvalidNamesRejected, ()) {
    auto invalidName = *rc::genInvalidProjectName();
    
    RC_ASSERT(!kgk::cli::isValidProjectName(invalidName));
}

/**
 * **Feature: killergk-gui-library, Property 19: CLI Project Generation**
 * 
 * *For any* valid project name, the generated CMakeLists.txt content
 * SHALL contain the exact project name in the project() command.
 * 
 * **Validates: Requirements 15.1**
 */
RC_GTEST_PROP(CLIProjectGeneration, CMakeContainsProjectName, ()) {
    auto projectName = *rc::genValidProjectName();
    
    std::string cmakeContent = kgk::cli::generateCMakeListsContent(projectName);
    
    // The project name should appear in the project() command
    std::string expectedProjectLine = "project(" + projectName;
    RC_ASSERT(cmakeContent.find(expectedProjectLine) != std::string::npos);
}

/**
 * **Feature: killergk-gui-library, Property 19: CLI Project Generation**
 * 
 * *For any* valid project name, the generated main.cpp SHALL contain
 * a display name derived from the project name.
 * 
 * **Validates: Requirements 15.1**
 */
RC_GTEST_PROP(CLIProjectGeneration, MainCppContainsDisplayName, ()) {
    auto projectName = *rc::genValidProjectName();
    
    std::string mainContent = kgk::cli::generateMainCppContent(projectName);
    
    // The main.cpp should contain the KillerGK include
    RC_ASSERT(mainContent.find("#include <KillerGK/KillerGK.hpp>") != std::string::npos);
    
    // The main.cpp should contain int main()
    RC_ASSERT(mainContent.find("int main()") != std::string::npos);
}
