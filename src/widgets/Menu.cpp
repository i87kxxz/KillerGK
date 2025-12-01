/**
 * @file Menu.cpp
 * @brief Menu widgets implementation - MenuBar, ContextMenu, CommandPalette
 */

#include "KillerGK/widgets/Menu.hpp"
#include <algorithm>
#include <cctype>

namespace KillerGK {

// =============================================================================
// MenuBar Implementation
// =============================================================================

struct MenuBar::MenuBarData {
    std::vector<MenuSection> sections;
    std::string openSectionId;
    float menuBarHeight = 28.0f;
    Color menuBarColor = Color(0.95f, 0.95f, 0.95f, 1.0f);
    Color hoverColor = Color(0.9f, 0.9f, 0.9f, 1.0f);
    std::function<void(const MenuItem&)> onItemClickCallback;
};

MenuBar::MenuBar()
    : Widget()
    , m_menuBarData(std::make_shared<MenuBarData>())
{
    height(28.0f);
    backgroundColor(Color(0.95f, 0.95f, 0.95f, 1.0f));
}

MenuBar MenuBar::create() {
    return MenuBar();
}

MenuBar& MenuBar::addSection(const MenuSection& section) {
    m_menuBarData->sections.push_back(section);
    return *this;
}

MenuBar& MenuBar::removeSection(const std::string& id) {
    auto it = std::remove_if(m_menuBarData->sections.begin(), m_menuBarData->sections.end(),
        [&id](const MenuSection& s) { return s.id == id; });
    m_menuBarData->sections.erase(it, m_menuBarData->sections.end());
    return *this;
}

const std::vector<MenuSection>& MenuBar::getSections() const {
    return m_menuBarData->sections;
}

MenuSection* MenuBar::getSection(const std::string& id) {
    for (auto& section : m_menuBarData->sections) {
        if (section.id == id) return &section;
    }
    return nullptr;
}

MenuBar& MenuBar::openSection(const std::string& id) {
    m_menuBarData->openSectionId = id;
    return *this;
}

MenuBar& MenuBar::closeAll() {
    m_menuBarData->openSectionId.clear();
    return *this;
}

const std::string& MenuBar::getOpenSectionId() const {
    return m_menuBarData->openSectionId;
}

MenuBar& MenuBar::menuBarHeight(float height) {
    m_menuBarData->menuBarHeight = height;
    this->height(height);
    return *this;
}

float MenuBar::getMenuBarHeight() const {
    return m_menuBarData->menuBarHeight;
}

MenuBar& MenuBar::menuBarColor(const Color& color) {
    m_menuBarData->menuBarColor = color;
    backgroundColor(color);
    return *this;
}

const Color& MenuBar::getMenuBarColor() const {
    return m_menuBarData->menuBarColor;
}

MenuBar& MenuBar::hoverColor(const Color& color) {
    m_menuBarData->hoverColor = color;
    return *this;
}

const Color& MenuBar::getHoverColor() const {
    return m_menuBarData->hoverColor;
}

MenuBar& MenuBar::onItemClick(std::function<void(const MenuItem&)> callback) {
    m_menuBarData->onItemClickCallback = std::move(callback);
    return *this;
}

// =============================================================================
// ContextMenu Implementation
// =============================================================================

struct ContextMenu::ContextMenuData {
    std::vector<MenuItem> items;
    bool visible = false;
    float posX = 0.0f;
    float posY = 0.0f;
    int highlightedIndex = -1;
    
    Color menuColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
    Color hoverColor = Color(0.9f, 0.95f, 1.0f, 1.0f);
    float itemHeight = 28.0f;
    float minWidth = 150.0f;
    
    std::function<void(const MenuItem&)> onItemClickCallback;
    std::function<void()> onCloseCallback;
};

ContextMenu::ContextMenu()
    : Widget()
    , m_contextMenuData(std::make_shared<ContextMenuData>())
{
    backgroundColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
    borderWidth(1.0f);
    borderColor(Color(0.8f, 0.8f, 0.8f, 1.0f));
    shadow(8.0f, 0.0f, 4.0f, Color(0.0f, 0.0f, 0.0f, 0.15f));
    visible(false);
}

ContextMenu ContextMenu::create() {
    return ContextMenu();
}

ContextMenu& ContextMenu::items(const std::vector<MenuItem>& items) {
    m_contextMenuData->items = items;
    return *this;
}

ContextMenu& ContextMenu::addItem(const MenuItem& item) {
    m_contextMenuData->items.push_back(item);
    return *this;
}

ContextMenu& ContextMenu::addSeparator() {
    m_contextMenuData->items.push_back(MenuItem::separator());
    return *this;
}

ContextMenu& ContextMenu::removeItem(const std::string& id) {
    auto it = std::remove_if(m_contextMenuData->items.begin(), m_contextMenuData->items.end(),
        [&id](const MenuItem& item) { return item.id == id; });
    m_contextMenuData->items.erase(it, m_contextMenuData->items.end());
    return *this;
}

ContextMenu& ContextMenu::clearItems() {
    m_contextMenuData->items.clear();
    return *this;
}

const std::vector<MenuItem>& ContextMenu::getItems() const {
    return m_contextMenuData->items;
}

ContextMenu& ContextMenu::show(float x, float y) {
    m_contextMenuData->posX = x;
    m_contextMenuData->posY = y;
    m_contextMenuData->visible = true;
    m_contextMenuData->highlightedIndex = -1;
    visible(true);
    return *this;
}

ContextMenu& ContextMenu::hide() {
    m_contextMenuData->visible = false;
    visible(false);
    if (m_contextMenuData->onCloseCallback) {
        m_contextMenuData->onCloseCallback();
    }
    return *this;
}

bool ContextMenu::isMenuVisible() const {
    return m_contextMenuData->visible;
}

void ContextMenu::getPosition(float& x, float& y) const {
    x = m_contextMenuData->posX;
    y = m_contextMenuData->posY;
}

ContextMenu& ContextMenu::menuColor(const Color& color) {
    m_contextMenuData->menuColor = color;
    backgroundColor(color);
    return *this;
}

const Color& ContextMenu::getMenuColor() const {
    return m_contextMenuData->menuColor;
}

ContextMenu& ContextMenu::hoverColor(const Color& color) {
    m_contextMenuData->hoverColor = color;
    return *this;
}

const Color& ContextMenu::getHoverColor() const {
    return m_contextMenuData->hoverColor;
}

ContextMenu& ContextMenu::itemHeight(float height) {
    m_contextMenuData->itemHeight = height;
    return *this;
}

float ContextMenu::getItemHeight() const {
    return m_contextMenuData->itemHeight;
}

ContextMenu& ContextMenu::minWidth(float width) {
    m_contextMenuData->minWidth = width;
    return *this;
}

float ContextMenu::getMinWidth() const {
    return m_contextMenuData->minWidth;
}

void ContextMenu::highlightNext() {
    if (m_contextMenuData->items.empty()) return;
    
    int start = m_contextMenuData->highlightedIndex;
    do {
        m_contextMenuData->highlightedIndex++;
        if (m_contextMenuData->highlightedIndex >= static_cast<int>(m_contextMenuData->items.size())) {
            m_contextMenuData->highlightedIndex = 0;
        }
        const auto& item = m_contextMenuData->items[m_contextMenuData->highlightedIndex];
        if (item.type != MenuItemType::Separator && item.enabled) {
            break;
        }
    } while (m_contextMenuData->highlightedIndex != start);
}

void ContextMenu::highlightPrevious() {
    if (m_contextMenuData->items.empty()) return;
    
    int start = m_contextMenuData->highlightedIndex;
    do {
        m_contextMenuData->highlightedIndex--;
        if (m_contextMenuData->highlightedIndex < 0) {
            m_contextMenuData->highlightedIndex = static_cast<int>(m_contextMenuData->items.size()) - 1;
        }
        const auto& item = m_contextMenuData->items[m_contextMenuData->highlightedIndex];
        if (item.type != MenuItemType::Separator && item.enabled) {
            break;
        }
    } while (m_contextMenuData->highlightedIndex != start);
}

void ContextMenu::selectHighlighted() {
    if (m_contextMenuData->highlightedIndex >= 0 && 
        m_contextMenuData->highlightedIndex < static_cast<int>(m_contextMenuData->items.size())) {
        const auto& item = m_contextMenuData->items[m_contextMenuData->highlightedIndex];
        if (item.enabled && item.type != MenuItemType::Separator) {
            if (item.action) {
                item.action();
            }
            if (m_contextMenuData->onItemClickCallback) {
                m_contextMenuData->onItemClickCallback(item);
            }
            hide();
        }
    }
}

int ContextMenu::getHighlightedIndex() const {
    return m_contextMenuData->highlightedIndex;
}

ContextMenu& ContextMenu::onItemClick(std::function<void(const MenuItem&)> callback) {
    m_contextMenuData->onItemClickCallback = std::move(callback);
    return *this;
}

ContextMenu& ContextMenu::onClose(std::function<void()> callback) {
    m_contextMenuData->onCloseCallback = std::move(callback);
    return *this;
}


// =============================================================================
// CommandPalette Implementation
// =============================================================================

struct CommandPalette::CommandPaletteData {
    std::vector<MenuItem> commands;
    bool visible = false;
    std::string searchText;
    std::string placeholder = "Type a command...";
    int highlightedIndex = 0;
    int maxVisibleItems = 10;
    
    float paletteWidth = 500.0f;
    Color paletteColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
    Color hoverColor = Color(0.9f, 0.95f, 1.0f, 1.0f);
    float itemHeight = 36.0f;
    
    std::function<void(const MenuItem&)> onSelectCallback;
    std::function<void()> onCloseCallback;
    std::function<void(const std::string&)> onSearchChangeCallback;
};

CommandPalette::CommandPalette()
    : Widget()
    , m_paletteData(std::make_shared<CommandPaletteData>())
{
    backgroundColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
    borderRadius(8.0f);
    shadow(16.0f, 0.0f, 8.0f, Color(0.0f, 0.0f, 0.0f, 0.2f));
    visible(false);
}

CommandPalette CommandPalette::create() {
    return CommandPalette();
}

CommandPalette& CommandPalette::commands(const std::vector<MenuItem>& commands) {
    m_paletteData->commands = commands;
    return *this;
}

CommandPalette& CommandPalette::addCommand(const MenuItem& command) {
    m_paletteData->commands.push_back(command);
    return *this;
}

CommandPalette& CommandPalette::removeCommand(const std::string& id) {
    auto it = std::remove_if(m_paletteData->commands.begin(), m_paletteData->commands.end(),
        [&id](const MenuItem& cmd) { return cmd.id == id; });
    m_paletteData->commands.erase(it, m_paletteData->commands.end());
    return *this;
}

const std::vector<MenuItem>& CommandPalette::getCommands() const {
    return m_paletteData->commands;
}

std::vector<MenuItem> CommandPalette::getFilteredCommands() const {
    if (m_paletteData->searchText.empty()) {
        return m_paletteData->commands;
    }
    
    std::vector<MenuItem> filtered;
    std::string searchLower = m_paletteData->searchText;
    std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(),
        [](unsigned char c) { return std::tolower(c); });
    
    for (const auto& cmd : m_paletteData->commands) {
        std::string textLower = cmd.text;
        std::transform(textLower.begin(), textLower.end(), textLower.begin(),
            [](unsigned char c) { return std::tolower(c); });
        
        std::string idLower = cmd.id;
        std::transform(idLower.begin(), idLower.end(), idLower.begin(),
            [](unsigned char c) { return std::tolower(c); });
        
        if (textLower.find(searchLower) != std::string::npos ||
            idLower.find(searchLower) != std::string::npos) {
            filtered.push_back(cmd);
        }
    }
    
    return filtered;
}

CommandPalette& CommandPalette::show() {
    m_paletteData->visible = true;
    m_paletteData->searchText.clear();
    m_paletteData->highlightedIndex = 0;
    visible(true);
    return *this;
}

CommandPalette& CommandPalette::hide() {
    m_paletteData->visible = false;
    visible(false);
    if (m_paletteData->onCloseCallback) {
        m_paletteData->onCloseCallback();
    }
    return *this;
}

bool CommandPalette::isPaletteVisible() const {
    return m_paletteData->visible;
}

CommandPalette& CommandPalette::searchText(const std::string& text) {
    m_paletteData->searchText = text;
    m_paletteData->highlightedIndex = 0;
    if (m_paletteData->onSearchChangeCallback) {
        m_paletteData->onSearchChangeCallback(text);
    }
    return *this;
}

const std::string& CommandPalette::getSearchText() const {
    return m_paletteData->searchText;
}

CommandPalette& CommandPalette::placeholder(const std::string& text) {
    m_paletteData->placeholder = text;
    return *this;
}

const std::string& CommandPalette::getPlaceholder() const {
    return m_paletteData->placeholder;
}

CommandPalette& CommandPalette::maxVisibleItems(int count) {
    m_paletteData->maxVisibleItems = count;
    return *this;
}

int CommandPalette::getMaxVisibleItems() const {
    return m_paletteData->maxVisibleItems;
}

CommandPalette& CommandPalette::paletteWidth(float width) {
    m_paletteData->paletteWidth = width;
    this->width(width);
    return *this;
}

float CommandPalette::getPaletteWidth() const {
    return m_paletteData->paletteWidth;
}

CommandPalette& CommandPalette::paletteColor(const Color& color) {
    m_paletteData->paletteColor = color;
    backgroundColor(color);
    return *this;
}

const Color& CommandPalette::getPaletteColor() const {
    return m_paletteData->paletteColor;
}

CommandPalette& CommandPalette::hoverColor(const Color& color) {
    m_paletteData->hoverColor = color;
    return *this;
}

const Color& CommandPalette::getHoverColor() const {
    return m_paletteData->hoverColor;
}

CommandPalette& CommandPalette::itemHeight(float height) {
    m_paletteData->itemHeight = height;
    return *this;
}

float CommandPalette::getItemHeight() const {
    return m_paletteData->itemHeight;
}

void CommandPalette::highlightNext() {
    auto filtered = getFilteredCommands();
    if (filtered.empty()) return;
    
    m_paletteData->highlightedIndex++;
    if (m_paletteData->highlightedIndex >= static_cast<int>(filtered.size())) {
        m_paletteData->highlightedIndex = 0;
    }
}

void CommandPalette::highlightPrevious() {
    auto filtered = getFilteredCommands();
    if (filtered.empty()) return;
    
    m_paletteData->highlightedIndex--;
    if (m_paletteData->highlightedIndex < 0) {
        m_paletteData->highlightedIndex = static_cast<int>(filtered.size()) - 1;
    }
}

void CommandPalette::selectHighlighted() {
    auto filtered = getFilteredCommands();
    if (m_paletteData->highlightedIndex >= 0 && 
        m_paletteData->highlightedIndex < static_cast<int>(filtered.size())) {
        const auto& cmd = filtered[m_paletteData->highlightedIndex];
        if (cmd.enabled) {
            if (cmd.action) {
                cmd.action();
            }
            if (m_paletteData->onSelectCallback) {
                m_paletteData->onSelectCallback(cmd);
            }
            hide();
        }
    }
}

int CommandPalette::getHighlightedIndex() const {
    return m_paletteData->highlightedIndex;
}

CommandPalette& CommandPalette::onSelect(std::function<void(const MenuItem&)> callback) {
    m_paletteData->onSelectCallback = std::move(callback);
    return *this;
}

CommandPalette& CommandPalette::onClose(std::function<void()> callback) {
    m_paletteData->onCloseCallback = std::move(callback);
    return *this;
}

CommandPalette& CommandPalette::onSearchChange(std::function<void(const std::string&)> callback) {
    m_paletteData->onSearchChangeCallback = std::move(callback);
    return *this;
}

} // namespace KillerGK
