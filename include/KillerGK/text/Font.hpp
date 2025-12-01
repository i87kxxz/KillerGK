/**
 * @file Font.hpp
 * @brief Font loading and glyph management for KillerGK
 * 
 * Provides font loading from TTF/OTF files using FreeType,
 * glyph atlas generation, and font caching.
 */

#pragma once

#include "../core/Types.hpp"
#include "../rendering/Texture.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace KillerGK {

/**
 * @brief Glyph metrics and texture coordinates
 */
struct Glyph {
    uint32_t codepoint = 0;     ///< Unicode codepoint
    float width = 0.0f;         ///< Glyph width in pixels
    float height = 0.0f;        ///< Glyph height in pixels
    float bearingX = 0.0f;      ///< Horizontal bearing (offset from baseline)
    float bearingY = 0.0f;      ///< Vertical bearing (offset from baseline)
    float advance = 0.0f;       ///< Horizontal advance to next glyph
    
    // Texture atlas coordinates (normalized 0-1)
    float texU0 = 0.0f;         ///< Left texture coordinate
    float texV0 = 0.0f;         ///< Top texture coordinate
    float texU1 = 0.0f;         ///< Right texture coordinate
    float texV1 = 0.0f;         ///< Bottom texture coordinate
    
    int atlasX = 0;             ///< X position in atlas (pixels)
    int atlasY = 0;             ///< Y position in atlas (pixels)
};

/**
 * @brief Font style flags
 */
enum class FontStyle : uint32_t {
    Regular = 0,
    Bold = 1 << 0,
    Italic = 1 << 1,
    BoldItalic = Bold | Italic
};

/**
 * @brief Font configuration for loading
 */
struct FontConfig {
    float size = 16.0f;                 ///< Font size in pixels
    FontStyle style = FontStyle::Regular;
    bool antialiased = true;            ///< Enable anti-aliasing
    bool subpixel = true;               ///< Enable subpixel rendering
    int atlasWidth = 1024;              ///< Glyph atlas width
    int atlasHeight = 1024;             ///< Glyph atlas height
    int padding = 2;                    ///< Padding between glyphs in atlas
    
    // Character ranges to preload
    uint32_t rangeStart = 32;           ///< First character to load (space)
    uint32_t rangeEnd = 127;            ///< Last character to load (ASCII)
    bool loadExtendedLatin = true;      ///< Load Latin-1 Supplement (128-255)
    bool loadArabic = false;            ///< Load Arabic characters
    bool loadHebrew = false;            ///< Load Hebrew characters
};

/**
 * @brief Handle to a font
 */
using FontHandle = std::shared_ptr<class Font>;

/**
 * @class Font
 * @brief Represents a loaded font with glyph atlas
 */
class Font {
public:
    ~Font();
    
    /**
     * @brief Load font from file
     * @param path Path to TTF/OTF file
     * @param config Font configuration
     * @return Font handle or nullptr on failure
     */
    static FontHandle loadFromFile(const std::string& path, 
                                    const FontConfig& config = FontConfig{});
    
    /**
     * @brief Load font from memory
     * @param data Font file data
     * @param size Data size in bytes
     * @param config Font configuration
     * @return Font handle or nullptr on failure
     */
    static FontHandle loadFromMemory(const uint8_t* data, size_t size,
                                      const FontConfig& config = FontConfig{});
    
    /**
     * @brief Get glyph for a codepoint
     * @param codepoint Unicode codepoint
     * @return Pointer to glyph or nullptr if not found
     */
    [[nodiscard]] const Glyph* getGlyph(uint32_t codepoint) const;
    
    /**
     * @brief Load additional glyphs on demand
     * @param codepoint Unicode codepoint to load
     * @return true if glyph was loaded successfully
     */
    bool loadGlyph(uint32_t codepoint);
    
    /**
     * @brief Load a range of glyphs
     * @param start First codepoint
     * @param end Last codepoint (inclusive)
     * @return Number of glyphs successfully loaded
     */
    int loadGlyphRange(uint32_t start, uint32_t end);
    
    /**
     * @brief Get kerning between two glyphs
     * @param left Left codepoint
     * @param right Right codepoint
     * @return Kerning adjustment in pixels
     */
    [[nodiscard]] float getKerning(uint32_t left, uint32_t right) const;
    
    /**
     * @brief Measure text width
     * @param text UTF-8 encoded text
     * @return Width in pixels
     */
    [[nodiscard]] float measureText(const std::string& text) const;
    
    /**
     * @brief Measure text dimensions
     * @param text UTF-8 encoded text
     * @return Size in pixels
     */
    [[nodiscard]] Size measureTextSize(const std::string& text) const;
    
    // Getters
    [[nodiscard]] const std::string& getPath() const { return m_path; }
    [[nodiscard]] const std::string& getFamilyName() const { return m_familyName; }
    [[nodiscard]] const std::string& getStyleName() const { return m_styleName; }
    [[nodiscard]] float getSize() const { return m_size; }
    [[nodiscard]] float getLineHeight() const { return m_lineHeight; }
    [[nodiscard]] float getAscender() const { return m_ascender; }
    [[nodiscard]] float getDescender() const { return m_descender; }
    [[nodiscard]] TextureHandle getAtlasTexture() const { return m_atlasTexture; }
    [[nodiscard]] int getAtlasWidth() const { return m_atlasWidth; }
    [[nodiscard]] int getAtlasHeight() const { return m_atlasHeight; }
    [[nodiscard]] size_t getGlyphCount() const { return m_glyphs.size(); }
    
private:
    Font() = default;
    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;
    
    bool initialize(const FontConfig& config);
    bool createAtlas(const FontConfig& config);
    bool renderGlyphToAtlas(uint32_t codepoint, int& cursorX, int& cursorY, int rowHeight);
    void updateAtlasTexture();
    
    std::string m_path;
    std::string m_familyName;
    std::string m_styleName;
    float m_size = 0.0f;
    float m_lineHeight = 0.0f;
    float m_ascender = 0.0f;
    float m_descender = 0.0f;
    
    // Glyph storage
    std::unordered_map<uint32_t, Glyph> m_glyphs;
    
    // Kerning pairs (key = (left << 32) | right)
    std::unordered_map<uint64_t, float> m_kerning;
    
    // Atlas data
    int m_atlasWidth = 0;
    int m_atlasHeight = 0;
    int m_atlasPadding = 0;
    int m_atlasCursorX = 0;
    int m_atlasCursorY = 0;
    int m_atlasRowHeight = 0;
    std::vector<uint8_t> m_atlasPixels;
    TextureHandle m_atlasTexture;
    bool m_atlasDirty = false;
    
    // FreeType handles (opaque)
    void* m_ftFace = nullptr;
    std::vector<uint8_t> m_fontData;  // Keep font data in memory for FreeType
};

/**
 * @class FontManager
 * @brief Manages font loading and caching
 */
class FontManager {
public:
    static FontManager& instance();
    
    /**
     * @brief Initialize the font manager
     * @return true if successful
     */
    bool initialize();
    
    /**
     * @brief Shutdown and release all resources
     */
    void shutdown();
    
    /**
     * @brief Load a font (cached)
     * @param path Font file path
     * @param config Font configuration
     * @return Font handle or nullptr on failure
     */
    FontHandle loadFont(const std::string& path, const FontConfig& config = FontConfig{});
    
    /**
     * @brief Get a cached font
     * @param path Font file path
     * @param size Font size
     * @return Font handle or nullptr if not cached
     */
    FontHandle getFont(const std::string& path, float size) const;
    
    /**
     * @brief Get the default font
     * @return Default font handle
     */
    FontHandle getDefaultFont() const;
    
    /**
     * @brief Set the default font
     * @param font Font to use as default
     */
    void setDefaultFont(FontHandle font);
    
    /**
     * @brief Unload a font from cache
     * @param path Font file path
     */
    void unloadFont(const std::string& path);
    
    /**
     * @brief Clear all cached fonts
     */
    void clearCache();
    
    /**
     * @brief Check if FreeType is available
     * @return true if FreeType is initialized
     */
    [[nodiscard]] bool isAvailable() const { return m_initialized; }
    
    /**
     * @brief Get FreeType library handle (internal use)
     */
    void* getFTLibrary() const { return m_ftLibrary; }
    
private:
    FontManager() = default;
    ~FontManager();
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;
    
    bool m_initialized = false;
    void* m_ftLibrary = nullptr;
    
    // Cache key: path + "_" + size
    std::unordered_map<std::string, FontHandle> m_fontCache;
    FontHandle m_defaultFont;
};

} // namespace KillerGK
