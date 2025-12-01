/**
 * @file IconFont.cpp
 * @brief Icon font and emoji support implementation
 */

#include "KillerGK/text/IconFont.hpp"
#include "KillerGK/rendering/Renderer2D.hpp"
#include <algorithm>
#include <cctype>

namespace KillerGK {

// Static members
std::unordered_map<IconFontType, FontHandle> IconFont::s_iconFonts;
std::unordered_map<IconFontType, std::unordered_map<std::string, IconDefinition>> IconFont::s_icons;
bool IconFont::s_initialized = false;

bool EmojiRenderer::s_initialized = false;
std::unordered_map<uint32_t, std::string> EmojiRenderer::s_emojiNames;
std::unordered_map<std::string, std::vector<uint32_t>> EmojiRenderer::s_emojiCategories;
std::unordered_map<uint32_t, Rect> EmojiRenderer::s_emojiAtlasCoords;
TextureHandle EmojiRenderer::s_emojiAtlas;

// ============================================================================
// IconFont Implementation
// ============================================================================

void IconFont::initializeFontAwesome() {
    auto& icons = s_icons[IconFontType::FontAwesome];
    
    // Font Awesome 5 Free codepoints (subset)
    icons["home"] = {"home", 0xf015, "navigation"};
    icons["user"] = {"user", 0xf007, "users"};
    icons["cog"] = {"cog", 0xf013, "settings"};
    icons["search"] = {"search", 0xf002, "interface"};
    icons["plus"] = {"plus", 0xf067, "interface"};
    icons["minus"] = {"minus", 0xf068, "interface"};
    icons["check"] = {"check", 0xf00c, "interface"};
    icons["times"] = {"times", 0xf00d, "interface"};
    icons["edit"] = {"edit", 0xf044, "editing"};
    icons["trash"] = {"trash", 0xf1f8, "interface"};
    icons["save"] = {"save", 0xf0c7, "files"};
    icons["file"] = {"file", 0xf15b, "files"};
    icons["folder"] = {"folder", 0xf07b, "files"};
    icons["download"] = {"download", 0xf019, "interface"};
    icons["upload"] = {"upload", 0xf093, "interface"};
    icons["play"] = {"play", 0xf04b, "media"};
    icons["pause"] = {"pause", 0xf04c, "media"};
    icons["stop"] = {"stop", 0xf04d, "media"};
    icons["forward"] = {"forward", 0xf04e, "media"};
    icons["backward"] = {"backward", 0xf04a, "media"};
    icons["volume-up"] = {"volume-up", 0xf028, "media"};
    icons["volume-down"] = {"volume-down", 0xf027, "media"};
    icons["volume-mute"] = {"volume-mute", 0xf6a9, "media"};
    icons["heart"] = {"heart", 0xf004, "social"};
    icons["star"] = {"star", 0xf005, "rating"};
    icons["bell"] = {"bell", 0xf0f3, "notifications"};
    icons["calendar"] = {"calendar", 0xf133, "time"};
    icons["clock"] = {"clock", 0xf017, "time"};
    icons["lock"] = {"lock", 0xf023, "security"};
    icons["unlock"] = {"unlock", 0xf09c, "security"};
    icons["eye"] = {"eye", 0xf06e, "interface"};
    icons["eye-slash"] = {"eye-slash", 0xf070, "interface"};
    icons["comment"] = {"comment", 0xf075, "communication"};
    icons["envelope"] = {"envelope", 0xf0e0, "communication"};
    icons["phone"] = {"phone", 0xf095, "communication"};
    icons["camera"] = {"camera", 0xf030, "media"};
    icons["image"] = {"image", 0xf03e, "media"};
    icons["video"] = {"video", 0xf03d, "media"};
    icons["music"] = {"music", 0xf001, "media"};
    icons["link"] = {"link", 0xf0c1, "interface"};
    icons["share"] = {"share", 0xf064, "social"};
    icons["print"] = {"print", 0xf02f, "interface"};
    icons["sync"] = {"sync", 0xf021, "interface"};
    icons["undo"] = {"undo", 0xf0e2, "editing"};
    icons["redo"] = {"redo", 0xf01e, "editing"};
    icons["copy"] = {"copy", 0xf0c5, "editing"};
    icons["cut"] = {"cut", 0xf0c4, "editing"};
    icons["paste"] = {"paste", 0xf0ea, "editing"};
}

void IconFont::initializeMaterialIcons() {
    auto& icons = s_icons[IconFontType::MaterialIcons];
    
    // Material Icons codepoints (subset)
    icons["home"] = {"home", 0xe88a, "navigation"};
    icons["account_circle"] = {"account_circle", 0xe853, "users"};
    icons["settings"] = {"settings", 0xe8b8, "settings"};
    icons["search"] = {"search", 0xe8b6, "interface"};
    icons["add"] = {"add", 0xe145, "interface"};
    icons["remove"] = {"remove", 0xe15b, "interface"};
    icons["check"] = {"check", 0xe5ca, "interface"};
    icons["close"] = {"close", 0xe5cd, "interface"};
    icons["edit"] = {"edit", 0xe3c9, "editing"};
    icons["delete"] = {"delete", 0xe872, "interface"};
    icons["save"] = {"save", 0xe161, "files"};
    icons["insert_drive_file"] = {"insert_drive_file", 0xe24d, "files"};
    icons["folder"] = {"folder", 0xe2c7, "files"};
    icons["file_download"] = {"file_download", 0xe2c4, "interface"};
    icons["file_upload"] = {"file_upload", 0xe2c6, "interface"};
    icons["play_arrow"] = {"play_arrow", 0xe037, "media"};
    icons["pause"] = {"pause", 0xe034, "media"};
    icons["stop"] = {"stop", 0xe047, "media"};
    icons["skip_next"] = {"skip_next", 0xe044, "media"};
    icons["skip_previous"] = {"skip_previous", 0xe045, "media"};
    icons["volume_up"] = {"volume_up", 0xe050, "media"};
    icons["volume_down"] = {"volume_down", 0xe04d, "media"};
    icons["volume_off"] = {"volume_off", 0xe04f, "media"};
    icons["favorite"] = {"favorite", 0xe87d, "social"};
    icons["star"] = {"star", 0xe838, "rating"};
    icons["notifications"] = {"notifications", 0xe7f4, "notifications"};
    icons["event"] = {"event", 0xe878, "time"};
    icons["schedule"] = {"schedule", 0xe8b5, "time"};
    icons["lock"] = {"lock", 0xe897, "security"};
    icons["lock_open"] = {"lock_open", 0xe898, "security"};
    icons["visibility"] = {"visibility", 0xe8f4, "interface"};
    icons["visibility_off"] = {"visibility_off", 0xe8f5, "interface"};
    icons["chat"] = {"chat", 0xe0b7, "communication"};
    icons["email"] = {"email", 0xe0be, "communication"};
    icons["call"] = {"call", 0xe0b0, "communication"};
    icons["photo_camera"] = {"photo_camera", 0xe412, "media"};
    icons["photo"] = {"photo", 0xe410, "media"};
    icons["videocam"] = {"videocam", 0xe04b, "media"};
    icons["music_note"] = {"music_note", 0xe405, "media"};
    icons["link"] = {"link", 0xe157, "interface"};
    icons["share"] = {"share", 0xe80d, "social"};
    icons["print"] = {"print", 0xe8ad, "interface"};
    icons["refresh"] = {"refresh", 0xe5d5, "interface"};
    icons["undo"] = {"undo", 0xe166, "editing"};
    icons["redo"] = {"redo", 0xe15a, "editing"};
    icons["content_copy"] = {"content_copy", 0xe14d, "editing"};
    icons["content_cut"] = {"content_cut", 0xe14e, "editing"};
    icons["content_paste"] = {"content_paste", 0xe14f, "editing"};
    icons["format_bold"] = {"format_bold", 0xe238, "formatting"};
    icons["format_italic"] = {"format_italic", 0xe23f, "formatting"};
    icons["format_underlined"] = {"format_underlined", 0xe249, "formatting"};
    icons["format_align_left"] = {"format_align_left", 0xe236, "formatting"};
    icons["format_align_center"] = {"format_align_center", 0xe234, "formatting"};
    icons["format_align_right"] = {"format_align_right", 0xe237, "formatting"};
    icons["format_list_bulleted"] = {"format_list_bulleted", 0xe241, "formatting"};
    icons["format_list_numbered"] = {"format_list_numbered", 0xe242, "formatting"};
    icons["code"] = {"code", 0xe86f, "development"};
    icons["terminal"] = {"terminal", 0xeb8e, "development"};
    icons["bug_report"] = {"bug_report", 0xe868, "development"};
    icons["build"] = {"build", 0xe869, "development"};
    icons["storage"] = {"storage", 0xe1db, "data"};
    icons["dns"] = {"dns", 0xe875, "data"};
    icons["cloud"] = {"cloud", 0xe2bd, "data"};
    icons["wifi"] = {"wifi", 0xe63e, "connectivity"};
    icons["bluetooth"] = {"bluetooth", 0xe1a7, "connectivity"};
    icons["battery_full"] = {"battery_full", 0xe1a4, "device"};
    icons["power_settings_new"] = {"power_settings_new", 0xe8ac, "device"};
    icons["computer"] = {"computer", 0xe30a, "device"};
    icons["laptop"] = {"laptop", 0xe31e, "device"};
    icons["smartphone"] = {"smartphone", 0xe32c, "device"};
    icons["tablet"] = {"tablet", 0xe32f, "device"};
    icons["keyboard"] = {"keyboard", 0xe312, "device"};
    icons["mouse"] = {"mouse", 0xe323, "device"};
    icons["headset"] = {"headset", 0xe310, "device"};
    icons["mic"] = {"mic", 0xe029, "device"};
    icons["language"] = {"language", 0xe894, "navigation"};
    icons["map"] = {"map", 0xe55b, "navigation"};
    icons["place"] = {"place", 0xe55f, "navigation"};
    icons["explore"] = {"explore", 0xe87a, "navigation"};
    icons["wb_sunny"] = {"wb_sunny", 0xe430, "weather"};
    icons["nights_stay"] = {"nights_stay", 0xea46, "weather"};
    icons["cloud_queue"] = {"cloud_queue", 0xe2c2, "weather"};
    icons["info"] = {"info", 0xe88e, "interface"};
    icons["help"] = {"help", 0xe887, "interface"};
    icons["warning"] = {"warning", 0xe002, "interface"};
    icons["error"] = {"error", 0xe000, "interface"};
    icons["check_circle"] = {"check_circle", 0xe86c, "interface"};
    icons["cancel"] = {"cancel", 0xe5c9, "interface"};
    icons["add_circle"] = {"add_circle", 0xe147, "interface"};
    icons["remove_circle"] = {"remove_circle", 0xe15c, "interface"};
    icons["chevron_left"] = {"chevron_left", 0xe5cb, "navigation"};
    icons["chevron_right"] = {"chevron_right", 0xe5cc, "navigation"};
    icons["expand_more"] = {"expand_more", 0xe5cf, "navigation"};
    icons["expand_less"] = {"expand_less", 0xe5ce, "navigation"};
    icons["arrow_back"] = {"arrow_back", 0xe5c4, "navigation"};
    icons["arrow_forward"] = {"arrow_forward", 0xe5c8, "navigation"};
    icons["arrow_upward"] = {"arrow_upward", 0xe5d8, "navigation"};
    icons["arrow_downward"] = {"arrow_downward", 0xe5db, "navigation"};
    icons["fullscreen"] = {"fullscreen", 0xe5d0, "interface"};
    icons["fullscreen_exit"] = {"fullscreen_exit", 0xe5d1, "interface"};
    icons["menu"] = {"menu", 0xe5d2, "navigation"};
    icons["more_horiz"] = {"more_horiz", 0xe5d3, "interface"};
    icons["more_vert"] = {"more_vert", 0xe5d4, "interface"};
}

bool IconFont::loadIconFont(const std::string& path, IconFontType type, float size) {
    if (!s_initialized) {
        initializeFontAwesome();
        initializeMaterialIcons();
        s_initialized = true;
    }
    
    FontConfig config;
    config.size = size;
    config.rangeStart = 0xE000;  // Private Use Area start
    config.rangeEnd = 0xF8FF;    // Private Use Area end
    config.loadExtendedLatin = false;
    
    FontHandle font = Font::loadFromFile(path, config);
    if (font) {
        s_iconFonts[type] = font;
        return true;
    }
    return false;
}

FontHandle IconFont::getIconFont(IconFontType type) {
    auto it = s_iconFonts.find(type);
    return (it != s_iconFonts.end()) ? it->second : nullptr;
}

uint32_t IconFont::getIconCodepoint(IconFontType type, const std::string& name) {
    if (!s_initialized) {
        initializeFontAwesome();
        initializeMaterialIcons();
        s_initialized = true;
    }
    
    auto typeIt = s_icons.find(type);
    if (typeIt == s_icons.end()) return 0;
    
    auto iconIt = typeIt->second.find(name);
    return (iconIt != typeIt->second.end()) ? iconIt->second.codepoint : 0;
}

std::string IconFont::getIconString(IconFontType type, const std::string& name) {
    uint32_t codepoint = getIconCodepoint(type, name);
    if (codepoint == 0) return "";
    
    std::string result;
    if (codepoint < 0x80) {
        result += static_cast<char>(codepoint);
    } else if (codepoint < 0x800) {
        result += static_cast<char>(0xC0 | (codepoint >> 6));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else if (codepoint < 0x10000) {
        result += static_cast<char>(0xE0 | (codepoint >> 12));
        result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else {
        result += static_cast<char>(0xF0 | (codepoint >> 18));
        result += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
        result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    }
    return result;
}

void IconFont::registerIcon(IconFontType type, const std::string& name, uint32_t codepoint) {
    s_icons[type][name] = {name, codepoint, "custom"};
}

const std::unordered_map<std::string, IconDefinition>& IconFont::getIcons(IconFontType type) {
    if (!s_initialized) {
        initializeFontAwesome();
        initializeMaterialIcons();
        s_initialized = true;
    }
    
    static std::unordered_map<std::string, IconDefinition> empty;
    auto it = s_icons.find(type);
    return (it != s_icons.end()) ? it->second : empty;
}

std::vector<std::string> IconFont::searchIcons(IconFontType type, const std::string& query) {
    std::vector<std::string> results;
    
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    const auto& icons = getIcons(type);
    for (const auto& [name, def] : icons) {
        std::string lowerName = name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        
        if (lowerName.find(lowerQuery) != std::string::npos) {
            results.push_back(name);
        }
    }
    
    return results;
}

// ============================================================================
// EmojiRenderer Implementation
// ============================================================================

bool EmojiRenderer::initialize(const std::string& emojiAtlasPath,
                                const std::string& emojiDataPath) {
    if (s_initialized) return true;
    
    // Initialize emoji categories with common emojis
    s_emojiCategories["smileys"] = {
        0x1F600, 0x1F601, 0x1F602, 0x1F603, 0x1F604, 0x1F605, 0x1F606, 0x1F607,
        0x1F608, 0x1F609, 0x1F60A, 0x1F60B, 0x1F60C, 0x1F60D, 0x1F60E, 0x1F60F,
        0x1F610, 0x1F611, 0x1F612, 0x1F613, 0x1F614, 0x1F615, 0x1F616, 0x1F617,
        0x1F618, 0x1F619, 0x1F61A, 0x1F61B, 0x1F61C, 0x1F61D, 0x1F61E, 0x1F61F,
        0x1F620, 0x1F621, 0x1F622, 0x1F623, 0x1F624, 0x1F625, 0x1F626, 0x1F627,
        0x1F628, 0x1F629, 0x1F62A, 0x1F62B, 0x1F62C, 0x1F62D, 0x1F62E, 0x1F62F,
        0x1F630, 0x1F631, 0x1F632, 0x1F633, 0x1F634, 0x1F635, 0x1F636, 0x1F637
    };
    
    s_emojiCategories["people"] = {
        0x1F466, 0x1F467, 0x1F468, 0x1F469, 0x1F46A, 0x1F46B, 0x1F46C, 0x1F46D,
        0x1F46E, 0x1F46F, 0x1F470, 0x1F471, 0x1F472, 0x1F473, 0x1F474, 0x1F475,
        0x1F476, 0x1F477, 0x1F478, 0x1F479, 0x1F47A, 0x1F47B, 0x1F47C, 0x1F47D,
        0x1F47E, 0x1F47F, 0x1F480, 0x1F481, 0x1F482, 0x1F483
    };
    
    s_emojiCategories["animals"] = {
        0x1F400, 0x1F401, 0x1F402, 0x1F403, 0x1F404, 0x1F405, 0x1F406, 0x1F407,
        0x1F408, 0x1F409, 0x1F40A, 0x1F40B, 0x1F40C, 0x1F40D, 0x1F40E, 0x1F40F,
        0x1F410, 0x1F411, 0x1F412, 0x1F413, 0x1F414, 0x1F415, 0x1F416, 0x1F417,
        0x1F418, 0x1F419, 0x1F41A, 0x1F41B, 0x1F41C, 0x1F41D, 0x1F41E, 0x1F41F,
        0x1F420, 0x1F421, 0x1F422, 0x1F423, 0x1F424, 0x1F425, 0x1F426, 0x1F427,
        0x1F428, 0x1F429, 0x1F42A, 0x1F42B, 0x1F42C, 0x1F42D, 0x1F42E, 0x1F42F,
        0x1F430, 0x1F431, 0x1F432, 0x1F433, 0x1F434, 0x1F435, 0x1F436, 0x1F437,
        0x1F438, 0x1F439, 0x1F43A, 0x1F43B, 0x1F43C, 0x1F43D, 0x1F43E
    };
    
    s_emojiCategories["food"] = {
        0x1F34A, 0x1F34B, 0x1F34C, 0x1F34D, 0x1F34E, 0x1F34F, 0x1F350, 0x1F351,
        0x1F352, 0x1F353, 0x1F354, 0x1F355, 0x1F356, 0x1F357, 0x1F358, 0x1F359,
        0x1F35A, 0x1F35B, 0x1F35C, 0x1F35D, 0x1F35E, 0x1F35F, 0x1F360, 0x1F361,
        0x1F362, 0x1F363, 0x1F364, 0x1F365, 0x1F366, 0x1F367, 0x1F368, 0x1F369,
        0x1F36A, 0x1F36B, 0x1F36C, 0x1F36D, 0x1F36E, 0x1F36F, 0x1F370
    };
    
    s_emojiCategories["objects"] = {
        0x1F4A1, 0x1F4A2, 0x1F4A3, 0x1F4A4, 0x1F4A5, 0x1F4A6, 0x1F4A7, 0x1F4A8,
        0x1F4A9, 0x1F4AA, 0x1F4AB, 0x1F4AC, 0x1F4AD, 0x1F4AE, 0x1F4AF, 0x1F4B0,
        0x1F4B1, 0x1F4B2, 0x1F4B3, 0x1F4B4, 0x1F4B5, 0x1F4B6, 0x1F4B7, 0x1F4B8,
        0x1F4B9, 0x1F4BA, 0x1F4BB, 0x1F4BC, 0x1F4BD, 0x1F4BE, 0x1F4BF, 0x1F4C0
    };
    
    s_emojiCategories["symbols"] = {
        0x2764, 0x2763, 0x2665, 0x2666, 0x2660, 0x2663, 0x2716, 0x2714,
        0x2611, 0x2610, 0x2B50, 0x2B55, 0x2705, 0x2757, 0x2753, 0x2754,
        0x2755, 0x2795, 0x2796, 0x2797, 0x27A1, 0x2B05, 0x2B06, 0x2B07
    };
    
    // Initialize emoji names
    s_emojiNames[0x1F600] = "grinning face";
    s_emojiNames[0x1F601] = "beaming face with smiling eyes";
    s_emojiNames[0x1F602] = "face with tears of joy";
    s_emojiNames[0x1F603] = "grinning face with big eyes";
    s_emojiNames[0x1F604] = "grinning face with smiling eyes";
    s_emojiNames[0x1F605] = "grinning face with sweat";
    s_emojiNames[0x1F606] = "grinning squinting face";
    s_emojiNames[0x1F607] = "smiling face with halo";
    s_emojiNames[0x1F608] = "smiling face with horns";
    s_emojiNames[0x1F609] = "winking face";
    s_emojiNames[0x1F60A] = "smiling face with smiling eyes";
    s_emojiNames[0x1F60B] = "face savoring food";
    s_emojiNames[0x1F60C] = "relieved face";
    s_emojiNames[0x1F60D] = "smiling face with heart-eyes";
    s_emojiNames[0x1F60E] = "smiling face with sunglasses";
    s_emojiNames[0x1F60F] = "smirking face";
    s_emojiNames[0x2764] = "red heart";
    s_emojiNames[0x2B50] = "star";
    s_emojiNames[0x1F44D] = "thumbs up";
    s_emojiNames[0x1F44E] = "thumbs down";
    s_emojiNames[0x1F44B] = "waving hand";
    s_emojiNames[0x1F44F] = "clapping hands";
    
    // Load emoji atlas if provided
    if (!emojiAtlasPath.empty()) {
        s_emojiAtlas = Texture::loadFromFile(emojiAtlasPath);
    }
    
    (void)emojiDataPath;  // Would load additional emoji data
    
    s_initialized = true;
    return true;
}

bool EmojiRenderer::isEmoji(uint32_t codepoint) {
    // Common emoji ranges
    return (codepoint >= 0x1F300 && codepoint <= 0x1F9FF) ||  // Misc Symbols and Pictographs
           (codepoint >= 0x2600 && codepoint <= 0x26FF) ||    // Misc Symbols
           (codepoint >= 0x2700 && codepoint <= 0x27BF) ||    // Dingbats
           (codepoint >= 0x1F600 && codepoint <= 0x1F64F) ||  // Emoticons
           (codepoint >= 0x1F680 && codepoint <= 0x1F6FF) ||  // Transport and Map
           (codepoint >= 0x1F1E0 && codepoint <= 0x1F1FF);    // Flags
}

bool EmojiRenderer::isEmojiModifier(uint32_t codepoint) {
    // Skin tone modifiers
    return (codepoint >= 0x1F3FB && codepoint <= 0x1F3FF);
}

bool EmojiRenderer::isZWJ(uint32_t codepoint) {
    return codepoint == 0x200D;
}

std::string EmojiRenderer::getEmojiName(uint32_t codepoint) {
    auto it = s_emojiNames.find(codepoint);
    return (it != s_emojiNames.end()) ? it->second : "";
}

std::vector<uint32_t> EmojiRenderer::searchEmojis(const std::string& query) {
    std::vector<uint32_t> results;
    
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    for (const auto& [codepoint, name] : s_emojiNames) {
        std::string lowerName = name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        
        if (lowerName.find(lowerQuery) != std::string::npos) {
            results.push_back(codepoint);
        }
    }
    
    return results;
}

std::vector<std::string> EmojiRenderer::getCategories() {
    std::vector<std::string> categories;
    for (const auto& [name, emojis] : s_emojiCategories) {
        categories.push_back(name);
    }
    return categories;
}

std::vector<uint32_t> EmojiRenderer::getEmojisInCategory(const std::string& category) {
    auto it = s_emojiCategories.find(category);
    return (it != s_emojiCategories.end()) ? it->second : std::vector<uint32_t>{};
}

void EmojiRenderer::renderEmoji(uint32_t codepoint, float x, float y, float size) {
    if (!s_emojiAtlas) return;
    
    float u0, v0, u1, v1;
    if (getEmojiTexCoords(codepoint, u0, v0, u1, v1)) {
        Rect dstRect(x, y, size, size);
        Rect srcRect(u0 * s_emojiAtlas->getWidth(), v0 * s_emojiAtlas->getHeight(),
                     (u1 - u0) * s_emojiAtlas->getWidth(), (v1 - v0) * s_emojiAtlas->getHeight());
        Renderer2D::instance().drawTexturedRect(dstRect, s_emojiAtlas, srcRect);
    }
}

bool EmojiRenderer::getEmojiTexCoords(uint32_t codepoint,
                                       float& u0, float& v0, float& u1, float& v1) {
    auto it = s_emojiAtlasCoords.find(codepoint);
    if (it == s_emojiAtlasCoords.end()) {
        return false;
    }
    
    const Rect& coords = it->second;
    u0 = coords.x;
    v0 = coords.y;
    u1 = coords.x + coords.width;
    v1 = coords.y + coords.height;
    return true;
}

} // namespace KillerGK
