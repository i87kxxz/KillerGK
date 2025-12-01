/**
 * @file TextRenderer.cpp
 * @brief Text rendering and layout engine implementation
 */

#include "KillerGK/text/TextRenderer.hpp"
#include "KillerGK/rendering/Renderer2D.hpp"
#include <algorithm>
#include <cmath>

namespace KillerGK {

TextRenderer& TextRenderer::instance() {
    static TextRenderer instance;
    return instance;
}

bool TextRenderer::initialize() {
    if (m_initialized) return true;
    
    // Ensure font manager is initialized
    if (!FontManager::instance().initialize()) {
        return false;
    }
    
    m_initialized = true;
    return true;
}

void TextRenderer::shutdown() {
    m_initialized = false;
}

uint32_t TextRenderer::decodeUTF8(const std::string& text, size_t& index) {
    if (index >= text.size()) return 0;
    
    uint8_t c = static_cast<uint8_t>(text[index]);
    uint32_t codepoint = 0;
    
    if ((c & 0x80) == 0) {
        codepoint = c;
        index += 1;
    } else if ((c & 0xE0) == 0xC0) {
        codepoint = (c & 0x1F) << 6;
        if (index + 1 < text.size()) {
            codepoint |= (static_cast<uint8_t>(text[index + 1]) & 0x3F);
        }
        index += 2;
    } else if ((c & 0xF0) == 0xE0) {
        codepoint = (c & 0x0F) << 12;
        if (index + 1 < text.size()) {
            codepoint |= (static_cast<uint8_t>(text[index + 1]) & 0x3F) << 6;
        }
        if (index + 2 < text.size()) {
            codepoint |= (static_cast<uint8_t>(text[index + 2]) & 0x3F);
        }
        index += 3;
    } else if ((c & 0xF8) == 0xF0) {
        codepoint = (c & 0x07) << 18;
        if (index + 1 < text.size()) {
            codepoint |= (static_cast<uint8_t>(text[index + 1]) & 0x3F) << 12;
        }
        if (index + 2 < text.size()) {
            codepoint |= (static_cast<uint8_t>(text[index + 2]) & 0x3F) << 6;
        }
        if (index + 3 < text.size()) {
            codepoint |= (static_cast<uint8_t>(text[index + 3]) & 0x3F);
        }
        index += 4;
    } else {
        index += 1;
    }
    
    return codepoint;
}

bool TextRenderer::isWordBreak(uint32_t codepoint) {
    return isWhitespace(codepoint) || 
           codepoint == '-' || 
           codepoint == '/' ||
           codepoint == '\\';
}

bool TextRenderer::isWhitespace(uint32_t codepoint) {
    return codepoint == ' ' || 
           codepoint == '\t' || 
           codepoint == '\n' || 
           codepoint == '\r';
}

TextLayout TextRenderer::layoutText(const std::string& text, const Rect& bounds,
                                     const TextStyle& style) {
    TextLayout layout;
    layout.bounds = bounds;
    
    FontHandle font = style.font;
    if (!font) {
        font = FontManager::instance().getDefaultFont();
        if (!font) return layout;
    }
    
    float lineHeightPx = font->getLineHeight() * style.lineHeight;
    float maxWidth = bounds.width;
    float cursorX = 0.0f;
    float cursorY = font->getAscender();
    
    TextLine currentLine;
    currentLine.baseline = cursorY;
    currentLine.startIndex = 0;
    
    size_t wordStartIdx = 0;
    float wordStartX = 0.0f;
    std::vector<PositionedGlyph> wordGlyphs;
    
    uint32_t prevCodepoint = 0;
    size_t i = 0;
    
    while (i < text.size()) {
        size_t charStart = i;
        uint32_t codepoint = decodeUTF8(text, i);
        
        if (codepoint == 0) break;
        
        // Handle newline
        if (codepoint == '\n') {
            // Flush word
            for (auto& g : wordGlyphs) {
                currentLine.glyphs.push_back(g);
            }
            wordGlyphs.clear();
            
            currentLine.width = cursorX;
            currentLine.height = lineHeightPx;
            currentLine.endIndex = charStart;
            layout.lines.push_back(currentLine);
            
            // Check max lines
            if (style.maxLines > 0 && 
                static_cast<int>(layout.lines.size()) >= style.maxLines) {
                layout.truncated = true;
                break;
            }
            
            // Start new line
            cursorX = 0.0f;
            cursorY += lineHeightPx;
            currentLine = TextLine();
            currentLine.baseline = cursorY;
            currentLine.startIndex = i;
            wordStartIdx = i;
            wordStartX = 0.0f;
            prevCodepoint = 0;
            continue;
        }
        
        // Get glyph
        const Glyph* glyph = font->getGlyph(codepoint);
        if (!glyph) {
            // Try to load glyph on demand
            const_cast<Font*>(font.get())->loadGlyph(codepoint);
            glyph = font->getGlyph(codepoint);
        }
        
        if (!glyph) continue;
        
        // Calculate advance with kerning and spacing
        float advance = glyph->advance + style.letterSpacing;
        if (prevCodepoint != 0) {
            advance += font->getKerning(prevCodepoint, codepoint);
        }
        if (isWhitespace(codepoint)) {
            advance += style.wordSpacing;
        }
        
        // Check for word wrap
        if (style.wordWrap && maxWidth > 0 && cursorX + advance > maxWidth) {
            // Word doesn't fit, wrap to next line
            if (!wordGlyphs.empty() && wordStartX > 0) {
                // Move word to next line
                currentLine.width = wordStartX;
                currentLine.height = lineHeightPx;
                currentLine.endIndex = wordStartIdx;
                layout.lines.push_back(currentLine);
                
                // Check max lines
                if (style.maxLines > 0 && 
                    static_cast<int>(layout.lines.size()) >= style.maxLines) {
                    layout.truncated = true;
                    break;
                }
                
                // Start new line with word
                cursorY += lineHeightPx;
                currentLine = TextLine();
                currentLine.baseline = cursorY;
                currentLine.startIndex = wordStartIdx;
                
                // Reposition word glyphs
                float offsetX = wordStartX;
                cursorX = 0.0f;
                for (auto& g : wordGlyphs) {
                    g.x -= offsetX;
                    cursorX = std::max(cursorX, g.x + g.glyph->advance);
                    currentLine.glyphs.push_back(g);
                }
                wordGlyphs.clear();
                wordStartX = 0.0f;
            } else {
                // Single word too long, force break
                currentLine.width = cursorX;
                currentLine.height = lineHeightPx;
                currentLine.endIndex = charStart;
                layout.lines.push_back(currentLine);
                
                if (style.maxLines > 0 && 
                    static_cast<int>(layout.lines.size()) >= style.maxLines) {
                    layout.truncated = true;
                    break;
                }
                
                cursorX = 0.0f;
                cursorY += lineHeightPx;
                currentLine = TextLine();
                currentLine.baseline = cursorY;
                currentLine.startIndex = charStart;
                wordGlyphs.clear();
                wordStartX = 0.0f;
            }
        }
        
        // Create positioned glyph
        PositionedGlyph pg;
        pg.glyph = glyph;
        pg.x = cursorX + glyph->bearingX;
        pg.y = cursorY - glyph->bearingY;
        pg.color = style.color;
        
        // Track word boundaries
        if (isWordBreak(codepoint)) {
            // Flush word to line
            for (auto& g : wordGlyphs) {
                currentLine.glyphs.push_back(g);
            }
            wordGlyphs.clear();
            currentLine.glyphs.push_back(pg);
            wordStartIdx = i;
            wordStartX = cursorX + advance;
        } else {
            wordGlyphs.push_back(pg);
        }
        
        cursorX += advance;
        prevCodepoint = codepoint;
    }
    
    // Flush remaining word and line
    if (!layout.truncated) {
        for (auto& g : wordGlyphs) {
            currentLine.glyphs.push_back(g);
        }
        
        if (!currentLine.glyphs.empty()) {
            currentLine.width = cursorX;
            currentLine.height = lineHeightPx;
            currentLine.endIndex = text.size();
            layout.lines.push_back(currentLine);
        }
    }
    
    // Calculate total dimensions
    layout.totalWidth = 0.0f;
    layout.totalHeight = 0.0f;
    for (const auto& line : layout.lines) {
        layout.totalWidth = std::max(layout.totalWidth, line.width);
        layout.totalHeight += line.height;
    }
    
    // Apply alignment
    applyAlignment(layout, bounds, style);
    
    return layout;
}

void TextRenderer::applyAlignment(TextLayout& layout, const Rect& bounds,
                                   const TextStyle& style) {
    // Horizontal alignment
    for (auto& line : layout.lines) {
        float offsetX = 0.0f;
        
        switch (style.align) {
            case TextAlign::Left:
                offsetX = bounds.x;
                break;
            case TextAlign::Center:
                offsetX = bounds.x + (bounds.width - line.width) / 2.0f;
                break;
            case TextAlign::Right:
                offsetX = bounds.x + bounds.width - line.width;
                break;
            case TextAlign::Justify:
                // Justify is handled separately
                offsetX = bounds.x;
                break;
        }
        
        for (auto& glyph : line.glyphs) {
            glyph.x += offsetX;
        }
    }
    
    // Vertical alignment
    float offsetY = 0.0f;
    switch (style.verticalAlign) {
        case TextVerticalAlign::Top:
            offsetY = bounds.y;
            break;
        case TextVerticalAlign::Middle:
            offsetY = bounds.y + (bounds.height - layout.totalHeight) / 2.0f;
            break;
        case TextVerticalAlign::Bottom:
            offsetY = bounds.y + bounds.height - layout.totalHeight;
            break;
    }
    
    for (auto& line : layout.lines) {
        for (auto& glyph : line.glyphs) {
            glyph.y += offsetY;
        }
        line.baseline += offsetY;
    }
}

void TextRenderer::renderLayout(const TextLayout& layout) {
    // Get font atlas texture
    FontHandle font = FontManager::instance().getDefaultFont();
    if (!font || !font->getAtlasTexture()) return;
    
    TextureHandle atlas = font->getAtlasTexture();
    
    // Render each glyph
    for (const auto& line : layout.lines) {
        for (const auto& pg : line.glyphs) {
            if (!pg.glyph) continue;
            
            // Source rectangle in atlas
            Rect srcRect(
                static_cast<float>(pg.glyph->atlasX),
                static_cast<float>(pg.glyph->atlasY),
                pg.glyph->width,
                pg.glyph->height
            );
            
            // Destination rectangle
            Rect dstRect(
                pg.x,
                pg.y,
                pg.glyph->width,
                pg.glyph->height
            );
            
            // Draw glyph using Renderer2D
            Renderer2D::instance().drawTexturedRect(dstRect, atlas, srcRect, pg.color);
        }
    }
}

void TextRenderer::renderText(const std::string& text, float x, float y,
                               const TextStyle& style) {
    // Create bounds at position with unlimited size
    Rect bounds(x, y, 10000.0f, 10000.0f);
    TextLayout layout = layoutText(text, bounds, style);
    renderLayout(layout);
}

void TextRenderer::renderTextInBounds(const std::string& text, const Rect& bounds,
                                       const TextStyle& style) {
    TextLayout layout = layoutText(text, bounds, style);
    renderLayout(layout);
}

Size TextRenderer::measureText(const std::string& text, const TextStyle& style) {
    Rect bounds(0, 0, 10000.0f, 10000.0f);
    TextStyle measureStyle = style;
    measureStyle.wordWrap = false;
    
    TextLayout layout = layoutText(text, bounds, measureStyle);
    return Size(layout.totalWidth, layout.totalHeight);
}

int TextRenderer::getCharacterIndexAt(const TextLayout& layout, float x, float y) {
    // Find line at y position
    float lineY = 0.0f;
    for (size_t lineIdx = 0; lineIdx < layout.lines.size(); ++lineIdx) {
        const auto& line = layout.lines[lineIdx];
        if (y >= lineY && y < lineY + line.height) {
            // Found line, find character at x
            for (size_t i = 0; i < line.glyphs.size(); ++i) {
                const auto& glyph = line.glyphs[i];
                float glyphRight = glyph.x + (glyph.glyph ? glyph.glyph->advance : 0);
                if (x < glyphRight) {
                    return static_cast<int>(line.startIndex + i);
                }
            }
            // Past end of line
            return static_cast<int>(line.endIndex);
        }
        lineY += line.height;
    }
    
    return -1;
}

Point TextRenderer::getCharacterPosition(const TextLayout& layout, size_t index) {
    for (const auto& line : layout.lines) {
        if (index >= line.startIndex && index <= line.endIndex) {
            size_t localIdx = index - line.startIndex;
            if (localIdx < line.glyphs.size()) {
                return Point(line.glyphs[localIdx].x, line.baseline);
            } else if (!line.glyphs.empty()) {
                // End of line
                const auto& lastGlyph = line.glyphs.back();
                float x = lastGlyph.x + (lastGlyph.glyph ? lastGlyph.glyph->advance : 0);
                return Point(x, line.baseline);
            }
        }
    }
    
    return Point(0, 0);
}

} // namespace KillerGK
