/**
 * @file TabControl.cpp
 * @brief TabControl widget implementation
 */

#include "KillerGK/widgets/TabControl.hpp"
#include <algorithm>

namespace KillerGK {

// =============================================================================
// TabControlData - Internal data structure
// =============================================================================

struct TabControl::TabControlData {
    std::vector<TabItem> tabs;
    int selectedIndex = -1;
    
    // Appearance
    TabPosition position = TabPosition::Top;
    float tabHeaderSize = 40.0f;
    float minTabWidth = 80.0f;
    float maxTabWidth = 200.0f;
    Color headerColor = Color(0.95f, 0.95f, 0.95f, 1.0f);
    Color selectedTabColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
    Color hoverColor = Color(0.9f, 0.9f, 0.9f, 1.0f);
    Color contentColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
    bool showIndicator = true;
    Color indicatorColor = Color(0.25f, 0.47f, 0.85f, 1.0f);
    
    // Animation
    bool animateTransitions = true;
    float transitionDuration = 200.0f;
    
    // Custom renderer
    TabRenderer customRenderer;
    
    // Callbacks
    std::function<void(const TabItem&)> onTabChangeCallback;
    std::function<bool(const TabItem&)> onTabCloseCallback;
    std::function<void(int, int)> onTabReorderCallback;
};

// =============================================================================
// TabControl Implementation
// =============================================================================

TabControl::TabControl()
    : Widget()
    , m_tabData(std::make_shared<TabControlData>())
{
    backgroundColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
}

TabControl TabControl::create() {
    return TabControl();
}

// Tab Management
TabControl& TabControl::tabs(const std::vector<TabItem>& tabs) {
    m_tabData->tabs = tabs;
    if (!tabs.empty() && m_tabData->selectedIndex < 0) {
        m_tabData->selectedIndex = 0;
    }
    return *this;
}

TabControl& TabControl::addTab(const TabItem& tab) {
    m_tabData->tabs.push_back(tab);
    if (m_tabData->selectedIndex < 0) {
        m_tabData->selectedIndex = 0;
    }
    return *this;
}

TabControl& TabControl::addTab(const std::string& id, const std::string& title, Widget* content) {
    TabItem tab(id, title, content);
    return addTab(tab);
}

TabControl& TabControl::insertTab(int index, const TabItem& tab) {
    if (index < 0) index = 0;
    if (index > static_cast<int>(m_tabData->tabs.size())) {
        index = static_cast<int>(m_tabData->tabs.size());
    }
    
    m_tabData->tabs.insert(m_tabData->tabs.begin() + index, tab);
    
    // Adjust selection if needed
    if (m_tabData->selectedIndex >= index) {
        m_tabData->selectedIndex++;
    }
    if (m_tabData->selectedIndex < 0) {
        m_tabData->selectedIndex = 0;
    }
    
    return *this;
}

TabControl& TabControl::removeTab(const std::string& id) {
    auto it = std::find_if(m_tabData->tabs.begin(), m_tabData->tabs.end(),
        [&id](const TabItem& tab) { return tab.id == id; });
    
    if (it != m_tabData->tabs.end()) {
        int removedIndex = static_cast<int>(std::distance(m_tabData->tabs.begin(), it));
        m_tabData->tabs.erase(it);
        
        // Adjust selection
        if (m_tabData->selectedIndex >= static_cast<int>(m_tabData->tabs.size())) {
            m_tabData->selectedIndex = static_cast<int>(m_tabData->tabs.size()) - 1;
        }
        if (m_tabData->selectedIndex == removedIndex && m_tabData->onTabChangeCallback && 
            m_tabData->selectedIndex >= 0) {
            m_tabData->onTabChangeCallback(m_tabData->tabs[m_tabData->selectedIndex]);
        }
    }
    
    return *this;
}

TabControl& TabControl::removeTabAt(int index) {
    if (index >= 0 && index < static_cast<int>(m_tabData->tabs.size())) {
        m_tabData->tabs.erase(m_tabData->tabs.begin() + index);
        
        if (m_tabData->selectedIndex >= static_cast<int>(m_tabData->tabs.size())) {
            m_tabData->selectedIndex = static_cast<int>(m_tabData->tabs.size()) - 1;
        }
        if (m_tabData->selectedIndex == index && m_tabData->onTabChangeCallback && 
            m_tabData->selectedIndex >= 0) {
            m_tabData->onTabChangeCallback(m_tabData->tabs[m_tabData->selectedIndex]);
        }
    }
    return *this;
}

TabControl& TabControl::clearTabs() {
    m_tabData->tabs.clear();
    m_tabData->selectedIndex = -1;
    return *this;
}

const std::vector<TabItem>& TabControl::getTabs() const {
    return m_tabData->tabs;
}

TabItem* TabControl::getTab(const std::string& id) {
    for (auto& tab : m_tabData->tabs) {
        if (tab.id == id) return &tab;
    }
    return nullptr;
}

TabItem* TabControl::getTabAt(int index) {
    if (index >= 0 && index < static_cast<int>(m_tabData->tabs.size())) {
        return &m_tabData->tabs[index];
    }
    return nullptr;
}

size_t TabControl::getTabCount() const {
    return m_tabData->tabs.size();
}

// Selection
TabControl& TabControl::selectTab(const std::string& id) {
    for (size_t i = 0; i < m_tabData->tabs.size(); ++i) {
        if (m_tabData->tabs[i].id == id && m_tabData->tabs[i].enabled) {
            if (m_tabData->selectedIndex != static_cast<int>(i)) {
                m_tabData->selectedIndex = static_cast<int>(i);
                if (m_tabData->onTabChangeCallback) {
                    m_tabData->onTabChangeCallback(m_tabData->tabs[i]);
                }
            }
            break;
        }
    }
    return *this;
}

TabControl& TabControl::selectTabAt(int index) {
    if (index >= 0 && index < static_cast<int>(m_tabData->tabs.size()) && 
        m_tabData->tabs[index].enabled) {
        if (m_tabData->selectedIndex != index) {
            m_tabData->selectedIndex = index;
            if (m_tabData->onTabChangeCallback) {
                m_tabData->onTabChangeCallback(m_tabData->tabs[index]);
            }
        }
    }
    return *this;
}

TabControl& TabControl::selectNext() {
    if (m_tabData->tabs.empty()) return *this;
    
    int nextIndex = m_tabData->selectedIndex + 1;
    while (nextIndex < static_cast<int>(m_tabData->tabs.size())) {
        if (m_tabData->tabs[nextIndex].enabled) {
            selectTabAt(nextIndex);
            break;
        }
        nextIndex++;
    }
    return *this;
}

TabControl& TabControl::selectPrevious() {
    if (m_tabData->tabs.empty()) return *this;
    
    int prevIndex = m_tabData->selectedIndex - 1;
    while (prevIndex >= 0) {
        if (m_tabData->tabs[prevIndex].enabled) {
            selectTabAt(prevIndex);
            break;
        }
        prevIndex--;
    }
    return *this;
}

const TabItem* TabControl::getSelectedTab() const {
    if (m_tabData->selectedIndex >= 0 && 
        m_tabData->selectedIndex < static_cast<int>(m_tabData->tabs.size())) {
        return &m_tabData->tabs[m_tabData->selectedIndex];
    }
    return nullptr;
}

int TabControl::getSelectedIndex() const {
    return m_tabData->selectedIndex;
}


// Appearance
TabControl& TabControl::tabPosition(TabPosition position) {
    m_tabData->position = position;
    return *this;
}

TabPosition TabControl::getTabPosition() const {
    return m_tabData->position;
}

TabControl& TabControl::tabHeaderSize(float size) {
    m_tabData->tabHeaderSize = size;
    return *this;
}

float TabControl::getTabHeaderSize() const {
    return m_tabData->tabHeaderSize;
}

TabControl& TabControl::minTabWidth(float width) {
    m_tabData->minTabWidth = width;
    return *this;
}

float TabControl::getMinTabWidth() const {
    return m_tabData->minTabWidth;
}

TabControl& TabControl::maxTabWidth(float width) {
    m_tabData->maxTabWidth = width;
    return *this;
}

float TabControl::getMaxTabWidth() const {
    return m_tabData->maxTabWidth;
}

TabControl& TabControl::headerColor(const Color& color) {
    m_tabData->headerColor = color;
    return *this;
}

const Color& TabControl::getHeaderColor() const {
    return m_tabData->headerColor;
}

TabControl& TabControl::selectedTabColor(const Color& color) {
    m_tabData->selectedTabColor = color;
    return *this;
}

const Color& TabControl::getSelectedTabColor() const {
    return m_tabData->selectedTabColor;
}

TabControl& TabControl::hoverColor(const Color& color) {
    m_tabData->hoverColor = color;
    return *this;
}

const Color& TabControl::getHoverColor() const {
    return m_tabData->hoverColor;
}

TabControl& TabControl::contentColor(const Color& color) {
    m_tabData->contentColor = color;
    return *this;
}

const Color& TabControl::getContentColor() const {
    return m_tabData->contentColor;
}

TabControl& TabControl::showIndicator(bool show) {
    m_tabData->showIndicator = show;
    return *this;
}

bool TabControl::hasIndicator() const {
    return m_tabData->showIndicator;
}

TabControl& TabControl::indicatorColor(const Color& color) {
    m_tabData->indicatorColor = color;
    return *this;
}

const Color& TabControl::getIndicatorColor() const {
    return m_tabData->indicatorColor;
}

// Animation
TabControl& TabControl::animateTransition(bool enabled) {
    m_tabData->animateTransitions = enabled;
    return *this;
}

bool TabControl::hasTransitionAnimation() const {
    return m_tabData->animateTransitions;
}

TabControl& TabControl::transitionDuration(float duration) {
    m_tabData->transitionDuration = duration;
    return *this;
}

float TabControl::getTransitionDuration() const {
    return m_tabData->transitionDuration;
}

// Custom Rendering
TabControl& TabControl::tabRenderer(TabRenderer renderer) {
    m_tabData->customRenderer = std::move(renderer);
    return *this;
}

// Callbacks
TabControl& TabControl::onTabChange(std::function<void(const TabItem&)> callback) {
    m_tabData->onTabChangeCallback = std::move(callback);
    return *this;
}

TabControl& TabControl::onTabClose(std::function<bool(const TabItem&)> callback) {
    m_tabData->onTabCloseCallback = std::move(callback);
    return *this;
}

TabControl& TabControl::onTabReorder(std::function<void(int, int)> callback) {
    m_tabData->onTabReorderCallback = std::move(callback);
    return *this;
}

} // namespace KillerGK
