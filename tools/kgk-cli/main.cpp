/**
 * @file main.cpp
 * @brief KillerGK CLI Tool - Command-line interface for KillerGK project management
 * 
 * This CLI tool provides commands for creating, building, running, and packaging
 * KillerGK applications.
 * 
 * Requirements: 15.1
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <regex>
#include <cctype>

namespace kgk {
namespace cli {

// Version information
constexpr const char* VERSION = "1.0.0";
constexpr const char* PROGRAM_NAME = "kgk-cli";

/**
 * @brief Represents a CLI command with its metadata and handler
 */
struct Command {
    std::string name;
    std::string shortDescription;
    std::string longDescription;
    std::string usage;
    std::vector<std::string> examples;
    std::function<int(const std::vector<std::string>&)> handler;
};

/**
 * @brief Project generator for the "new" command
 * 
 * Creates a new KillerGK project with:
 * - Directory structure (src/, include/, build/)
 * - CMakeLists.txt with proper configuration
 * - Starter main.cpp code
 * 
 * Requirements: 15.1
 */
class ProjectGenerator {
public:
    /**
     * @brief Validate project name
     * @param name Project name to validate
     * @return true if valid, false otherwise
     */
    static bool isValidProjectName(const std::string& name) {
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
     * @brief Generate a new KillerGK project
     * @param projectName Name of the project
     * @param targetDir Target directory (defaults to current directory)
     * @return 0 on success, non-zero on error
     */
    static int generate(const std::string& projectName, const std::string& targetDir = ".") {
        namespace fs = std::filesystem;
        
        // Validate project name
        if (!isValidProjectName(projectName)) {
            std::cerr << "Error: Invalid project name '" << projectName << "'\n";
            std::cerr << "Project name must:\n";
            std::cerr << "  - Start with a letter\n";
            std::cerr << "  - Contain only letters, numbers, underscores, and hyphens\n";
            std::cerr << "  - Be 1-64 characters long\n";
            return 1;
        }
        
        // Create project directory path
        fs::path projectPath = fs::path(targetDir) / projectName;
        
        // Check if directory already exists
        if (fs::exists(projectPath)) {
            std::cerr << "Error: Directory '" << projectPath.string() << "' already exists.\n";
            return 1;
        }
        
        std::cout << "Creating new KillerGK project: " << projectName << "\n";
        
        try {
            // Create directory structure
            fs::create_directories(projectPath / "src");
            fs::create_directories(projectPath / "include");
            fs::create_directories(projectPath / "build");
            
            // Generate CMakeLists.txt
            if (!generateCMakeLists(projectPath, projectName)) {
                std::cerr << "Error: Failed to create CMakeLists.txt\n";
                return 1;
            }
            
            // Generate main.cpp
            if (!generateMainCpp(projectPath, projectName)) {
                std::cerr << "Error: Failed to create main.cpp\n";
                return 1;
            }
            
            // Generate .gitignore
            if (!generateGitignore(projectPath)) {
                std::cerr << "Error: Failed to create .gitignore\n";
                return 1;
            }
            
            std::cout << "\nProject created successfully!\n\n";
            std::cout << "To get started:\n";
            std::cout << "  cd " << projectName << "\n";
            std::cout << "  mkdir build && cd build\n";
            std::cout << "  cmake ..\n";
            std::cout << "  cmake --build .\n";
            std::cout << "\nOr use kgk-cli:\n";
            std::cout << "  cd " << projectName << "\n";
            std::cout << "  kgk-cli build\n";
            std::cout << "  kgk-cli run\n";
            
            return 0;
        }
        catch (const fs::filesystem_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
            // Cleanup on failure
            try {
                if (fs::exists(projectPath)) {
                    fs::remove_all(projectPath);
                }
            } catch (...) {}
            return 1;
        }
    }

private:
    /**
     * @brief Generate CMakeLists.txt for the project
     */
    static bool generateCMakeLists(const std::filesystem::path& projectPath, 
                                    const std::string& projectName) {
        std::ofstream file(projectPath / "CMakeLists.txt");
        if (!file.is_open()) {
            return false;
        }
        
        file << R"(cmake_minimum_required(VERSION 3.20)
project()" << projectName << R"( VERSION 1.0.0 LANGUAGES CXX)

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
        
        file.close();
        return file.good() || !file.fail();
    }

    /**
     * @brief Generate starter main.cpp
     */
    static bool generateMainCpp(const std::filesystem::path& projectPath,
                                 const std::string& projectName) {
        std::ofstream file(projectPath / "src" / "main.cpp");
        if (!file.is_open()) {
            return false;
        }
        
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
        
        file << R"(/**
 * @file main.cpp
 * @brief )" << displayName << R"( - A KillerGK Application
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
        .title(")" << displayName << R"(")
        .size(1280, 720)
        .theme(theme)
        .run([]() {
            // Create the main window
            auto window = Window::create()
                .title(")" << displayName << R"(")
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
        
        file.close();
        return file.good() || !file.fail();
    }

    /**
     * @brief Generate .gitignore file
     */
    static bool generateGitignore(const std::filesystem::path& projectPath) {
        std::ofstream file(projectPath / ".gitignore");
        if (!file.is_open()) {
            return false;
        }
        
        file << R"(# Build directories
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
        
        file.close();
        return file.good() || !file.fail();
    }
};

/**
 * @brief CLI Application class that manages command parsing and execution
 */
class CLIApplication {
public:
    CLIApplication() {
        registerCommands();
    }

    /**
     * @brief Parse and execute command line arguments
     * @param argc Argument count
     * @param argv Argument values
     * @return Exit code (0 for success, non-zero for errors)
     */
    int run(int argc, char* argv[]) {
        std::vector<std::string> args;
        for (int i = 1; i < argc; ++i) {
            args.push_back(argv[i]);
        }

        // No arguments - show help
        if (args.empty()) {
            return showHelp({});
        }

        const std::string& commandName = args[0];

        // Check for global flags
        if (commandName == "--help" || commandName == "-h") {
            return showHelp({});
        }
        if (commandName == "--version" || commandName == "-v") {
            return showVersion({});
        }

        // Find and execute command
        auto it = m_commands.find(commandName);
        if (it == m_commands.end()) {
            std::cerr << "Error: Unknown command '" << commandName << "'\n";
            std::cerr << "Run '" << PROGRAM_NAME << " --help' for usage.\n";
            return 1;
        }

        // Remove command name from args and pass rest to handler
        std::vector<std::string> commandArgs(args.begin() + 1, args.end());
        
        // Check for command-specific help
        if (!commandArgs.empty() && (commandArgs[0] == "--help" || commandArgs[0] == "-h")) {
            return showCommandHelp(commandName);
        }

        return it->second.handler(commandArgs);
    }

    /**
     * @brief Get all registered commands
     */
    const std::map<std::string, Command>& getCommands() const {
        return m_commands;
    }

private:
    std::map<std::string, Command> m_commands;

    /**
     * @brief Register all available commands
     */
    void registerCommands() {
        // Help command
        m_commands["help"] = {
            "help",
            "Display help information",
            "Display help information about kgk-cli or a specific command.",
            "kgk-cli help [command]",
            {"kgk-cli help", "kgk-cli help new", "kgk-cli help build"},
            [this](const std::vector<std::string>& args) { return showHelp(args); }
        };

        // Version command
        m_commands["version"] = {
            "version",
            "Display version information",
            "Display the current version of kgk-cli and KillerGK library.",
            "kgk-cli version",
            {"kgk-cli version"},
            [this](const std::vector<std::string>& args) { return showVersion(args); }
        };

        // New command - Creates a new KillerGK project
        // Requirements: 15.1
        m_commands["new"] = {
            "new",
            "Create a new KillerGK project",
            "Create a new KillerGK project with CMake configuration and starter code.\n"
            "The project will be created in a new directory with the specified name.\n\n"
            "The generated project includes:\n"
            "  - CMakeLists.txt with proper KillerGK configuration\n"
            "  - src/main.cpp with starter application code\n"
            "  - include/ directory for your headers\n"
            "  - build/ directory for build artifacts\n"
            "  - .gitignore with common exclusions",
            "kgk-cli new <project-name>",
            {"kgk-cli new my-app", "kgk-cli new my-game", "kgk-cli new hello-world"},
            [](const std::vector<std::string>& args) {
                if (args.empty()) {
                    std::cerr << "Error: Project name is required.\n";
                    std::cerr << "Usage: kgk-cli new <project-name>\n";
                    std::cerr << "\nExamples:\n";
                    std::cerr << "  kgk-cli new my-app\n";
                    std::cerr << "  kgk-cli new my-game\n";
                    return 1;
                }
                
                const std::string& projectName = args[0];
                return ProjectGenerator::generate(projectName);
            }
        };

        // Build command (placeholder)
        m_commands["build"] = {
            "build",
            "Build the project",
            "Compile the KillerGK project using CMake.\n"
            "By default, builds in Debug mode. Use --release for optimized builds.",
            "kgk-cli build [options]",
            {"kgk-cli build", "kgk-cli build --release", "kgk-cli build --clean"},
            [](const std::vector<std::string>& args) {
                (void)args;
                std::cout << "Command 'build' is not yet implemented.\n";
                return 0;
            }
        };

        // Run command (placeholder)
        m_commands["run"] = {
            "run",
            "Build and run the project",
            "Build the project (if needed) and execute the application.",
            "kgk-cli run [options]",
            {"kgk-cli run", "kgk-cli run --release"},
            [](const std::vector<std::string>& args) {
                (void)args;
                std::cout << "Command 'run' is not yet implemented.\n";
                return 0;
            }
        };

        // Package command (placeholder)
        m_commands["package"] = {
            "package",
            "Create distributable installer",
            "Package the application into a distributable installer.\n"
            "Creates platform-specific installers using KillerGK's GUI installer framework.",
            "kgk-cli package [options]",
            {"kgk-cli package", "kgk-cli package --format=msi"},
            [](const std::vector<std::string>& args) {
                (void)args;
                std::cout << "Command 'package' is not yet implemented.\n";
                return 0;
            }
        };

        // Clean command (placeholder)
        m_commands["clean"] = {
            "clean",
            "Clean build artifacts",
            "Remove all build artifacts and temporary files.",
            "kgk-cli clean",
            {"kgk-cli clean"},
            [](const std::vector<std::string>& args) {
                (void)args;
                std::cout << "Command 'clean' is not yet implemented.\n";
                return 0;
            }
        };

        // Update command (placeholder)
        m_commands["update"] = {
            "update",
            "Update KillerGK to latest version",
            "Check for and install updates to the KillerGK library.",
            "kgk-cli update",
            {"kgk-cli update"},
            [](const std::vector<std::string>& args) {
                (void)args;
                std::cout << "Command 'update' is not yet implemented.\n";
                return 0;
            }
        };

        // Docs command (placeholder)
        m_commands["docs"] = {
            "docs",
            "Open documentation",
            "Open the KillerGK documentation in your default browser.",
            "kgk-cli docs [topic]",
            {"kgk-cli docs", "kgk-cli docs widgets"},
            [](const std::vector<std::string>& args) {
                (void)args;
                std::cout << "Command 'docs' is not yet implemented.\n";
                return 0;
            }
        };

        // Tutorial command (placeholder)
        m_commands["tutorial"] = {
            "tutorial",
            "Launch tutorial application",
            "Launch the interactive KillerGK tutorial application.",
            "kgk-cli tutorial",
            {"kgk-cli tutorial"},
            [](const std::vector<std::string>& args) {
                (void)args;
                std::cout << "Command 'tutorial' is not yet implemented.\n";
                return 0;
            }
        };
    }

    /**
     * @brief Display general help or help for a specific command
     */
    int showHelp(const std::vector<std::string>& args) {
        if (!args.empty()) {
            return showCommandHelp(args[0]);
        }

        std::cout << "KillerGK CLI - Command-line tool for KillerGK projects\n\n";
        std::cout << "Usage: " << PROGRAM_NAME << " <command> [options]\n\n";
        std::cout << "Commands:\n";

        // Find the longest command name for alignment
        size_t maxLen = 0;
        for (const auto& [name, cmd] : m_commands) {
            maxLen = std::max(maxLen, name.length());
        }

        // Print commands in a nice format
        for (const auto& [name, cmd] : m_commands) {
            std::cout << "  " << std::left << std::setw(static_cast<int>(maxLen + 2)) 
                      << name << cmd.shortDescription << "\n";
        }

        std::cout << "\nGlobal Options:\n";
        std::cout << "  -h, --help      Display this help message\n";
        std::cout << "  -v, --version   Display version information\n";
        std::cout << "\nRun '" << PROGRAM_NAME << " help <command>' for more information on a command.\n";

        return 0;
    }

    /**
     * @brief Display help for a specific command
     */
    int showCommandHelp(const std::string& commandName) {
        auto it = m_commands.find(commandName);
        if (it == m_commands.end()) {
            std::cerr << "Error: Unknown command '" << commandName << "'\n";
            return 1;
        }

        const Command& cmd = it->second;
        std::cout << cmd.name << " - " << cmd.shortDescription << "\n\n";
        std::cout << "Description:\n  " << cmd.longDescription << "\n\n";
        std::cout << "Usage:\n  " << cmd.usage << "\n\n";
        
        if (!cmd.examples.empty()) {
            std::cout << "Examples:\n";
            for (const auto& example : cmd.examples) {
                std::cout << "  " << example << "\n";
            }
        }

        return 0;
    }

    /**
     * @brief Display version information
     */
    int showVersion(const std::vector<std::string>& args) {
        (void)args; // Unused
        std::cout << PROGRAM_NAME << " version " << VERSION << "\n";
        std::cout << "KillerGK GUI Library version 1.0.0\n";
        std::cout << "Copyright (c) 2024 KillerGK Team\n";
        return 0;
    }
};

} // namespace cli
} // namespace kgk

int main(int argc, char* argv[]) {
    kgk::cli::CLIApplication app;
    return app.run(argc, argv);
}
