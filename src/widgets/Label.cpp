/**
 * @file Label.cpp
 * @brief Label widget implementation
 */

#include "KillerGK/widgets/Label.hpp"

namespace KillerGK {

// =============================================================================
// LabelData - Internal data structure
// =============================================================================

struct Label::LabelData {
    std::string text;
    
    // Text styling
    Color textColor = Color::Black;
    float fontSize = 14.0f;
    std::string fontFamily = "sans-serif";
    FontWeight fontWeight = FontWeight::Normal;
    FontStyle fontStyle = FontStyle::Normal;
    float lineHeight = 1.2f;
    float letterSpacing = 0.0f;
    
    // Alignment
    TextAlignment alignment = TextAlignment::Left;
    VerticalAlignment verticalAlignment = VerticalAlignment::Top;
    
    // Wrapping and overflow
    bool wrap = false;
    TextOverflow overflow = TextOverflow::Clip;
    int maxLines = 0;  // 0 = unlimited
    
    // Decoration
    bool underline = false;
    bool strikethrough = false;
    
    // Selection
    bool selectable = false;
};

// =============================================================================
// Label Implementation
// =============================================================================

Label::Label() 
    : Widget()
    , m_labelData(std::make_shared<LabelData>()) 
{
    // Labels are transparent by default
    backgroundColor(Color::Transparent);
}

Label Label::create() {
    return Label();
}

// Text Content
Label& Label::text(const std::string& text) {
    m_labelData->text = text;
    return *this;
}

const std::string& Label::getText() const {
    return m_labelData->text;
}

// Text Styling
Label& Label::textColor(const Color& color) {
    m_labelData->textColor = color;
    return *this;
}

const Color& Label::getTextColor() const {
    return m_labelData->textColor;
}

Label& Label::fontSize(float size) {
    m_labelData->fontSize = size;
    return *this;
}

float Label::getFontSize() const {
    return m_labelData->fontSize;
}

Label& Label::fontFamily(const std::string& family) {
    m_labelData->fontFamily = family;
    return *this;
}

const std::string& Label::getFontFamily() const {
    return m_labelData->fontFamily;
}

Label& Label::fontWeight(FontWeight weight) {
    m_labelData->fontWeight = weight;
    return *this;
}

FontWeight Label::getFontWeight() const {
    return m_labelData->fontWeight;
}

Label& Label::fontStyle(FontStyle style) {
    m_labelData->fontStyle = style;
    return *this;
}

FontStyle Label::getFontStyle() const {
    return m_labelData->fontStyle;
}

Label& Label::lineHeight(float height) {
    m_labelData->lineHeight = height;
    return *this;
}

float Label::getLineHeight() const {
    return m_labelData->lineHeight;
}

Label& Label::letterSpacing(float spacing) {
    m_labelData->letterSpacing = spacing;
    return *this;
}

float Label::getLetterSpacing() const {
    return m_labelData->letterSpacing;
}

// Alignment
Label& Label::alignment(TextAlignment align) {
    m_labelData->alignment = align;
    return *this;
}

TextAlignment Label::getAlignment() const {
    return m_labelData->alignment;
}

Label& Label::verticalAlignment(VerticalAlignment align) {
    m_labelData->verticalAlignment = align;
    return *this;
}

VerticalAlignment Label::getVerticalAlignment() const {
    return m_labelData->verticalAlignment;
}

// Wrapping and Overflow
Label& Label::wrap(bool enabled) {
    m_labelData->wrap = enabled;
    if (enabled) {
        m_labelData->overflow = TextOverflow::Wrap;
    }
    return *this;
}

bool Label::isWrapping() const {
    return m_labelData->wrap;
}

Label& Label::overflow(TextOverflow overflow) {
    m_labelData->overflow = overflow;
    m_labelData->wrap = (overflow == TextOverflow::Wrap);
    return *this;
}

TextOverflow Label::getOverflow() const {
    return m_labelData->overflow;
}

Label& Label::maxLines(int lines) {
    m_labelData->maxLines = lines;
    return *this;
}

int Label::getMaxLines() const {
    return m_labelData->maxLines;
}

// Decoration
Label& Label::underline(bool enabled) {
    m_labelData->underline = enabled;
    return *this;
}

bool Label::hasUnderline() const {
    return m_labelData->underline;
}

Label& Label::strikethrough(bool enabled) {
    m_labelData->strikethrough = enabled;
    return *this;
}

bool Label::hasStrikethrough() const {
    return m_labelData->strikethrough;
}

// Selection
Label& Label::selectable(bool enabled) {
    m_labelData->selectable = enabled;
    return *this;
}

bool Label::isSelectable() const {
    return m_labelData->selectable;
}

} // namespace KillerGK
