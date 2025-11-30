/**
 * @file test_helpers.hpp
 * @brief Test helper utilities and RapidCheck generators for KillerGK
 * 
 * This file provides:
 * - RapidCheck arbitrary generators for KillerGK types
 * - Helper macros for property-based tests
 * - Common test utilities
 * 
 * Testing Framework:
 * - Unit Testing: Google Test (gtest)
 * - Property-Based Testing: RapidCheck
 * 
 * Property Test Annotation Format:
 *   // **Feature: killergk-gui-library, Property N: Property Name**
 *   // **Validates: Requirements X.Y**
 * 
 * @see design.md Testing Strategy section
 */

#pragma once

#include <rapidcheck.h>
#include "KillerGK/core/Types.hpp"

namespace rc {

// =============================================================================
// RapidCheck Arbitrary Generators for KillerGK Types
// =============================================================================

/**
 * @brief Generator for normalized float [0.0, 1.0]
 */
inline Gen<float> genNormalizedFloat() {
    return gen::map(gen::inRange(0, 1000), [](int v) {
        return static_cast<float>(v) / 1000.0f;
    });
}

/**
 * @brief Generator for float in range [min, max]
 */
inline Gen<float> genFloatInRange(float min, float max) {
    return gen::map(gen::inRange(0, 10000), [min, max](int v) {
        return min + (static_cast<float>(v) / 10000.0f) * (max - min);
    });
}

/**
 * @brief Generator for Color type
 * Generates colors with all components in valid range [0.0, 1.0]
 */
template<>
struct Arbitrary<KillerGK::Color> {
    static Gen<KillerGK::Color> arbitrary() {
        return gen::build<KillerGK::Color>(
            gen::set(&KillerGK::Color::r, genNormalizedFloat()),
            gen::set(&KillerGK::Color::g, genNormalizedFloat()),
            gen::set(&KillerGK::Color::b, genNormalizedFloat()),
            gen::set(&KillerGK::Color::a, genNormalizedFloat())
        );
    }
};

/**
 * @brief Generator for Point type
 * Generates points with coordinates in reasonable range
 */
template<>
struct Arbitrary<KillerGK::Point> {
    static Gen<KillerGK::Point> arbitrary() {
        return gen::build<KillerGK::Point>(
            gen::set(&KillerGK::Point::x, genFloatInRange(-10000.0f, 10000.0f)),
            gen::set(&KillerGK::Point::y, genFloatInRange(-10000.0f, 10000.0f))
        );
    }
};

/**
 * @brief Generator for Size type
 * Generates sizes with non-negative dimensions
 */
template<>
struct Arbitrary<KillerGK::Size> {
    static Gen<KillerGK::Size> arbitrary() {
        return gen::build<KillerGK::Size>(
            gen::set(&KillerGK::Size::width, genFloatInRange(0.0f, 10000.0f)),
            gen::set(&KillerGK::Size::height, genFloatInRange(0.0f, 10000.0f))
        );
    }
};

/**
 * @brief Generator for Rect type
 * Generates rectangles with non-negative dimensions
 */
template<>
struct Arbitrary<KillerGK::Rect> {
    static Gen<KillerGK::Rect> arbitrary() {
        return gen::build<KillerGK::Rect>(
            gen::set(&KillerGK::Rect::x, genFloatInRange(-10000.0f, 10000.0f)),
            gen::set(&KillerGK::Rect::y, genFloatInRange(-10000.0f, 10000.0f)),
            gen::set(&KillerGK::Rect::width, genFloatInRange(0.0f, 10000.0f)),
            gen::set(&KillerGK::Rect::height, genFloatInRange(0.0f, 10000.0f))
        );
    }
};

} // namespace rc

// =============================================================================
// Property-Based Testing Configuration
// =============================================================================

/**
 * @brief Minimum iterations for property-based tests
 * As specified in design.md Testing Strategy section
 */
constexpr int KGK_MIN_PROPERTY_ITERATIONS = 100;

// =============================================================================
// Test Helper Macros
// =============================================================================

/**
 * @brief Macro for defining property tests with proper annotation
 * 
 * Usage:
 *   KGK_PROPERTY_TEST(TestSuite, TestName, PropertyNumber, RequirementsRef)
 * 
 * Example:
 *   KGK_PROPERTY_TEST(ColorProperties, LightenPreservesRange, 7, "5.1, 5.3")
 */
#define KGK_PROPERTY_TEST(TestSuite, TestName, PropertyNumber, RequirementsRef) \
    RC_GTEST_PROP(TestSuite, TestName, ())

// =============================================================================
// Test Utility Functions
// =============================================================================

namespace KillerGK::TestUtils {

/**
 * @brief Check if two floats are approximately equal
 * @param a First value
 * @param b Second value
 * @param epsilon Maximum allowed difference
 * @return true if values are within epsilon of each other
 */
inline bool approxEqual(float a, float b, float epsilon = 0.0001f) {
    return std::abs(a - b) < epsilon;
}

/**
 * @brief Check if a float is in valid range [0, 1]
 * @param value Value to check
 * @return true if value is in [0, 1]
 */
inline bool isNormalized(float value) {
    return value >= 0.0f && value <= 1.0f;
}

/**
 * @brief Check if a Color has all components in valid range
 * @param color Color to check
 * @return true if all components are in [0, 1]
 */
inline bool isValidColor(const Color& color) {
    return isNormalized(color.r) && 
           isNormalized(color.g) && 
           isNormalized(color.b) && 
           isNormalized(color.a);
}

} // namespace KillerGK::TestUtils
