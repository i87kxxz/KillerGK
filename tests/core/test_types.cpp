/**
 * @file test_types.cpp
 * @brief Unit tests and property tests for core types
 */

#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>

#include "KillerGK/core/Types.hpp"

using namespace KillerGK;

// ============================================================================
// Color Unit Tests
// ============================================================================

TEST(ColorTest, DefaultConstructor) {
    Color c;
    EXPECT_FLOAT_EQ(c.r, 0.0f);
    EXPECT_FLOAT_EQ(c.g, 0.0f);
    EXPECT_FLOAT_EQ(c.b, 0.0f);
    EXPECT_FLOAT_EQ(c.a, 1.0f);
}

TEST(ColorTest, ParameterizedConstructor) {
    Color c(0.5f, 0.6f, 0.7f, 0.8f);
    EXPECT_FLOAT_EQ(c.r, 0.5f);
    EXPECT_FLOAT_EQ(c.g, 0.6f);
    EXPECT_FLOAT_EQ(c.b, 0.7f);
    EXPECT_FLOAT_EQ(c.a, 0.8f);
}

TEST(ColorTest, HexParsing) {
    Color c = Color::hex("#FF0000");
    EXPECT_FLOAT_EQ(c.r, 1.0f);
    EXPECT_FLOAT_EQ(c.g, 0.0f);
    EXPECT_FLOAT_EQ(c.b, 0.0f);
    EXPECT_FLOAT_EQ(c.a, 1.0f);
}

TEST(ColorTest, HexParsingShorthand) {
    Color c = Color::hex("#F00");
    EXPECT_FLOAT_EQ(c.r, 1.0f);
    EXPECT_FLOAT_EQ(c.g, 0.0f);
    EXPECT_FLOAT_EQ(c.b, 0.0f);
}

TEST(ColorTest, HexParsingWithAlpha) {
    Color c = Color::hex("#FF000080");
    EXPECT_FLOAT_EQ(c.r, 1.0f);
    EXPECT_NEAR(c.a, 0.5f, 0.01f);
}

TEST(ColorTest, RgbFactory) {
    Color c = Color::rgb(255, 128, 0);
    EXPECT_FLOAT_EQ(c.r, 1.0f);
    EXPECT_NEAR(c.g, 0.5f, 0.01f);
    EXPECT_FLOAT_EQ(c.b, 0.0f);
}

TEST(ColorTest, RgbaFactory) {
    Color c = Color::rgba(255, 0, 0, 0.5f);
    EXPECT_FLOAT_EQ(c.r, 1.0f);
    EXPECT_FLOAT_EQ(c.a, 0.5f);
}

TEST(ColorTest, Lighten) {
    Color c(0.5f, 0.5f, 0.5f, 1.0f);
    Color lighter = c.lighten(0.2f);
    EXPECT_FLOAT_EQ(lighter.r, 0.7f);
    EXPECT_FLOAT_EQ(lighter.g, 0.7f);
    EXPECT_FLOAT_EQ(lighter.b, 0.7f);
}

TEST(ColorTest, Darken) {
    Color c(0.5f, 0.5f, 0.5f, 1.0f);
    Color darker = c.darken(0.2f);
    EXPECT_FLOAT_EQ(darker.r, 0.3f);
    EXPECT_FLOAT_EQ(darker.g, 0.3f);
    EXPECT_FLOAT_EQ(darker.b, 0.3f);
}

TEST(ColorTest, WithAlpha) {
    Color c(1.0f, 0.0f, 0.0f, 1.0f);
    Color transparent = c.withAlpha(0.5f);
    EXPECT_FLOAT_EQ(transparent.r, 1.0f);
    EXPECT_FLOAT_EQ(transparent.a, 0.5f);
}

TEST(ColorTest, PredefinedColors) {
    EXPECT_EQ(Color::White, Color(1.0f, 1.0f, 1.0f, 1.0f));
    EXPECT_EQ(Color::Black, Color(0.0f, 0.0f, 0.0f, 1.0f));
    EXPECT_EQ(Color::Red, Color(1.0f, 0.0f, 0.0f, 1.0f));
}

// ============================================================================
// Rect Unit Tests
// ============================================================================

TEST(RectTest, DefaultConstructor) {
    Rect r;
    EXPECT_FLOAT_EQ(r.x, 0.0f);
    EXPECT_FLOAT_EQ(r.y, 0.0f);
    EXPECT_FLOAT_EQ(r.width, 0.0f);
    EXPECT_FLOAT_EQ(r.height, 0.0f);
}

TEST(RectTest, Contains) {
    Rect r(10, 10, 100, 100);
    EXPECT_TRUE(r.contains(50, 50));
    EXPECT_TRUE(r.contains(10, 10));
    EXPECT_TRUE(r.contains(110, 110));
    EXPECT_FALSE(r.contains(5, 50));
    EXPECT_FALSE(r.contains(50, 5));
}

TEST(RectTest, Intersects) {
    Rect r1(0, 0, 100, 100);
    Rect r2(50, 50, 100, 100);
    Rect r3(200, 200, 50, 50);
    
    EXPECT_TRUE(r1.intersects(r2));
    EXPECT_TRUE(r2.intersects(r1));
    EXPECT_FALSE(r1.intersects(r3));
}

TEST(RectTest, Expand) {
    Rect r(10, 10, 100, 100);
    Rect expanded = r.expand(5);
    
    EXPECT_FLOAT_EQ(expanded.x, 5);
    EXPECT_FLOAT_EQ(expanded.y, 5);
    EXPECT_FLOAT_EQ(expanded.width, 110);
    EXPECT_FLOAT_EQ(expanded.height, 110);
}

// ============================================================================
// Point Unit Tests
// ============================================================================

TEST(PointTest, DefaultConstructor) {
    Point p;
    EXPECT_FLOAT_EQ(p.x, 0.0f);
    EXPECT_FLOAT_EQ(p.y, 0.0f);
}

TEST(PointTest, ParameterizedConstructor) {
    Point p(10.5f, 20.5f);
    EXPECT_FLOAT_EQ(p.x, 10.5f);
    EXPECT_FLOAT_EQ(p.y, 20.5f);
}

// ============================================================================
// Size Unit Tests
// ============================================================================

TEST(SizeTest, DefaultConstructor) {
    Size s;
    EXPECT_FLOAT_EQ(s.width, 0.0f);
    EXPECT_FLOAT_EQ(s.height, 0.0f);
}

TEST(SizeTest, ParameterizedConstructor) {
    Size s(100.0f, 200.0f);
    EXPECT_FLOAT_EQ(s.width, 100.0f);
    EXPECT_FLOAT_EQ(s.height, 200.0f);
}
