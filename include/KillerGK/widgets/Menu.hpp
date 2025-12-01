/**
 * @file Menu.hpp
 * @brief Menu widgets for KillerGK - MenuBar, ContextMenu, CommandPalette
 */

#pragma once

#include "Widget.hpp"
#include "../core/Types.hpp"
#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace KillerGK {

/**
 * @enum MenuItemType
 * @brief Type of menu item
 */
enum class MenuItemType {
    Normal,     ///< Regular clickable item
    Checkbox,   ///< Checkable item
    Radio,      ///< Radio button item
    Separator,  ///< Visual separator
    Submenu     ///< Item with submenu
};

/**
 * @struct MenuItem
 * @brief Represents an item in a menu
 */
struct MenuItem {
    std::string id;                      ///< Unique item identifier
    std::string text;                    ///< Display text
    std::string icon;                    ///< Optional icon path
    std::string shortcut;                ///< Keyboard shortcut text (e.g., "Ctrl+S")
    MenuItemType type = MenuItemType::Normal;
    bool enabled = true;                 ///< Whether item is clickable
    bool checked = false;                ///< For checkbox/radio items
    std::string radioGroup;              ///< Radio group name for radio items
    std::vector<MenuItem> submenu;       ///< Submenu items
    std::function<void()> action;        ///< Action callback
    std::any userData;                   ///< Custom user data

    MenuItem() = default;
    MenuItem(const std::string& itemId, const std::string& itemText)
        : id(itemId), text(itemText) {}
    MenuItem(const std::string& itemId, const std::string& itemText, const std::string& itemShortcut)
        : id(itemId), text(itemText), shortcut(itemShortcut) {}

    /**
     * @brief Create a separator item
     * @return Separator MenuItem
     */
    static MenuItem separator() {
        MenuItem item;
        item.type = MenuItemType::Separator;
        return item;
    }

    /**
     * @brief Add submenu item
     * @param item Item to add to submenu
     * @return Reference to this item for chaining
     */
    MenuItem& addSubmenuItem(const MenuItem& item) {
        type = MenuItemType::Submenu;
        submenu.push_back(item);
        return *this;
    }
};

/**
 * @struct MenuSection
 * @brief A section in a menu (for MenuBar)
 */
struct MenuSection {
    std::string id;
    std::string title;
    std::vector<MenuItem> items;

    MenuSection() = default;
    MenuSection(const std::string& sectionId, const std::string& sectionTitle)
        : id(sectionId), title(sectionTitle) {}
};

/**
 * @class MenuBar
 * @brief Horizontal menu bar widget
 * 
 * Example:
 * @code
 * auto menuBar = MenuBar::create()
 *     .addSection({"file", "File", {
 *         {"new", "New", "Ctrl+N"},
 *         {"open", "Open", "Ctrl+O"},
 *         MenuItem::separator(),
 *         {"exit", "Exit"}
 *     }});
 * @endcode
 */
class MenuBar : public Widget {
public:
    virtual ~MenuBar() = default;
    static MenuBar create();

    /**
     * @brief Add a menu section
     * @param section Menu section to add
     * @return Reference to this MenuBar for chaining
     */
    MenuBar& addSection(const MenuSection& section);

    /**
     * @brief Remove section by id
     * @param id Section identifier
     * @return Reference to this MenuBar for chaining
     */
    MenuBar& removeSection(const std::string& id);

    /**
     * @brief Get all sections
     * @return Vector of sections
     */
    [[nodiscard]] const std::vector<MenuSection>& getSections() const;

    /**
     * @brief Get section by id
     * @param id Section identifier
     * @return Pointer to section or nullptr
     */
    [[nodiscard]] MenuSection* getSection(const std::string& id);

    /**
     * @brief Open menu section by id
     * @param id Section identifier
     * @return Reference to this MenuBar for chaining
     */
    MenuBar& openSection(const std::string& id);

    /**
     * @brief Close all open menus
     * @return Reference to this MenuBar for chaining
     */
    MenuBar& closeAll();

    /**
     * @brief Get currently open section id
     * @return Open section id or empty string
     */
    [[nodiscard]] const std::string& getOpenSectionId() const;

    // Appearance
    MenuBar& menuBarHeight(float height);
    [[nodiscard]] float getMenuBarHeight() const;

    MenuBar& menuBarColor(const Color& color);
    [[nodiscard]] const Color& getMenuBarColor() const;

    MenuBar& hoverColor(const Color& color);
    [[nodiscard]] const Color& getHoverColor() const;

    // Callbacks
    MenuBar& onItemClick(std::function<void(const MenuItem&)> callback);

protected:
    MenuBar();
    struct MenuBarData;
    std::shared_ptr<MenuBarData> m_menuBarData;
};


/**
 * @class ContextMenu
 * @brief Popup context menu widget
 * 
 * Example:
 * @code
 * auto contextMenu = ContextMenu::create()
 *     .items({
 *         {"cut", "Cut", "Ctrl+X"},
 *         {"copy", "Copy", "Ctrl+C"},
 *         {"paste", "Paste", "Ctrl+V"}
 *     });
 * contextMenu.show(mouseX, mouseY);
 * @endcode
 */
class ContextMenu : public Widget {
public:
    virtual ~ContextMenu() = default;
    static ContextMenu create();

    /**
     * @brief Set menu items
     * @param items Vector of menu items
     * @return Reference to this ContextMenu for chaining
     */
    ContextMenu& items(const std::vector<MenuItem>& items);

    /**
     * @brief Add a menu item
     * @param item Item to add
     * @return Reference to this ContextMenu for chaining
     */
    ContextMenu& addItem(const MenuItem& item);

    /**
     * @brief Add separator
     * @return Reference to this ContextMenu for chaining
     */
    ContextMenu& addSeparator();

    /**
     * @brief Remove item by id
     * @param id Item identifier
     * @return Reference to this ContextMenu for chaining
     */
    ContextMenu& removeItem(const std::string& id);

    /**
     * @brief Clear all items
     * @return Reference to this ContextMenu for chaining
     */
    ContextMenu& clearItems();

    /**
     * @brief Get all items
     * @return Vector of items
     */
    [[nodiscard]] const std::vector<MenuItem>& getItems() const;

    /**
     * @brief Show context menu at position
     * @param x X coordinate
     * @param y Y coordinate
     * @return Reference to this ContextMenu for chaining
     */
    ContextMenu& show(float x, float y);

    /**
     * @brief Hide context menu
     * @return Reference to this ContextMenu for chaining
     */
    ContextMenu& hide();

    /**
     * @brief Check if menu is visible
     * @return true if visible
     */
    [[nodiscard]] bool isMenuVisible() const;

    /**
     * @brief Get menu position
     * @param x Output X coordinate
     * @param y Output Y coordinate
     */
    void getPosition(float& x, float& y) const;

    // Appearance
    ContextMenu& menuColor(const Color& color);
    [[nodiscard]] const Color& getMenuColor() const;

    ContextMenu& hoverColor(const Color& color);
    [[nodiscard]] const Color& getHoverColor() const;

    ContextMenu& itemHeight(float height);
    [[nodiscard]] float getItemHeight() const;

    ContextMenu& minWidth(float width);
    [[nodiscard]] float getMinWidth() const;

    // Keyboard navigation
    void highlightNext();
    void highlightPrevious();
    void selectHighlighted();
    [[nodiscard]] int getHighlightedIndex() const;

    // Callbacks
    ContextMenu& onItemClick(std::function<void(const MenuItem&)> callback);
    ContextMenu& onClose(std::function<void()> callback);

protected:
    ContextMenu();
    struct ContextMenuData;
    std::shared_ptr<ContextMenuData> m_contextMenuData;
};

/**
 * @class CommandPalette
 * @brief Searchable command palette widget (like VS Code's Ctrl+Shift+P)
 * 
 * Example:
 * @code
 * auto palette = CommandPalette::create()
 *     .commands({
 *         {"file.new", "New File", "Ctrl+N"},
 *         {"file.open", "Open File", "Ctrl+O"},
 *         {"edit.find", "Find", "Ctrl+F"}
 *     })
 *     .placeholder("Type a command...")
 *     .onSelect([](const MenuItem& cmd) {
 *         std::cout << "Executing: " << cmd.text;
 *     });
 * @endcode
 */
class CommandPalette : public Widget {
public:
    virtual ~CommandPalette() = default;
    static CommandPalette create();

    /**
     * @brief Set available commands
     * @param commands Vector of command items
     * @return Reference to this CommandPalette for chaining
     */
    CommandPalette& commands(const std::vector<MenuItem>& commands);

    /**
     * @brief Add a command
     * @param command Command to add
     * @return Reference to this CommandPalette for chaining
     */
    CommandPalette& addCommand(const MenuItem& command);

    /**
     * @brief Remove command by id
     * @param id Command identifier
     * @return Reference to this CommandPalette for chaining
     */
    CommandPalette& removeCommand(const std::string& id);

    /**
     * @brief Get all commands
     * @return Vector of commands
     */
    [[nodiscard]] const std::vector<MenuItem>& getCommands() const;

    /**
     * @brief Get filtered commands based on search
     * @return Vector of matching commands
     */
    [[nodiscard]] std::vector<MenuItem> getFilteredCommands() const;

    /**
     * @brief Show command palette
     * @return Reference to this CommandPalette for chaining
     */
    CommandPalette& show();

    /**
     * @brief Hide command palette
     * @return Reference to this CommandPalette for chaining
     */
    CommandPalette& hide();

    /**
     * @brief Check if palette is visible
     * @return true if visible
     */
    [[nodiscard]] bool isPaletteVisible() const;

    /**
     * @brief Set search text
     * @param text Search text
     * @return Reference to this CommandPalette for chaining
     */
    CommandPalette& searchText(const std::string& text);

    /**
     * @brief Get current search text
     * @return Search text
     */
    [[nodiscard]] const std::string& getSearchText() const;

    /**
     * @brief Set placeholder text
     * @param text Placeholder text
     * @return Reference to this CommandPalette for chaining
     */
    CommandPalette& placeholder(const std::string& text);

    /**
     * @brief Get placeholder text
     * @return Placeholder text
     */
    [[nodiscard]] const std::string& getPlaceholder() const;

    /**
     * @brief Set maximum visible items
     * @param count Maximum items
     * @return Reference to this CommandPalette for chaining
     */
    CommandPalette& maxVisibleItems(int count);

    /**
     * @brief Get maximum visible items
     * @return Maximum items count
     */
    [[nodiscard]] int getMaxVisibleItems() const;

    // Appearance
    CommandPalette& paletteWidth(float width);
    [[nodiscard]] float getPaletteWidth() const;

    CommandPalette& paletteColor(const Color& color);
    [[nodiscard]] const Color& getPaletteColor() const;

    CommandPalette& hoverColor(const Color& color);
    [[nodiscard]] const Color& getHoverColor() const;

    CommandPalette& itemHeight(float height);
    [[nodiscard]] float getItemHeight() const;

    // Keyboard navigation
    void highlightNext();
    void highlightPrevious();
    void selectHighlighted();
    [[nodiscard]] int getHighlightedIndex() const;

    // Callbacks
    CommandPalette& onSelect(std::function<void(const MenuItem&)> callback);
    CommandPalette& onClose(std::function<void()> callback);
    CommandPalette& onSearchChange(std::function<void(const std::string&)> callback);

protected:
    CommandPalette();
    struct CommandPaletteData;
    std::shared_ptr<CommandPaletteData> m_paletteData;
};

} // namespace KillerGK
