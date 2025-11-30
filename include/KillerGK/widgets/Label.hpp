/**
 * @file Label.hpp
 * @brief Label widget for KillerGK with Builder Pattern API
 */

#pragma once

#include "Widget.hpp"
#include "../core/Types.hpp"
#include <string>
#include <memory>

namespace KillerGK {

/**
 * @enum TextAlignment
 * @brief Horizontal text alignment options
 */
enum class TextAlignment {
    Left,
    Center,
    Right
};

/**
 * @enum VerticalAlignment
 * @brief Vertical text alignment options
 */
enum class VerticalAlignment {
    Top,
    Middle,
    Bottom
};

/**
 * @enum TextOverflow
 * @brief Text overflow handling options
 */
enum class TextOverflow {
    Clip,       ///< Clip text at boundary
    Ellipsis,   ///< Show ellipsis (...) for overflow
    Wrap        ///< Wrap text to next line
};

/**
 * @enum FontWeight
 * @brief Font weight options
 */
enum class FontWeight {
    Thin = 100,
    ExtraLight = 200,
    Light = 300,
    Normal = 400,
    Medium = 500,
    SemiBold = 600,
    Bold = 700,
    ExtraBold = 800,
    Black = 900
};

/**
 * @enum FontStyle
 * @brief Font style options
 */
enum class FontStyle {
    Normal,
    Italic,
    Oblique
};

/**
 * @class Label
 * @brief Text display widget with alignment and wrapping support
 * 
 * Provides text display with configurable alignment, wrapping,
 * font styling, and overflow handling.
 * 
 * Example:
 * @code
 * auto label = Label::create()
 *     .text("Hello, World!")
 *     .fontSize(16.0f)
 *     .textColor(Color::Black)
 *     .alignment(TextAlignment::Center)
 *     .wrap(true);
 * @endcode
 */
class Label : public Widget {
public:
    virtual ~Label() = default;

    /**
     * @brief Create a new Label instance
     * @return New Label with default properties
     */
    static Label create();

    // =========================================================================
    // Text Content
    // =========================================================================

    /**
     * @brief Set label text
     * @param text Text to display
     * @return Reference to this Label for chaining
     */
    Label& text(const std::string& text);

    /**
     * @brief Get label text
     * @return Current text
     */
    [[nodiscard]] const std::string& getText() const;

    // =========================================================================
    // Text Styling
    // =========================================================================

    /**
     * @brief Set text color
     * @param color Text color
     * @return Reference to this Label for chaining
     */
    Label& textColor(const Color& color);

    /**
     * @brief Get text color
     * @return Current text color
     */
    [[nodiscard]] const Color& getTextColor() const;

    /**
     * @brief Set font size
     * @param size Font size in pixels
     * @return Reference to this Label for chaining
     */
    Label& fontSize(float size);

    /**
     * @brief Get font size
     * @return Current font size
     */
    [[nodiscard]] float getFontSize() const;

    /**
     * @brief Set font family
     * @param family Font family name
     * @return Reference to this Label for chaining
     */
    Label& fontFamily(const std::string& family);

    /**
     * @brief Get font family
     * @return Current font family
     */
    [[nodiscard]] const std::string& getFontFamily() const;

    /**
     * @brief Set font weight
     * @param weight Font weight
     * @return Reference to this Label for chaining
     */
    Label& fontWeight(FontWeight weight);

    /**
     * @brief Get font weight
     * @return Current font weight
     */
    [[nodiscard]] FontWeight getFontWeight() const;

    /**
     * @brief Set font style
     * @param style Font style
     * @return Reference to this Label for chaining
     */
    Label& fontStyle(FontStyle style);

    /**
     * @brief Get font style
     * @return Current font style
     */
    [[nodiscard]] FontStyle getFontStyle() const;

    /**
     * @brief Set line height multiplier
     * @param height Line height as multiplier of font size
     * @return Reference to this Label for chaining
     */
    Label& lineHeight(float height);

    /**
     * @brief Get line height multiplier
     * @return Current line height
     */
    [[nodiscard]] float getLineHeight() const;

    /**
     * @brief Set letter spacing
     * @param spacing Letter spacing in pixels
     * @return Reference to this Label for chaining
     */
    Label& letterSpacing(float spacing);

    /**
     * @brief Get letter spacing
     * @return Current letter spacing
     */
    [[nodiscard]] float getLetterSpacing() const;

    // =========================================================================
    // Alignment
    // =========================================================================

    /**
     * @brief Set horizontal text alignment
     * @param align Text alignment
     * @return Reference to this Label for chaining
     */
    Label& alignment(TextAlignment align);

    /**
     * @brief Get horizontal text alignment
     * @return Current alignment
     */
    [[nodiscard]] TextAlignment getAlignment() const;

    /**
     * @brief Set vertical text alignment
     * @param align Vertical alignment
     * @return Reference to this Label for chaining
     */
    Label& verticalAlignment(VerticalAlignment align);

    /**
     * @brief Get vertical text alignment
     * @return Current vertical alignment
     */
    [[nodiscard]] VerticalAlignment getVerticalAlignment() const;

    // =========================================================================
    // Text Wrapping and Overflow
    // =========================================================================

    /**
     * @brief Enable or disable text wrapping
     * @param enabled Whether wrapping is enabled
     * @return Reference to this Label for chaining
     */
    Label& wrap(bool enabled);

    /**
     * @brief Check if text wrapping is enabled
     * @return true if wrapping is enabled
     */
    [[nodiscard]] bool isWrapping() const;

    /**
     * @brief Set text overflow behavior
     * @param overflow Overflow handling mode
     * @return Reference to this Label for chaining
     */
    Label& overflow(TextOverflow overflow);

    /**
     * @brief Get text overflow behavior
     * @return Current overflow mode
     */
    [[nodiscard]] TextOverflow getOverflow() const;

    /**
     * @brief Set maximum number of lines (0 = unlimited)
     * @param lines Maximum lines
     * @return Reference to this Label for chaining
     */
    Label& maxLines(int lines);

    /**
     * @brief Get maximum number of lines
     * @return Maximum lines (0 = unlimited)
     */
    [[nodiscard]] int getMaxLines() const;

    // =========================================================================
    // Text Decoration
    // =========================================================================

    /**
     * @brief Enable or disable underline
     * @param enabled Whether underline is enabled
     * @return Reference to this Label for chaining
     */
    Label& underline(bool enabled);

    /**
     * @brief Check if underline is enabled
     * @return true if underline is enabled
     */
    [[nodiscard]] bool hasUnderline() const;

    /**
     * @brief Enable or disable strikethrough
     * @param enabled Whether strikethrough is enabled
     * @return Reference to this Label for chaining
     */
    Label& strikethrough(bool enabled);

    /**
     * @brief Check if strikethrough is enabled
     * @return true if strikethrough is enabled
     */
    [[nodiscard]] bool hasStrikethrough() const;

    // =========================================================================
    // Selection
    // =========================================================================

    /**
     * @brief Enable or disable text selection
     * @param enabled Whether selection is enabled
     * @return Reference to this Label for chaining
     */
    Label& selectable(bool enabled);

    /**
     * @brief Check if text selection is enabled
     * @return true if selectable
     */
    [[nodiscard]] bool isSelectable() const;

protected:
    Label();

    struct LabelData;
    std::shared_ptr<LabelData> m_labelData;
};

} // namespace KillerGK
