# Contributing to KillerGK

Thank you for your interest in contributing to KillerGK. This document provides guidelines and instructions for contributing.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Making Changes](#making-changes)
- [Coding Standards](#coding-standards)
- [Testing](#testing)
- [Submitting Changes](#submitting-changes)
- [Issue Reporting](#issue-reporting)

## Code of Conduct

By participating in this project, you agree to maintain a respectful and inclusive environment. Be considerate of others and focus on constructive feedback.

## Getting Started

1. Fork the repository on GitHub
2. Clone your fork locally
3. Set up the development environment
4. Create a branch for your changes
5. Make your changes and test them
6. Submit a pull request

## Development Setup

### Prerequisites

- C++20 compatible compiler (MSVC 2019+, GCC 10+, Clang 12+)
- CMake 3.20 or higher
- Vulkan SDK 1.3+
- Git

### Building from Source

```bash
# Clone your fork
git clone https://github.com/YOUR_USERNAME/KillerGK.git
cd KillerGK

# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build . --config Debug

# Run tests
ctest --output-on-failure
```

## Making Changes

### Branch Naming

Use descriptive branch names:

- `feature/widget-name` - New features
- `fix/issue-description` - Bug fixes
- `docs/topic` - Documentation updates
- `refactor/component` - Code refactoring

### Commit Messages

Write clear, concise commit messages:

```
type: Short description (max 50 chars)

Longer description if needed. Explain what and why,
not how. Wrap at 72 characters.

Fixes #123
```

Types: `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`

## Coding Standards

### C++ Style Guide

- Use C++20 features where appropriate
- Follow the existing code style in the project
- Use meaningful variable and function names
- Document public APIs with Doxygen comments

### Header Files

```cpp
/**
 * @file ClassName.hpp
 * @brief Brief description of the class
 */

#pragma once

#include "Dependencies.hpp"

namespace KillerGK {

/**
 * @class ClassName
 * @brief Brief description
 * 
 * Detailed description of the class.
 */
class ClassName {
public:
    /**
     * @brief Method description
     * @param param Parameter description
     * @return Return value description
     */
    ReturnType methodName(ParamType param);

private:
    MemberType m_memberName;
};

} // namespace KillerGK
```

### Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Classes | PascalCase | `TextField` |
| Methods | camelCase | `getText()` |
| Variables | camelCase | `textColor` |
| Member Variables | m_ prefix | `m_data` |
| Constants | UPPER_SNAKE | `MAX_LENGTH` |
| Namespaces | PascalCase | `KillerGK` |

### Builder Pattern

All widgets should follow the Builder Pattern:

```cpp
auto widget = Widget::create()
    .id("myWidget")
    .width(100)
    .height(50)
    .backgroundColor(Color::Blue);
```

## Testing

### Running Tests

```bash
# Build tests
cmake --build build --target test_core_types
cmake --build build --target test_properties

# Run unit tests
./build/bin/Debug/test_core_types

# Run property-based tests
./build/bin/Debug/test_properties
```

### Writing Tests

- Write unit tests for new functionality
- Add property-based tests for correctness properties
- Ensure all tests pass before submitting

### Test Structure

```cpp
// Unit test example
TEST(WidgetTest, PropertyIsPreserved) {
    auto widget = Widget::create().width(100);
    EXPECT_EQ(widget.getWidth(), 100);
}

// Property-based test example
RC_GTEST_PROP(WidgetProperties, BuilderPreservesWidth, ()) {
    auto width = *rc::gen::inRange(0, 10000);
    auto widget = Widget::create().width(static_cast<float>(width));
    RC_ASSERT(widget.getWidth() == static_cast<float>(width));
}
```

## Submitting Changes

### Pull Request Process

1. Ensure your code follows the coding standards
2. Update documentation if needed
3. Add tests for new functionality
4. Ensure all tests pass
5. Update the changelog if applicable
6. Submit the pull request

### Pull Request Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Refactoring

## Testing
Describe how you tested your changes

## Checklist
- [ ] Code follows project style guidelines
- [ ] Self-reviewed the code
- [ ] Added necessary documentation
- [ ] Added tests for new functionality
- [ ] All tests pass
```

## Issue Reporting

### Bug Reports

Include the following information:

- KillerGK version
- Operating system and version
- Steps to reproduce
- Expected behavior
- Actual behavior
- Error messages or logs

### Feature Requests

Describe:

- The problem you're trying to solve
- Your proposed solution
- Alternative solutions considered
- Additional context

## Questions

If you have questions, feel free to:

- Open a GitHub issue
- Check existing documentation
- Review closed issues for similar questions

---

Thank you for contributing to KillerGK!
