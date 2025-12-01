/**
 * @file TabControl.hpp
 * @brief TabControl widget for KillerGK with animated tab transitions
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
 * @enum TabPosition
 * @brief Position of tab headers
 */
enum class TabPosition {
    Top,
    Bottom,
    Left,
    Right
};

/**
 * @struct TabItem
 * @brief Represents a tab in the TabControl
 */
struct TabItem {
    std::string id;              ///< Unique tab identifier
    std::string title;           ///< Tab title text
    std::string icon;            ///< Optional icon path
    bool closable = false;       ///< Whether tab can be closed
    bool enabled = true;         ///< Whether tab is selectable
    Widget* content = nullptr;   ///< Tab content widget
    std::any userData;           ///< Custom user data

    TabItem() = default;
    TabItem(const std::string& tabId, const std::string& tabTitle)
        : id(tabId), title(tabTitle) {}
    TabItem(const std::string& tabId, const std::string& tabTitle, Widget* tabContent)
        : id(tabId), title(tabTitle), content(tabContent) {}
};

/**
 * @class TabControl
 * @brief Tab panel widget with animated transitions between tabs
 * 
 * Manages multiple tab panels with customizable tab headers,
 * animated transitions, and closable tabs.
 * 
 * Example:
 * @code
 * auto tabs = TabControl::create()
 *     .addTab(TabItem("tab1", "General", &generalPanel))
 *     .addTab(TabItem("tab2", "Settings", &settingsPanel))
 *     .tabPosition(TabPosition::Top)
 *     .animateTransition(true)
 *     .onTabChange([](const TabItem& tab) {
 *         std::cout << "Switched to: " << tab.title;
 *     });
 * @endcode
 */
class TabControl : public Widget {
public:
    using TabRenderer = std::function<void(const TabItem&, bool selected, bool hovered)>;

    virtual ~TabControl() = default;

    /**
     * @brief Create a new TabControl instance
     * @return New TabControl with default properties
     */
    static TabControl create();

    // =========================================================================
    // Tab Management
    // =========================================================================

    /**
     * @brief Set tabs from a vector
     * @param tabs Vector of TabItem
     * @return Reference to this TabControl for chaining
     */
    TabControl& tabs(const std::vector<TabItem>& tabs);

    /**
     * @brief Add a tab
     * @param tab Tab to add
     * @return Reference to this TabControl for chaining
     */
    TabControl& addTab(const TabItem& tab);

    /**
     * @brief Add tab with basic parameters
     * @param id Tab identifier
     * @param title Tab title
     * @param content Tab content widget
     * @return Reference to this TabControl for chaining
     */
    TabControl& addTab(const std::string& id, const std::string& title, Widget* content = nullptr);

    /**
     * @brief Insert tab at specific index
     * @param index Position to insert
     * @param tab Tab to insert
     * @return Reference to this TabControl for chaining
     */
    TabControl& insertTab(int index, const TabItem& tab);

    /**
     * @brief Remove tab by id
     * @param id Tab identifier
     * @return Reference to this TabControl for chaining
     */
    TabControl& removeTab(const std::string& id);

    /**
     * @brief Remove tab by index
     * @param index Tab index
     * @return Reference to this TabControl for chaining
     */
    TabControl& removeTabAt(int index);

    /**
     * @brief Clear all tabs
     * @return Reference to this TabControl for chaining
     */
    TabControl& clearTabs();

    /**
     * @brief Get all tabs
     * @return Vector of tabs
     */
    [[nodiscard]] const std::vector<TabItem>& getTabs() const;

    /**
     * @brief Get tab by id
     * @param id Tab identifier
     * @return Pointer to tab or nullptr
     */
    [[nodiscard]] TabItem* getTab(const std::string& id);

    /**
     * @brief Get tab by index
     * @param index Tab index
     * @return Pointer to tab or nullptr
     */
    [[nodiscard]] TabItem* getTabAt(int index);

    /**
     * @brief Get tab count
     * @return Number of tabs
     */
    [[nodiscard]] size_t getTabCount() const;

    // =========================================================================
    // Selection
    // =========================================================================

    /**
     * @brief Select tab by id
     * @param id Tab identifier
     * @return Reference to this TabControl for chaining
     */
    TabControl& selectTab(const std::string& id);

    /**
     * @brief Select tab by index
     * @param index Tab index
     * @return Reference to this TabControl for chaining
     */
    TabControl& selectTabAt(int index);

    /**
     * @brief Select next tab
     * @return Reference to this TabControl for chaining
     */
    TabControl& selectNext();

    /**
     * @brief Select previous tab
     * @return Reference to this TabControl for chaining
     */
    TabControl& selectPrevious();

    /**
     * @brief Get selected tab
     * @return Pointer to selected tab or nullptr
     */
    [[nodiscard]] const TabItem* getSelectedTab() const;

    /**
     * @brief Get selected tab index
     * @return Selected tab index or -1
     */
    [[nodiscard]] int getSelectedIndex() const;

    // =========================================================================
    // Appearance
    // =========================================================================

    /**
     * @brief Set tab header position
     * @param position Tab position
     * @return Reference to this TabControl for chaining
     */
    TabControl& tabPosition(TabPosition position);

    /**
     * @brief Get tab header position
     * @return Tab position
     */
    [[nodiscard]] TabPosition getTabPosition() const;

    /**
     * @brief Set tab header height (or width for left/right)
     * @param size Tab header size
     * @return Reference to this TabControl for chaining
     */
    TabControl& tabHeaderSize(float size);

    /**
     * @brief Get tab header size
     * @return Tab header size
     */
    [[nodiscard]] float getTabHeaderSize() const;

    /**
     * @brief Set minimum tab width
     * @param width Minimum width
     * @return Reference to this TabControl for chaining
     */
    TabControl& minTabWidth(float width);

    /**
     * @brief Get minimum tab width
     * @return Minimum tab width
     */
    [[nodiscard]] float getMinTabWidth() const;

    /**
     * @brief Set maximum tab width
     * @param width Maximum width
     * @return Reference to this TabControl for chaining
     */
    TabControl& maxTabWidth(float width);

    /**
     * @brief Get maximum tab width
     * @return Maximum tab width
     */
    [[nodiscard]] float getMaxTabWidth() const;

    /**
     * @brief Set tab header background color
     * @param color Header background color
     * @return Reference to this TabControl for chaining
     */
    TabControl& headerColor(const Color& color);

    /**
     * @brief Get tab header background color
     * @return Header background color
     */
    [[nodiscard]] const Color& getHeaderColor() const;

    /**
     * @brief Set selected tab color
     * @param color Selected tab color
     * @return Reference to this TabControl for chaining
     */
    TabControl& selectedTabColor(const Color& color);

    /**
     * @brief Get selected tab color
     * @return Selected tab color
     */
    [[nodiscard]] const Color& getSelectedTabColor() const;

    /**
     * @brief Set tab hover color
     * @param color Hover color
     * @return Reference to this TabControl for chaining
     */
    TabControl& hoverColor(const Color& color);

    /**
     * @brief Get tab hover color
     * @return Hover color
     */
    [[nodiscard]] const Color& getHoverColor() const;

    /**
     * @brief Set content area background color
     * @param color Content background color
     * @return Reference to this TabControl for chaining
     */
    TabControl& contentColor(const Color& color);

    /**
     * @brief Get content area background color
     * @return Content background color
     */
    [[nodiscard]] const Color& getContentColor() const;

    /**
     * @brief Show or hide tab indicator line
     * @param show Whether to show indicator
     * @return Reference to this TabControl for chaining
     */
    TabControl& showIndicator(bool show);

    /**
     * @brief Check if indicator is shown
     * @return true if indicator visible
     */
    [[nodiscard]] bool hasIndicator() const;

    /**
     * @brief Set indicator color
     * @param color Indicator color
     * @return Reference to this TabControl for chaining
     */
    TabControl& indicatorColor(const Color& color);

    /**
     * @brief Get indicator color
     * @return Indicator color
     */
    [[nodiscard]] const Color& getIndicatorColor() const;

    // =========================================================================
    // Animation
    // =========================================================================

    /**
     * @brief Enable or disable transition animation
     * @param enabled Whether animation is enabled
     * @return Reference to this TabControl for chaining
     */
    TabControl& animateTransition(bool enabled);

    /**
     * @brief Check if transition animation is enabled
     * @return true if animation enabled
     */
    [[nodiscard]] bool hasTransitionAnimation() const;

    /**
     * @brief Set transition animation duration
     * @param duration Duration in milliseconds
     * @return Reference to this TabControl for chaining
     */
    TabControl& transitionDuration(float duration);

    /**
     * @brief Get transition animation duration
     * @return Duration in milliseconds
     */
    [[nodiscard]] float getTransitionDuration() const;

    // =========================================================================
    // Custom Rendering
    // =========================================================================

    /**
     * @brief Set custom tab header renderer
     * @param renderer Tab rendering function
     * @return Reference to this TabControl for chaining
     */
    TabControl& tabRenderer(TabRenderer renderer);

    // =========================================================================
    // Callbacks
    // =========================================================================

    /**
     * @brief Set tab change callback
     * @param callback Function called when selected tab changes
     * @return Reference to this TabControl for chaining
     */
    TabControl& onTabChange(std::function<void(const TabItem&)> callback);

    /**
     * @brief Set tab close callback
     * @param callback Function called when tab close is requested
     * @return Reference to this TabControl for chaining
     */
    TabControl& onTabClose(std::function<bool(const TabItem&)> callback);

    /**
     * @brief Set tab reorder callback
     * @param callback Function called when tabs are reordered
     * @return Reference to this TabControl for chaining
     */
    TabControl& onTabReorder(std::function<void(int oldIndex, int newIndex)> callback);

protected:
    TabControl();

    struct TabControlData;
    std::shared_ptr<TabControlData> m_tabData;
};

} // namespace KillerGK
