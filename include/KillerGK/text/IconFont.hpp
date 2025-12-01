/**
 * @file IconFont.hpp
 * @brief Icon font and emoji support for KillerGK
 * 
 * Provides support for icon fonts (Font Awesome, Material Icons)
 * and color emoji rendering.
 */

#pragma once

#include "../core/Types.hpp"
#include "Font.hpp"
#include <string>
#include <unordered_map>
#include <memory>

namespace KillerGK {

/**
 * @brief Icon font types
 */
enum class IconFontType {
    FontAwesome,        ///< Font Awesome icons
    MaterialIcons,      ///< Material Design icons
    Custom              ///< Custom icon font
};

/**
 * @brief Icon definition
 */
struct IconDefinition {
    std::string name;       ///< Icon name (e.g., "home", "settings")
    uint32_t codepoint;     ///< Unicode codepoint in icon font
    std::string category;   ///< Icon category
};

/**
 * @class IconFont
 * @brief Manages icon fonts and provides icon lookup
 */
class IconFont {
public:
    /**
     * @brief Load an icon font
     * @param path Path to font file
     * @param type Icon font type
     * @param size Font size
     * @return true if loaded successfully
     */
    static bool loadIconFont(const std::string& path, IconFontType type, float size = 16.0f);
    
    /**
     * @brief Get icon font handle
     * @param type Icon font type
     * @return Font handle or nullptr
     */
    static FontHandle getIconFont(IconFontType type);
    
    /**
     * @brief Get icon codepoint by name
     * @param type Icon font type
     * @param name Icon name
     * @return Codepoint or 0 if not found
     */
    static uint32_t getIconCodepoint(IconFontType type, const std::string& name);
    
    /**
     * @brief Get icon as UTF-8 string
     * @param type Icon font type
     * @param name Icon name
     * @return UTF-8 string containing icon character
     */
    static std::string getIconString(IconFontType type, const std::string& name);
    
    /**
     * @brief Register custom icon
     * @param type Icon font type
     * @param name Icon name
     * @param codepoint Unicode codepoint
     */
    static void registerIcon(IconFontType type, const std::string& name, uint32_t codepoint);
    
    /**
     * @brief Get all icons for a font type
     * @param type Icon font type
     * @return Map of icon names to definitions
     */
    static const std::unordered_map<std::string, IconDefinition>& getIcons(IconFontType type);
    
    /**
     * @brief Search icons by name
     * @param type Icon font type
     * @param query Search query
     * @return Matching icon names
     */
    static std::vector<std::string> searchIcons(IconFontType type, const std::string& query);
    
private:
    static void initializeFontAwesome();
    static void initializeMaterialIcons();
    
    static std::unordered_map<IconFontType, FontHandle> s_iconFonts;
    static std::unordered_map<IconFontType, std::unordered_map<std::string, IconDefinition>> s_icons;
    static bool s_initialized;
};

// Font Awesome icon name constants
namespace FA {
    constexpr const char* Home = "home";
    constexpr const char* User = "user";
    constexpr const char* Settings = "cog";
    constexpr const char* Search = "search";
    constexpr const char* Plus = "plus";
    constexpr const char* Minus = "minus";
    constexpr const char* Check = "check";
    constexpr const char* Times = "times";
    constexpr const char* Edit = "edit";
    constexpr const char* Trash = "trash";
    constexpr const char* Save = "save";
    constexpr const char* File = "file";
    constexpr const char* Folder = "folder";
    constexpr const char* Download = "download";
    constexpr const char* Upload = "upload";
    constexpr const char* Play = "play";
    constexpr const char* Pause = "pause";
    constexpr const char* Stop = "stop";
    constexpr const char* Forward = "forward";
    constexpr const char* Backward = "backward";
    constexpr const char* VolumeUp = "volume-up";
    constexpr const char* VolumeDown = "volume-down";
    constexpr const char* VolumeMute = "volume-mute";
    constexpr const char* Heart = "heart";
    constexpr const char* Star = "star";
    constexpr const char* Bell = "bell";
    constexpr const char* Calendar = "calendar";
    constexpr const char* Clock = "clock";
    constexpr const char* Lock = "lock";
    constexpr const char* Unlock = "unlock";
    constexpr const char* Eye = "eye";
    constexpr const char* EyeSlash = "eye-slash";
    constexpr const char* Comment = "comment";
    constexpr const char* Envelope = "envelope";
    constexpr const char* Phone = "phone";
    constexpr const char* Camera = "camera";
    constexpr const char* Image = "image";
    constexpr const char* Video = "video";
    constexpr const char* Music = "music";
    constexpr const char* Link = "link";
    constexpr const char* Share = "share";
    constexpr const char* Print = "print";
    constexpr const char* Refresh = "sync";
    constexpr const char* Undo = "undo";
    constexpr const char* Redo = "redo";
    constexpr const char* Copy = "copy";
    constexpr const char* Cut = "cut";
    constexpr const char* Paste = "paste";
    constexpr const char* Bold = "bold";
    constexpr const char* Italic = "italic";
    constexpr const char* Underline = "underline";
    constexpr const char* AlignLeft = "align-left";
    constexpr const char* AlignCenter = "align-center";
    constexpr const char* AlignRight = "align-right";
    constexpr const char* AlignJustify = "align-justify";
    constexpr const char* List = "list";
    constexpr const char* ListOl = "list-ol";
    constexpr const char* ListUl = "list-ul";
    constexpr const char* Table = "table";
    constexpr const char* Code = "code";
    constexpr const char* Terminal = "terminal";
    constexpr const char* Bug = "bug";
    constexpr const char* Wrench = "wrench";
    constexpr const char* Hammer = "hammer";
    constexpr const char* Database = "database";
    constexpr const char* Server = "server";
    constexpr const char* Cloud = "cloud";
    constexpr const char* Wifi = "wifi";
    constexpr const char* Bluetooth = "bluetooth";
    constexpr const char* Battery = "battery-full";
    constexpr const char* Power = "power-off";
    constexpr const char* Desktop = "desktop";
    constexpr const char* Laptop = "laptop";
    constexpr const char* Mobile = "mobile";
    constexpr const char* Tablet = "tablet";
    constexpr const char* Keyboard = "keyboard";
    constexpr const char* Mouse = "mouse";
    constexpr const char* Gamepad = "gamepad";
    constexpr const char* Headphones = "headphones";
    constexpr const char* Microphone = "microphone";
    constexpr const char* Globe = "globe";
    constexpr const char* Map = "map";
    constexpr const char* MapMarker = "map-marker";
    constexpr const char* Compass = "compass";
    constexpr const char* Sun = "sun";
    constexpr const char* Moon = "moon";
    constexpr const char* CloudSun = "cloud-sun";
    constexpr const char* CloudRain = "cloud-rain";
    constexpr const char* Snowflake = "snowflake";
    constexpr const char* Fire = "fire";
    constexpr const char* Bolt = "bolt";
    constexpr const char* Shield = "shield-alt";
    constexpr const char* Key = "key";
    constexpr const char* Fingerprint = "fingerprint";
    constexpr const char* UserCircle = "user-circle";
    constexpr const char* Users = "users";
    constexpr const char* UserPlus = "user-plus";
    constexpr const char* UserMinus = "user-minus";
    constexpr const char* SignIn = "sign-in-alt";
    constexpr const char* SignOut = "sign-out-alt";
    constexpr const char* ChevronLeft = "chevron-left";
    constexpr const char* ChevronRight = "chevron-right";
    constexpr const char* ChevronUp = "chevron-up";
    constexpr const char* ChevronDown = "chevron-down";
    constexpr const char* ArrowLeft = "arrow-left";
    constexpr const char* ArrowRight = "arrow-right";
    constexpr const char* ArrowUp = "arrow-up";
    constexpr const char* ArrowDown = "arrow-down";
    constexpr const char* Expand = "expand";
    constexpr const char* Compress = "compress";
    constexpr const char* Maximize = "window-maximize";
    constexpr const char* Minimize = "window-minimize";
    constexpr const char* Close = "window-close";
    constexpr const char* Bars = "bars";
    constexpr const char* EllipsisH = "ellipsis-h";
    constexpr const char* EllipsisV = "ellipsis-v";
    constexpr const char* Info = "info-circle";
    constexpr const char* Question = "question-circle";
    constexpr const char* Exclamation = "exclamation-circle";
    constexpr const char* Warning = "exclamation-triangle";
    constexpr const char* CheckCircle = "check-circle";
    constexpr const char* TimesCircle = "times-circle";
    constexpr const char* PlusCircle = "plus-circle";
    constexpr const char* MinusCircle = "minus-circle";
}

/**
 * @class EmojiRenderer
 * @brief Handles color emoji rendering
 */
class EmojiRenderer {
public:
    /**
     * @brief Initialize emoji support
     * @param emojiAtlasPath Path to emoji atlas image
     * @param emojiDataPath Path to emoji data file
     * @return true if initialized successfully
     */
    static bool initialize(const std::string& emojiAtlasPath = "",
                           const std::string& emojiDataPath = "");
    
    /**
     * @brief Check if a codepoint is an emoji
     * @param codepoint Unicode codepoint
     * @return true if emoji
     */
    static bool isEmoji(uint32_t codepoint);
    
    /**
     * @brief Check if a codepoint is an emoji modifier
     * @param codepoint Unicode codepoint
     * @return true if modifier (skin tone, etc.)
     */
    static bool isEmojiModifier(uint32_t codepoint);
    
    /**
     * @brief Check if a codepoint is a ZWJ (Zero Width Joiner)
     * @param codepoint Unicode codepoint
     * @return true if ZWJ
     */
    static bool isZWJ(uint32_t codepoint);
    
    /**
     * @brief Get emoji name
     * @param codepoint Unicode codepoint
     * @return Emoji name or empty string
     */
    static std::string getEmojiName(uint32_t codepoint);
    
    /**
     * @brief Search emojis by name
     * @param query Search query
     * @return Matching emoji codepoints
     */
    static std::vector<uint32_t> searchEmojis(const std::string& query);
    
    /**
     * @brief Get emoji categories
     * @return List of category names
     */
    static std::vector<std::string> getCategories();
    
    /**
     * @brief Get emojis in category
     * @param category Category name
     * @return Emoji codepoints in category
     */
    static std::vector<uint32_t> getEmojisInCategory(const std::string& category);
    
    /**
     * @brief Render emoji
     * @param codepoint Emoji codepoint
     * @param x X position
     * @param y Y position
     * @param size Emoji size
     */
    static void renderEmoji(uint32_t codepoint, float x, float y, float size);
    
    /**
     * @brief Get emoji texture coordinates in atlas
     * @param codepoint Emoji codepoint
     * @param u0 Output: left texture coordinate
     * @param v0 Output: top texture coordinate
     * @param u1 Output: right texture coordinate
     * @param v1 Output: bottom texture coordinate
     * @return true if emoji found
     */
    static bool getEmojiTexCoords(uint32_t codepoint, 
                                   float& u0, float& v0, float& u1, float& v1);
    
private:
    static bool s_initialized;
    static std::unordered_map<uint32_t, std::string> s_emojiNames;
    static std::unordered_map<std::string, std::vector<uint32_t>> s_emojiCategories;
    static std::unordered_map<uint32_t, Rect> s_emojiAtlasCoords;
    static TextureHandle s_emojiAtlas;
};

} // namespace KillerGK
