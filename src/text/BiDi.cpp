/**
 * @file BiDi.cpp
 * @brief Bidirectional text algorithm implementation
 */

#include "KillerGK/text/BiDi.hpp"
#include <algorithm>

namespace KillerGK {

uint32_t BiDi::decodeUTF8(const std::string& text, size_t& index) {
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

void BiDi::encodeUTF8(uint32_t codepoint, std::string& output) {
    if (codepoint < 0x80) {
        output += static_cast<char>(codepoint);
    } else if (codepoint < 0x800) {
        output += static_cast<char>(0xC0 | (codepoint >> 6));
        output += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else if (codepoint < 0x10000) {
        output += static_cast<char>(0xE0 | (codepoint >> 12));
        output += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        output += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else {
        output += static_cast<char>(0xF0 | (codepoint >> 18));
        output += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
        output += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        output += static_cast<char>(0x80 | (codepoint & 0x3F));
    }
}

bool BiDi::isArabic(uint32_t codepoint) {
    // Arabic block: U+0600 - U+06FF
    // Arabic Supplement: U+0750 - U+077F
    // Arabic Extended-A: U+08A0 - U+08FF
    // Arabic Presentation Forms-A: U+FB50 - U+FDFF
    // Arabic Presentation Forms-B: U+FE70 - U+FEFF
    return (codepoint >= 0x0600 && codepoint <= 0x06FF) ||
           (codepoint >= 0x0750 && codepoint <= 0x077F) ||
           (codepoint >= 0x08A0 && codepoint <= 0x08FF) ||
           (codepoint >= 0xFB50 && codepoint <= 0xFDFF) ||
           (codepoint >= 0xFE70 && codepoint <= 0xFEFF);
}

bool BiDi::isHebrew(uint32_t codepoint) {
    // Hebrew block: U+0590 - U+05FF
    // Hebrew Presentation Forms: U+FB1D - U+FB4F
    return (codepoint >= 0x0590 && codepoint <= 0x05FF) ||
           (codepoint >= 0xFB1D && codepoint <= 0xFB4F);
}

bool BiDi::isRTL(uint32_t codepoint) {
    return isArabic(codepoint) || isHebrew(codepoint);
}

BiDiType BiDi::getType(uint32_t codepoint) {
    // Simplified BiDi type detection
    
    // Arabic letters
    if (isArabic(codepoint)) {
        return BiDiType::AL;
    }
    
    // Hebrew letters
    if (isHebrew(codepoint)) {
        return BiDiType::R;
    }
    
    // European numbers
    if (codepoint >= '0' && codepoint <= '9') {
        return BiDiType::EN;
    }
    
    // Arabic-Indic digits
    if (codepoint >= 0x0660 && codepoint <= 0x0669) {
        return BiDiType::AN;
    }
    
    // Extended Arabic-Indic digits
    if (codepoint >= 0x06F0 && codepoint <= 0x06F9) {
        return BiDiType::EN;  // These behave like European numbers
    }
    
    // Whitespace
    if (codepoint == ' ' || codepoint == '\t') {
        return BiDiType::WS;
    }
    
    // Newline
    if (codepoint == '\n' || codepoint == '\r') {
        return BiDiType::B;
    }
    
    // Common separators
    if (codepoint == ',' || codepoint == '.' || codepoint == ':') {
        return BiDiType::CS;
    }
    
    // European separators
    if (codepoint == '+' || codepoint == '-') {
        return BiDiType::ES;
    }
    
    // European terminators
    if (codepoint == '%' || codepoint == '$' || codepoint == '#') {
        return BiDiType::ET;
    }
    
    // Explicit formatting characters
    if (codepoint == 0x202A) return BiDiType::LRE;
    if (codepoint == 0x202B) return BiDiType::RLE;
    if (codepoint == 0x202C) return BiDiType::PDF;
    if (codepoint == 0x202D) return BiDiType::LRO;
    if (codepoint == 0x202E) return BiDiType::RLO;
    if (codepoint == 0x2066) return BiDiType::LRI;
    if (codepoint == 0x2067) return BiDiType::RLI;
    if (codepoint == 0x2068) return BiDiType::FSI;
    if (codepoint == 0x2069) return BiDiType::PDI;
    
    // Default: Left-to-right for Latin and most other scripts
    if ((codepoint >= 'A' && codepoint <= 'Z') ||
        (codepoint >= 'a' && codepoint <= 'z') ||
        (codepoint >= 0x00C0 && codepoint <= 0x024F)) {  // Latin Extended
        return BiDiType::L;
    }
    
    // Other neutrals
    return BiDiType::ON;
}

TextDirection BiDi::detectDirection(const std::string& text) {
    size_t i = 0;
    while (i < text.size()) {
        uint32_t codepoint = decodeUTF8(text, i);
        BiDiType type = getType(codepoint);
        
        // First strong character determines direction
        if (type == BiDiType::L) {
            return TextDirection::LTR;
        }
        if (type == BiDiType::R || type == BiDiType::AL) {
            return TextDirection::RTL;
        }
    }
    
    // Default to LTR if no strong characters found
    return TextDirection::LTR;
}

uint32_t BiDi::getMirror(uint32_t codepoint) {
    // Common mirrored pairs for RTL display
    switch (codepoint) {
        case '(': return ')';
        case ')': return '(';
        case '[': return ']';
        case ']': return '[';
        case '{': return '}';
        case '}': return '{';
        case '<': return '>';
        case '>': return '<';
        case 0x00AB: return 0x00BB;  // « »
        case 0x00BB: return 0x00AB;
        case 0x2039: return 0x203A;  // ‹ ›
        case 0x203A: return 0x2039;
        case 0x2018: return 0x2019;  // ' '
        case 0x2019: return 0x2018;
        case 0x201C: return 0x201D;  // " "
        case 0x201D: return 0x201C;
        default: return codepoint;
    }
}

std::vector<int> BiDi::computeLevels(const std::vector<BiDiType>& types,
                                      TextDirection baseDirection) {
    std::vector<int> levels(types.size());
    int baseLevel = (baseDirection == TextDirection::RTL) ? 1 : 0;
    
    // Simplified level computation
    for (size_t i = 0; i < types.size(); ++i) {
        switch (types[i]) {
            case BiDiType::L:
                levels[i] = baseLevel;
                if (baseLevel % 2 == 1) {
                    levels[i] = baseLevel + 1;  // Bump to even
                }
                break;
            case BiDiType::R:
            case BiDiType::AL:
                levels[i] = baseLevel;
                if (baseLevel % 2 == 0) {
                    levels[i] = baseLevel + 1;  // Bump to odd
                }
                break;
            case BiDiType::EN:
            case BiDiType::AN:
                levels[i] = baseLevel;
                if (baseLevel % 2 == 1) {
                    levels[i] = baseLevel + 1;
                }
                break;
            default:
                levels[i] = baseLevel;
                break;
        }
    }
    
    return levels;
}

BiDiResult BiDi::analyze(const std::string& text, TextDirection baseDirection) {
    BiDiResult result;
    
    // Decode UTF-8 to codepoints
    std::vector<uint32_t> codepoints;
    std::vector<BiDiType> types;
    
    size_t i = 0;
    while (i < text.size()) {
        uint32_t cp = decodeUTF8(text, i);
        if (cp == 0) break;
        codepoints.push_back(cp);
        types.push_back(getType(cp));
    }
    
    if (codepoints.empty()) {
        return result;
    }
    
    // Detect base direction if auto
    if (baseDirection == TextDirection::Auto) {
        baseDirection = detectDirection(text);
    }
    result.paragraphDirection = baseDirection;
    
    // Compute embedding levels
    std::vector<int> levels = computeLevels(types, baseDirection);
    
    // Create runs from levels
    BiDiRun currentRun;
    currentRun.start = 0;
    currentRun.level = levels[0];
    currentRun.direction = (levels[0] % 2 == 0) ? TextDirection::LTR : TextDirection::RTL;
    
    for (size_t j = 1; j < levels.size(); ++j) {
        if (levels[j] != currentRun.level) {
            currentRun.length = j - currentRun.start;
            result.runs.push_back(currentRun);
            
            currentRun.start = j;
            currentRun.level = levels[j];
            currentRun.direction = (levels[j] % 2 == 0) ? TextDirection::LTR : TextDirection::RTL;
        }
    }
    
    // Add final run
    currentRun.length = codepoints.size() - currentRun.start;
    result.runs.push_back(currentRun);
    
    // Compute visual order
    result.visualOrder.resize(codepoints.size());
    for (size_t j = 0; j < codepoints.size(); ++j) {
        result.visualOrder[j] = j;
    }
    
    // Reorder runs by level (simplified L2 rule)
    int maxLevel = 0;
    for (int level : levels) {
        maxLevel = std::max(maxLevel, level);
    }
    
    for (int level = maxLevel; level > 0; --level) {
        size_t runStart = 0;
        bool inRun = false;
        
        for (size_t j = 0; j <= codepoints.size(); ++j) {
            bool atLevel = (j < codepoints.size() && levels[j] >= level);
            
            if (atLevel && !inRun) {
                runStart = j;
                inRun = true;
            } else if (!atLevel && inRun) {
                // Reverse this run
                std::reverse(result.visualOrder.begin() + runStart,
                            result.visualOrder.begin() + j);
                inRun = false;
            }
        }
    }
    
    return result;
}

std::string BiDi::reorder(const std::string& text, TextDirection baseDirection) {
    BiDiResult result = analyze(text, baseDirection);
    
    if (result.visualOrder.empty()) {
        return text;
    }
    
    // Decode original text to codepoints
    std::vector<uint32_t> codepoints;
    size_t i = 0;
    while (i < text.size()) {
        uint32_t cp = decodeUTF8(text, i);
        if (cp == 0) break;
        codepoints.push_back(cp);
    }
    
    // Build reordered string
    std::string reordered;
    reordered.reserve(text.size());
    
    for (size_t idx : result.visualOrder) {
        if (idx < codepoints.size()) {
            uint32_t cp = codepoints[idx];
            
            // Apply mirroring for RTL characters at odd levels
            // Find the level for this character
            int level = 0;
            for (const auto& run : result.runs) {
                if (idx >= run.start && idx < run.start + run.length) {
                    level = run.level;
                    break;
                }
            }
            
            if (level % 2 == 1) {
                cp = getMirror(cp);
            }
            
            encodeUTF8(cp, reordered);
        }
    }
    
    return reordered;
}

void BiDi::resolveWeakTypes(std::vector<BiDiType>& types,
                            const std::vector<uint32_t>& codepoints) {
    // Simplified weak type resolution (W1-W7 rules)
    (void)codepoints;  // Not used in simplified version
    
    for (size_t i = 0; i < types.size(); ++i) {
        // W4: A single European separator between two European numbers
        if (types[i] == BiDiType::ES) {
            if (i > 0 && i + 1 < types.size() &&
                types[i - 1] == BiDiType::EN && types[i + 1] == BiDiType::EN) {
                types[i] = BiDiType::EN;
            }
        }
        
        // W5: European terminators adjacent to European numbers
        if (types[i] == BiDiType::ET) {
            if ((i > 0 && types[i - 1] == BiDiType::EN) ||
                (i + 1 < types.size() && types[i + 1] == BiDiType::EN)) {
                types[i] = BiDiType::EN;
            }
        }
        
        // W7: European numbers in RTL context become Arabic numbers
        // (simplified - not fully implemented)
    }
}

void BiDi::resolveNeutralTypes(std::vector<BiDiType>& types,
                                const std::vector<int>& levels) {
    // Simplified neutral type resolution (N1-N2 rules)
    for (size_t i = 0; i < types.size(); ++i) {
        if (types[i] == BiDiType::ON || types[i] == BiDiType::WS ||
            types[i] == BiDiType::CS || types[i] == BiDiType::ES ||
            types[i] == BiDiType::ET) {
            
            // Find surrounding strong types
            BiDiType prevStrong = BiDiType::L;
            BiDiType nextStrong = BiDiType::L;
            
            for (size_t j = i; j > 0; --j) {
                if (types[j - 1] == BiDiType::L || types[j - 1] == BiDiType::R ||
                    types[j - 1] == BiDiType::AL) {
                    prevStrong = types[j - 1];
                    break;
                }
            }
            
            for (size_t j = i + 1; j < types.size(); ++j) {
                if (types[j] == BiDiType::L || types[j] == BiDiType::R ||
                    types[j] == BiDiType::AL) {
                    nextStrong = types[j];
                    break;
                }
            }
            
            // N1: Neutrals between same direction become that direction
            if ((prevStrong == BiDiType::L && nextStrong == BiDiType::L) ||
                (prevStrong == BiDiType::R && nextStrong == BiDiType::R) ||
                (prevStrong == BiDiType::AL && nextStrong == BiDiType::AL)) {
                types[i] = prevStrong;
            } else {
                // N2: Use embedding direction
                types[i] = (levels[i] % 2 == 0) ? BiDiType::L : BiDiType::R;
            }
        }
    }
}

} // namespace KillerGK
