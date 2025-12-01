/**
 * @file Font.cpp
 * @brief Font loading and glyph management implementation
 */

#include "KillerGK/text/Font.hpp"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>

#ifdef KGK_HAS_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_LCD_FILTER_H
#endif

namespace KillerGK {

// ============================================================================
// FontManager Implementation
// ============================================================================

FontManager& FontManager::instance() {
    static FontManager instance;
    return instance;
}

FontManager::~FontManager() {
    shutdown();
}

bool FontManager::initialize() {
    if (m_initialized) return true;
    
#ifdef KGK_HAS_FREETYPE
    FT_Library library;
    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
        std::cerr << "Failed to initialize FreeType library" << std::endl;
        return false;
    }
    
    // Enable LCD filtering for subpixel rendering
    FT_Library_SetLcdFilter(library, FT_LCD_FILTER_DEFAULT);
    
    m_ftLibrary = library;
    m_initialized = true;
    return true;
#else
    std::cerr << "FreeType not available - text rendering disabled" << std::endl;
    return false;
#endif
}

void FontManager::shutdown() {
    if (!m_initialized) return;
    
    m_fontCache.clear();
    m_defaultFont.reset();
    
#ifdef KGK_HAS_FREETYPE
    if (m_ftLibrary) {
        FT_Done_FreeType(static_cast<FT_Library>(m_ftLibrary));
        m_ftLibrary = nullptr;
    }
#endif
    
    m_initialized = false;
}

FontHandle FontManager::loadFont(const std::string& path, const FontConfig& config) {
    // Create cache key
    std::string cacheKey = path + "_" + std::to_string(static_cast<int>(config.size));
    
    // Check cache
    auto it = m_fontCache.find(cacheKey);
    if (it != m_fontCache.end()) {
        return it->second;
    }
    
    // Load new font
    FontHandle font = Font::loadFromFile(path, config);
    if (font) {
        m_fontCache[cacheKey] = font;
        
        // Set as default if no default exists
        if (!m_defaultFont) {
            m_defaultFont = font;
        }
    }
    
    return font;
}

FontHandle FontManager::getFont(const std::string& path, float size) const {
    std::string cacheKey = path + "_" + std::to_string(static_cast<int>(size));
    auto it = m_fontCache.find(cacheKey);
    return (it != m_fontCache.end()) ? it->second : nullptr;
}

FontHandle FontManager::getDefaultFont() const {
    return m_defaultFont;
}

void FontManager::setDefaultFont(FontHandle font) {
    m_defaultFont = font;
}

void FontManager::unloadFont(const std::string& path) {
    // Remove all sizes of this font
    for (auto it = m_fontCache.begin(); it != m_fontCache.end();) {
        if (it->first.find(path) == 0) {
            it = m_fontCache.erase(it);
        } else {
            ++it;
        }
    }
}

void FontManager::clearCache() {
    m_fontCache.clear();
}

// ============================================================================
// Font Implementation
// ============================================================================

Font::~Font() {
#ifdef KGK_HAS_FREETYPE
    if (m_ftFace) {
        FT_Done_Face(static_cast<FT_Face>(m_ftFace));
        m_ftFace = nullptr;
    }
#endif
}

FontHandle Font::loadFromFile(const std::string& path, const FontConfig& config) {
    // Read file into memory
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open font file: " << path << std::endl;
        return nullptr;
    }
    
    size_t fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> data(fileSize);
    if (!file.read(reinterpret_cast<char*>(data.data()), fileSize)) {
        std::cerr << "Failed to read font file: " << path << std::endl;
        return nullptr;
    }
    
    FontHandle font = loadFromMemory(data.data(), data.size(), config);
    if (font) {
        font->m_path = path;
    }
    return font;
}

FontHandle Font::loadFromMemory(const uint8_t* data, size_t size, const FontConfig& config) {
#ifdef KGK_HAS_FREETYPE
    FontManager& manager = FontManager::instance();
    if (!manager.isAvailable()) {
        if (!manager.initialize()) {
            return nullptr;
        }
    }
    
    FT_Library library = static_cast<FT_Library>(manager.getFTLibrary());
    
    // Create font instance
    FontHandle font(new Font());
    
    // Keep font data in memory (FreeType requires it to stay valid)
    font->m_fontData.assign(data, data + size);
    
    // Create FreeType face
    FT_Face face;
    FT_Error error = FT_New_Memory_Face(library, 
                                         font->m_fontData.data(), 
                                         static_cast<FT_Long>(font->m_fontData.size()),
                                         0, &face);
    if (error) {
        std::cerr << "Failed to create FreeType face" << std::endl;
        return nullptr;
    }
    
    font->m_ftFace = face;
    
    // Initialize font
    if (!font->initialize(config)) {
        return nullptr;
    }
    
    return font;
#else
    (void)data;
    (void)size;
    (void)config;
    std::cerr << "FreeType not available" << std::endl;
    return nullptr;
#endif
}

bool Font::initialize(const FontConfig& config) {
#ifdef KGK_HAS_FREETYPE
    FT_Face face = static_cast<FT_Face>(m_ftFace);
    
    // Set font size
    FT_Error error = FT_Set_Pixel_Sizes(face, 0, static_cast<FT_UInt>(config.size));
    if (error) {
        std::cerr << "Failed to set font size" << std::endl;
        return false;
    }
    
    m_size = config.size;
    
    // Get font metrics
    m_familyName = face->family_name ? face->family_name : "Unknown";
    m_styleName = face->style_name ? face->style_name : "Regular";
    m_ascender = static_cast<float>(face->size->metrics.ascender) / 64.0f;
    m_descender = static_cast<float>(face->size->metrics.descender) / 64.0f;
    m_lineHeight = static_cast<float>(face->size->metrics.height) / 64.0f;
    
    // Create glyph atlas
    if (!createAtlas(config)) {
        return false;
    }
    
    return true;
#else
    (void)config;
    return false;
#endif
}

bool Font::createAtlas(const FontConfig& config) {
    m_atlasWidth = config.atlasWidth;
    m_atlasHeight = config.atlasHeight;
    m_atlasPadding = config.padding;
    m_atlasCursorX = m_atlasPadding;
    m_atlasCursorY = m_atlasPadding;
    m_atlasRowHeight = 0;
    
    // Allocate atlas pixels (single channel for grayscale)
    m_atlasPixels.resize(static_cast<size_t>(m_atlasWidth) * m_atlasHeight, 0);
    
    // Load basic ASCII range
    loadGlyphRange(config.rangeStart, config.rangeEnd);
    
    // Load extended Latin if requested
    if (config.loadExtendedLatin) {
        loadGlyphRange(128, 255);
    }
    
    // Load Arabic if requested
    if (config.loadArabic) {
        loadGlyphRange(0x0600, 0x06FF);  // Arabic
        loadGlyphRange(0xFB50, 0xFDFF);  // Arabic Presentation Forms-A
        loadGlyphRange(0xFE70, 0xFEFF);  // Arabic Presentation Forms-B
    }
    
    // Load Hebrew if requested
    if (config.loadHebrew) {
        loadGlyphRange(0x0590, 0x05FF);  // Hebrew
        loadGlyphRange(0xFB1D, 0xFB4F);  // Hebrew Presentation Forms
    }
    
    // Create GPU texture
    updateAtlasTexture();
    
    return true;
}

bool Font::loadGlyph(uint32_t codepoint) {
    // Check if already loaded
    if (m_glyphs.find(codepoint) != m_glyphs.end()) {
        return true;
    }
    
    return renderGlyphToAtlas(codepoint, m_atlasCursorX, m_atlasCursorY, m_atlasRowHeight);
}

int Font::loadGlyphRange(uint32_t start, uint32_t end) {
    int loaded = 0;
    for (uint32_t cp = start; cp <= end; ++cp) {
        if (loadGlyph(cp)) {
            ++loaded;
        }
    }
    return loaded;
}

bool Font::renderGlyphToAtlas(uint32_t codepoint, int& cursorX, int& cursorY, int rowHeight) {
#ifdef KGK_HAS_FREETYPE
    FT_Face face = static_cast<FT_Face>(m_ftFace);
    
    // Get glyph index
    FT_UInt glyphIndex = FT_Get_Char_Index(face, codepoint);
    if (glyphIndex == 0 && codepoint != 0) {
        // Glyph not found in font
        return false;
    }
    
    // Load glyph
    FT_Error error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
    if (error) {
        return false;
    }
    
    // Render glyph to bitmap
    error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
    if (error) {
        return false;
    }
    
    FT_Bitmap& bitmap = face->glyph->bitmap;
    int glyphWidth = static_cast<int>(bitmap.width);
    int glyphHeight = static_cast<int>(bitmap.rows);
    
    // Check if glyph fits in current row
    if (cursorX + glyphWidth + m_atlasPadding > m_atlasWidth) {
        // Move to next row
        cursorX = m_atlasPadding;
        cursorY += rowHeight + m_atlasPadding;
        rowHeight = 0;
    }
    
    // Check if glyph fits in atlas
    if (cursorY + glyphHeight + m_atlasPadding > m_atlasHeight) {
        std::cerr << "Font atlas full, cannot add glyph " << codepoint << std::endl;
        return false;
    }
    
    // Copy glyph bitmap to atlas
    for (int y = 0; y < glyphHeight; ++y) {
        for (int x = 0; x < glyphWidth; ++x) {
            int atlasIdx = (cursorY + y) * m_atlasWidth + (cursorX + x);
            int bitmapIdx = y * bitmap.pitch + x;
            m_atlasPixels[atlasIdx] = bitmap.buffer[bitmapIdx];
        }
    }
    
    // Create glyph entry
    Glyph glyph;
    glyph.codepoint = codepoint;
    glyph.width = static_cast<float>(glyphWidth);
    glyph.height = static_cast<float>(glyphHeight);
    glyph.bearingX = static_cast<float>(face->glyph->bitmap_left);
    glyph.bearingY = static_cast<float>(face->glyph->bitmap_top);
    glyph.advance = static_cast<float>(face->glyph->advance.x) / 64.0f;
    glyph.atlasX = cursorX;
    glyph.atlasY = cursorY;
    
    // Calculate normalized texture coordinates
    glyph.texU0 = static_cast<float>(cursorX) / m_atlasWidth;
    glyph.texV0 = static_cast<float>(cursorY) / m_atlasHeight;
    glyph.texU1 = static_cast<float>(cursorX + glyphWidth) / m_atlasWidth;
    glyph.texV1 = static_cast<float>(cursorY + glyphHeight) / m_atlasHeight;
    
    m_glyphs[codepoint] = glyph;
    
    // Update cursor position
    cursorX += glyphWidth + m_atlasPadding;
    if (glyphHeight > rowHeight) {
        rowHeight = glyphHeight;
    }
    
    m_atlasDirty = true;
    return true;
#else
    (void)codepoint;
    (void)cursorX;
    (void)cursorY;
    (void)rowHeight;
    return false;
#endif
}

void Font::updateAtlasTexture() {
    if (!m_atlasDirty && m_atlasTexture) {
        return;
    }
    
    // Convert grayscale to RGBA for texture
    std::vector<uint8_t> rgbaPixels(static_cast<size_t>(m_atlasWidth) * m_atlasHeight * 4);
    for (size_t i = 0; i < m_atlasPixels.size(); ++i) {
        size_t rgbaIdx = i * 4;
        rgbaPixels[rgbaIdx + 0] = 255;              // R
        rgbaPixels[rgbaIdx + 1] = 255;              // G
        rgbaPixels[rgbaIdx + 2] = 255;              // B
        rgbaPixels[rgbaIdx + 3] = m_atlasPixels[i]; // A (glyph coverage)
    }
    
    // Create texture
    TextureConfig texConfig;
    texConfig.minFilter = TextureFilter::Linear;
    texConfig.magFilter = TextureFilter::Linear;
    texConfig.wrapU = TextureWrap::ClampToEdge;
    texConfig.wrapV = TextureWrap::ClampToEdge;
    texConfig.generateMipmaps = false;
    
    m_atlasTexture = Texture::createFromPixels(rgbaPixels.data(), 
                                                m_atlasWidth, m_atlasHeight, 
                                                texConfig);
    m_atlasDirty = false;
}

const Glyph* Font::getGlyph(uint32_t codepoint) const {
    auto it = m_glyphs.find(codepoint);
    if (it != m_glyphs.end()) {
        return &it->second;
    }
    return nullptr;
}

float Font::getKerning(uint32_t left, uint32_t right) const {
#ifdef KGK_HAS_FREETYPE
    FT_Face face = static_cast<FT_Face>(m_ftFace);
    
    if (!FT_HAS_KERNING(face)) {
        return 0.0f;
    }
    
    FT_UInt leftIndex = FT_Get_Char_Index(face, left);
    FT_UInt rightIndex = FT_Get_Char_Index(face, right);
    
    FT_Vector kerning;
    FT_Error error = FT_Get_Kerning(face, leftIndex, rightIndex, 
                                     FT_KERNING_DEFAULT, &kerning);
    if (error) {
        return 0.0f;
    }
    
    return static_cast<float>(kerning.x) / 64.0f;
#else
    (void)left;
    (void)right;
    return 0.0f;
#endif
}

float Font::measureText(const std::string& text) const {
    return measureTextSize(text).width;
}

Size Font::measureTextSize(const std::string& text) const {
    float width = 0.0f;
    float maxHeight = 0.0f;
    uint32_t prevCodepoint = 0;
    
    // Simple UTF-8 decoding
    size_t i = 0;
    while (i < text.size()) {
        uint32_t codepoint = 0;
        uint8_t c = static_cast<uint8_t>(text[i]);
        
        if ((c & 0x80) == 0) {
            // ASCII
            codepoint = c;
            i += 1;
        } else if ((c & 0xE0) == 0xC0) {
            // 2-byte UTF-8
            codepoint = (c & 0x1F) << 6;
            if (i + 1 < text.size()) {
                codepoint |= (static_cast<uint8_t>(text[i + 1]) & 0x3F);
            }
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            // 3-byte UTF-8
            codepoint = (c & 0x0F) << 12;
            if (i + 1 < text.size()) {
                codepoint |= (static_cast<uint8_t>(text[i + 1]) & 0x3F) << 6;
            }
            if (i + 2 < text.size()) {
                codepoint |= (static_cast<uint8_t>(text[i + 2]) & 0x3F);
            }
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            // 4-byte UTF-8
            codepoint = (c & 0x07) << 18;
            if (i + 1 < text.size()) {
                codepoint |= (static_cast<uint8_t>(text[i + 1]) & 0x3F) << 12;
            }
            if (i + 2 < text.size()) {
                codepoint |= (static_cast<uint8_t>(text[i + 2]) & 0x3F) << 6;
            }
            if (i + 3 < text.size()) {
                codepoint |= (static_cast<uint8_t>(text[i + 3]) & 0x3F);
            }
            i += 4;
        } else {
            // Invalid UTF-8, skip
            i += 1;
            continue;
        }
        
        const Glyph* glyph = getGlyph(codepoint);
        if (glyph) {
            // Add kerning
            if (prevCodepoint != 0) {
                width += getKerning(prevCodepoint, codepoint);
            }
            
            width += glyph->advance;
            if (glyph->height > maxHeight) {
                maxHeight = glyph->height;
            }
        }
        
        prevCodepoint = codepoint;
    }
    
    return Size(width, maxHeight > 0 ? maxHeight : m_lineHeight);
}

} // namespace KillerGK
