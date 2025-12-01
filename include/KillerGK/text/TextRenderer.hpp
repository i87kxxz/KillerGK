/**
 * @file TextRenderer.hpp
 * @brief Text rendering and layout engine for KillerGK
 * 
 * Provides text layout, rendering with subpixel anti-aliasing,
 * text alignment, and word wrapping.
 */

#pragma once

#include "../core/Types.hpp"
#include "Font.hpp"
#include <memory>
#include <string>
#include <vector>

namespace KillerGK {

/**
 * @brief Text alignment options
 */
enum class TextAlign {
    Left,
    Center,
    Right,
    Justify
};

/**
 * @brief Vertical text alignment
 */
enum class TextVerticalAlign {
    Top,
    Middle,
    Bottom
};

/**
 * @brief Text overflow handling
 */
enum class TextOverflow {
    Visible,    ///< Text extends beyond bounds
    Clip,       ///< Text is clipped at bounds
    Ellipsis    ///< Text is truncated with "..."
};

/**
 * @brief Text decoration options
 */
enum class TextDecoration : uint32_t {
    None = 0,
    Underline = 1 << 0,
    Strikethrough = 1 << 1,
    Overline = 1 << 2
};

inline TextDecoration operator|(TextDecoration a, TextDecoration b) {
    return static_cast<TextDecoration>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline bool operator&(TextDecoration a, TextDecoration b) {
    return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0;
}

/**
 * @brief Text style configuration
 */
struct TextStyle {
    FontHandle font;                            ///< Font to use
    float fontSize = 16.0f;                     ///< Font size in pixels
    Color color = Color::White;                 ///< Text color
    TextAlign align = TextAlign::Left;          ///< Horizontal alignment
    TextVerticalAlign verticalAlign = TextVerticalAlign::Top;
    TextOverflow overflow = TextOverflow::Visible;
    TextDecoration decoration = TextDecoration::None;
    float lineHeight = 1.2f;                    ///< Line height multiplier
    float letterSpacing = 0.0f;                 ///< Extra spacing between letters
    float wordSpacing = 0.0f;                   ///< Extra spacing between words
    bool wordWrap = true;                       ///< Enable word wrapping
    int maxLines = 0;                           ///< Max lines (0 = unlimited)
    Color shadowColor = Color::Transparent;     ///< Text shadow color
    float shadowOffsetX = 0.0f;                 ///< Shadow X offset
    float shadowOffsetY = 0.0f;                 ///< Shadow Y offset
    float shadowBlur = 0.0f;                    ///< Shadow blur radius
};

/**
 * @brief A positioned glyph for rendering
 */
struct PositionedGlyph {
    const Glyph* glyph = nullptr;
    float x = 0.0f;
    float y = 0.0f;
    Color color;
};

/**
 * @brief A line of laid out text
 */
struct TextLine {
    std::vector<PositionedGlyph> glyphs;
    float width = 0.0f;
    float height = 0.0f;
    float baseline = 0.0f;
    size_t startIndex = 0;      ///< Start index in original text
    size_t endIndex = 0;        ///< End index in original text
};

/**
 * @brief Result of text layout
 */
struct TextLayout {
    std::vector<TextLine> lines;
    float totalWidth = 0.0f;
    float totalHeight = 0.0f;
    Rect bounds;
    bool truncated = false;     ///< True if text was truncated
};

/**
 * @class TextRenderer
 * @brief Renders text with layout and styling
 */
class TextRenderer {
public:
    static TextRenderer& instance();
    
    /**
     * @brief Initialize the text renderer
     * @return true if successful
     */
    bool initialize();
    
    /**
     * @brief Shutdown the text renderer
     */
    void shutdown();
    
    /**
     * @brief Layout text within bounds
     * @param text UTF-8 encoded text
     * @param bounds Bounding rectangle
     * @param style Text style
     * @return Layout result with positioned glyphs
     */
    TextLayout layoutText(const std::string& text, const Rect& bounds, 
                          const TextStyle& style);
    
    /**
     * @brief Render laid out text
     * @param layout Text layout from layoutText()
     */
    void renderLayout(const TextLayout& layout);
    
    /**
     * @brief Render text directly (layout + render)
     * @param text UTF-8 encoded text
     * @param x X position
     * @param y Y position
     * @param style Text style
     */
    void renderText(const std::string& text, float x, float y, 
                    const TextStyle& style);
    
    /**
     * @brief Render text within bounds
     * @param text UTF-8 encoded text
     * @param bounds Bounding rectangle
     * @param style Text style
     */
    void renderTextInBounds(const std::string& text, const Rect& bounds,
                            const TextStyle& style);
    
    /**
     * @brief Measure text dimensions
     * @param text UTF-8 encoded text
     * @param style Text style
     * @return Size of rendered text
     */
    Size measureText(const std::string& text, const TextStyle& style);
    
    /**
     * @brief Get character index at position
     * @param layout Text layout
     * @param x X position
     * @param y Y position
     * @return Character index or -1 if not found
     */
    int getCharacterIndexAt(const TextLayout& layout, float x, float y);
    
    /**
     * @brief Get position of character
     * @param layout Text layout
     * @param index Character index
     * @return Position of character
     */
    Point getCharacterPosition(const TextLayout& layout, size_t index);
    
    /**
     * @brief Check if renderer is initialized
     */
    [[nodiscard]] bool isInitialized() const { return m_initialized; }
    
private:
    TextRenderer() = default;
    ~TextRenderer() = default;
    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;
    
    // UTF-8 decoding helper
    static uint32_t decodeUTF8(const std::string& text, size_t& index);
    
    // Word breaking helper
    static bool isWordBreak(uint32_t codepoint);
    static bool isWhitespace(uint32_t codepoint);
    
    // Layout helpers
    void layoutLine(TextLine& line, const TextStyle& style, float maxWidth);
    void applyAlignment(TextLayout& layout, const Rect& bounds, const TextStyle& style);
    
    bool m_initialized = false;
};

} // namespace KillerGK
