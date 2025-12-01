/**
 * @file BiDi.hpp
 * @brief Bidirectional text algorithm for RTL language support
 * 
 * Implements the Unicode Bidirectional Algorithm (UBA) for proper
 * rendering of Arabic, Hebrew, and mixed LTR/RTL text.
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace KillerGK {

/**
 * @brief Text direction
 */
enum class TextDirection {
    LTR,    ///< Left-to-right
    RTL,    ///< Right-to-left
    Auto    ///< Detect from content
};

/**
 * @brief Bidirectional character type (simplified)
 */
enum class BiDiType {
    L,      ///< Left-to-right (Latin, etc.)
    R,      ///< Right-to-left (Hebrew)
    AL,     ///< Arabic letter
    EN,     ///< European number
    AN,     ///< Arabic number
    ES,     ///< European separator
    ET,     ///< European terminator
    CS,     ///< Common separator
    NSM,    ///< Non-spacing mark
    BN,     ///< Boundary neutral
    B,      ///< Paragraph separator
    S,      ///< Segment separator
    WS,     ///< Whitespace
    ON,     ///< Other neutral
    LRE,    ///< Left-to-right embedding
    LRO,    ///< Left-to-right override
    RLE,    ///< Right-to-left embedding
    RLO,    ///< Right-to-left override
    PDF,    ///< Pop directional format
    LRI,    ///< Left-to-right isolate
    RLI,    ///< Right-to-left isolate
    FSI,    ///< First strong isolate
    PDI     ///< Pop directional isolate
};

/**
 * @brief A run of text with consistent direction
 */
struct BiDiRun {
    size_t start = 0;       ///< Start index in original text
    size_t length = 0;      ///< Length in codepoints
    int level = 0;          ///< Embedding level (even = LTR, odd = RTL)
    TextDirection direction = TextDirection::LTR;
};

/**
 * @brief Result of bidirectional analysis
 */
struct BiDiResult {
    std::vector<BiDiRun> runs;
    TextDirection paragraphDirection = TextDirection::LTR;
    std::vector<size_t> visualOrder;  ///< Visual order of codepoints
};

/**
 * @class BiDi
 * @brief Bidirectional text processing
 */
class BiDi {
public:
    /**
     * @brief Analyze text for bidirectional rendering
     * @param text UTF-8 encoded text
     * @param baseDirection Base paragraph direction (Auto to detect)
     * @return BiDi analysis result
     */
    static BiDiResult analyze(const std::string& text, 
                               TextDirection baseDirection = TextDirection::Auto);
    
    /**
     * @brief Reorder text for visual display
     * @param text UTF-8 encoded text
     * @param baseDirection Base paragraph direction
     * @return Reordered text for visual display
     */
    static std::string reorder(const std::string& text,
                                TextDirection baseDirection = TextDirection::Auto);
    
    /**
     * @brief Get the bidirectional type of a codepoint
     * @param codepoint Unicode codepoint
     * @return BiDi type
     */
    static BiDiType getType(uint32_t codepoint);
    
    /**
     * @brief Check if codepoint is RTL
     * @param codepoint Unicode codepoint
     * @return true if RTL character
     */
    static bool isRTL(uint32_t codepoint);
    
    /**
     * @brief Check if codepoint is Arabic
     * @param codepoint Unicode codepoint
     * @return true if Arabic character
     */
    static bool isArabic(uint32_t codepoint);
    
    /**
     * @brief Check if codepoint is Hebrew
     * @param codepoint Unicode codepoint
     * @return true if Hebrew character
     */
    static bool isHebrew(uint32_t codepoint);
    
    /**
     * @brief Detect paragraph direction from text
     * @param text UTF-8 encoded text
     * @return Detected direction (LTR if no strong characters found)
     */
    static TextDirection detectDirection(const std::string& text);
    
    /**
     * @brief Mirror a character for RTL display
     * @param codepoint Unicode codepoint
     * @return Mirrored codepoint (or original if no mirror)
     */
    static uint32_t getMirror(uint32_t codepoint);
    
private:
    // UTF-8 decoding helper
    static uint32_t decodeUTF8(const std::string& text, size_t& index);
    
    // UTF-8 encoding helper
    static void encodeUTF8(uint32_t codepoint, std::string& output);
    
    // Resolve weak types
    static void resolveWeakTypes(std::vector<BiDiType>& types, 
                                  const std::vector<uint32_t>& codepoints);
    
    // Resolve neutral types
    static void resolveNeutralTypes(std::vector<BiDiType>& types,
                                     const std::vector<int>& levels);
    
    // Compute embedding levels
    static std::vector<int> computeLevels(const std::vector<BiDiType>& types,
                                           TextDirection baseDirection);
};

} // namespace KillerGK
