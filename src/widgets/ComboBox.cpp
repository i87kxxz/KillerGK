/**
 * @file ComboBox.cpp
 * @brief ComboBox widget implementation
 */

#include "KillerGK/widgets/ComboBox.hpp"
#include <algorithm>
#include <cctype>

namespace KillerGK {

// =============================================================================
// ComboBoxData - Internal data structure
// =============================================================================

struct ComboBox::ComboBoxData {
    std::vector<ComboBoxItem> items;
    int selectedIndex = -1;
    int highlightedIndex = -1;
    bool isOpen = false;
    bool searchEnabled = false;
    std::string searchText;
    std::string placeholder = "Select...";
    int maxVisibleItems = 8;
    float itemHeight = 32.0f;
    
    // Colors
    Color dropdownColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
    Color hoverColor = Color(0.9f, 0.95f, 1.0f, 1.0f);
    Color selectedColor = Color(0.85f, 0.9f, 1.0f, 1.0f);
    
    // Callbacks
    std::function<void(const ComboBoxItem&)> onSelectCallback;
    std::function<void(bool)> onDropdownToggleCallback;
    std::function<void(const std::string&)> onSearchCallback;
    ItemRenderer customRenderer;
};

// =============================================================================
// ComboBox Implementation
// =============================================================================

ComboBox::ComboBox()
    : Widget()
    , m_comboData(std::make_shared<ComboBoxData>())
{
    // Default styling
    backgroundColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
    borderRadius(4.0f);
    borderWidth(1.0f);
    borderColor(Color(0.8f, 0.8f, 0.8f, 1.0f));
    height(36.0f);
    padding(8.0f, 12.0f);
}

ComboBox ComboBox::create() {
    return ComboBox();
}

// Items Management
ComboBox& ComboBox::items(const std::vector<ComboBoxItem>& items) {
    m_comboData->items = items;
    m_comboData->selectedIndex = -1;
    m_comboData->highlightedIndex = -1;
    return *this;
}

ComboBox& ComboBox::addItem(const ComboBoxItem& item) {
    m_comboData->items.push_back(item);
    return *this;
}

ComboBox& ComboBox::addItem(const std::string& id, const std::string& text) {
    m_comboData->items.emplace_back(id, text);
    return *this;
}

ComboBox& ComboBox::removeItem(const std::string& id) {
    auto it = std::remove_if(m_comboData->items.begin(), m_comboData->items.end(),
        [&id](const ComboBoxItem& item) { return item.id == id; });
    
    if (it != m_comboData->items.end()) {
        size_t removedIndex = std::distance(m_comboData->items.begin(), it);
        m_comboData->items.erase(it, m_comboData->items.end());
        
        // Adjust selection if needed
        if (m_comboData->selectedIndex >= static_cast<int>(removedIndex)) {
            if (m_comboData->selectedIndex == static_cast<int>(removedIndex)) {
                m_comboData->selectedIndex = -1;
            } else {
                m_comboData->selectedIndex--;
            }
        }
    }
    return *this;
}

ComboBox& ComboBox::clearItems() {
    m_comboData->items.clear();
    m_comboData->selectedIndex = -1;
    m_comboData->highlightedIndex = -1;
    return *this;
}

const std::vector<ComboBoxItem>& ComboBox::getItems() const {
    return m_comboData->items;
}


std::vector<ComboBoxItem> ComboBox::getFilteredItems() const {
    if (!m_comboData->searchEnabled || m_comboData->searchText.empty()) {
        return m_comboData->items;
    }
    
    std::vector<ComboBoxItem> filtered;
    std::string searchLower = m_comboData->searchText;
    std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(),
        [](unsigned char c) { return std::tolower(c); });
    
    for (const auto& item : m_comboData->items) {
        std::string textLower = item.text;
        std::transform(textLower.begin(), textLower.end(), textLower.begin(),
            [](unsigned char c) { return std::tolower(c); });
        
        if (textLower.find(searchLower) != std::string::npos) {
            filtered.push_back(item);
        }
    }
    
    return filtered;
}

// Selection
ComboBox& ComboBox::select(const std::string& id) {
    for (size_t i = 0; i < m_comboData->items.size(); ++i) {
        if (m_comboData->items[i].id == id) {
            m_comboData->selectedIndex = static_cast<int>(i);
            if (m_comboData->onSelectCallback) {
                m_comboData->onSelectCallback(m_comboData->items[i]);
            }
            break;
        }
    }
    return *this;
}

ComboBox& ComboBox::selectIndex(int index) {
    if (index >= 0 && index < static_cast<int>(m_comboData->items.size())) {
        m_comboData->selectedIndex = index;
        if (m_comboData->onSelectCallback) {
            m_comboData->onSelectCallback(m_comboData->items[index]);
        }
    }
    return *this;
}

ComboBox& ComboBox::clearSelection() {
    m_comboData->selectedIndex = -1;
    return *this;
}

const ComboBoxItem* ComboBox::getSelectedItem() const {
    if (m_comboData->selectedIndex >= 0 && 
        m_comboData->selectedIndex < static_cast<int>(m_comboData->items.size())) {
        return &m_comboData->items[m_comboData->selectedIndex];
    }
    return nullptr;
}

int ComboBox::getSelectedIndex() const {
    return m_comboData->selectedIndex;
}

// Dropdown State
ComboBox& ComboBox::open() {
    if (!m_comboData->isOpen) {
        m_comboData->isOpen = true;
        m_comboData->highlightedIndex = m_comboData->selectedIndex;
        if (m_comboData->onDropdownToggleCallback) {
            m_comboData->onDropdownToggleCallback(true);
        }
    }
    return *this;
}

ComboBox& ComboBox::close() {
    if (m_comboData->isOpen) {
        m_comboData->isOpen = false;
        m_comboData->searchText.clear();
        if (m_comboData->onDropdownToggleCallback) {
            m_comboData->onDropdownToggleCallback(false);
        }
    }
    return *this;
}

ComboBox& ComboBox::toggle() {
    if (m_comboData->isOpen) {
        close();
    } else {
        open();
    }
    return *this;
}

bool ComboBox::isOpen() const {
    return m_comboData->isOpen;
}

// Search/Filter
ComboBox& ComboBox::searchable(bool enabled) {
    m_comboData->searchEnabled = enabled;
    return *this;
}

bool ComboBox::isSearchable() const {
    return m_comboData->searchEnabled;
}

ComboBox& ComboBox::searchText(const std::string& text) {
    m_comboData->searchText = text;
    m_comboData->highlightedIndex = 0;
    if (m_comboData->onSearchCallback) {
        m_comboData->onSearchCallback(text);
    }
    return *this;
}

const std::string& ComboBox::getSearchText() const {
    return m_comboData->searchText;
}

// Appearance
ComboBox& ComboBox::placeholder(const std::string& text) {
    m_comboData->placeholder = text;
    return *this;
}

const std::string& ComboBox::getPlaceholder() const {
    return m_comboData->placeholder;
}

ComboBox& ComboBox::maxVisibleItems(int count) {
    m_comboData->maxVisibleItems = count;
    return *this;
}

int ComboBox::getMaxVisibleItems() const {
    return m_comboData->maxVisibleItems;
}

ComboBox& ComboBox::itemHeight(float height) {
    m_comboData->itemHeight = height;
    return *this;
}

float ComboBox::getItemHeight() const {
    return m_comboData->itemHeight;
}

ComboBox& ComboBox::itemRenderer(ItemRenderer renderer) {
    m_comboData->customRenderer = std::move(renderer);
    return *this;
}

// Colors
ComboBox& ComboBox::dropdownColor(const Color& color) {
    m_comboData->dropdownColor = color;
    return *this;
}

const Color& ComboBox::getDropdownColor() const {
    return m_comboData->dropdownColor;
}

ComboBox& ComboBox::hoverColor(const Color& color) {
    m_comboData->hoverColor = color;
    return *this;
}

const Color& ComboBox::getHoverColor() const {
    return m_comboData->hoverColor;
}

ComboBox& ComboBox::selectedColor(const Color& color) {
    m_comboData->selectedColor = color;
    return *this;
}

const Color& ComboBox::getSelectedColor() const {
    return m_comboData->selectedColor;
}

// Callbacks
ComboBox& ComboBox::onSelect(std::function<void(const ComboBoxItem&)> callback) {
    m_comboData->onSelectCallback = std::move(callback);
    return *this;
}

ComboBox& ComboBox::onDropdownToggle(std::function<void(bool)> callback) {
    m_comboData->onDropdownToggleCallback = std::move(callback);
    return *this;
}

ComboBox& ComboBox::onSearch(std::function<void(const std::string&)> callback) {
    m_comboData->onSearchCallback = std::move(callback);
    return *this;
}

// Keyboard Navigation
int ComboBox::getHighlightedIndex() const {
    return m_comboData->highlightedIndex;
}

void ComboBox::highlightPrevious() {
    auto filtered = getFilteredItems();
    if (filtered.empty()) return;
    
    if (m_comboData->highlightedIndex <= 0) {
        m_comboData->highlightedIndex = static_cast<int>(filtered.size()) - 1;
    } else {
        m_comboData->highlightedIndex--;
    }
}

void ComboBox::highlightNext() {
    auto filtered = getFilteredItems();
    if (filtered.empty()) return;
    
    if (m_comboData->highlightedIndex >= static_cast<int>(filtered.size()) - 1) {
        m_comboData->highlightedIndex = 0;
    } else {
        m_comboData->highlightedIndex++;
    }
}

void ComboBox::selectHighlighted() {
    auto filtered = getFilteredItems();
    if (m_comboData->highlightedIndex >= 0 && 
        m_comboData->highlightedIndex < static_cast<int>(filtered.size())) {
        const auto& item = filtered[m_comboData->highlightedIndex];
        select(item.id);
        close();
    }
}

} // namespace KillerGK
