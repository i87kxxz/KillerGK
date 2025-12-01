/**
 * @file ComboBox.hpp
 * @brief ComboBox widget for KillerGK with dropdown list and search filtering
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
 * @struct ComboBoxItem
 * @brief Represents an item in the ComboBox dropdown list
 */
struct ComboBoxItem {
    std::string id;           ///< Unique identifier for the item
    std::string text;         ///< Display text
    std::string icon;         ///< Optional icon path
    bool enabled = true;      ///< Whether item is selectable
    std::any userData;        ///< Custom user data

    ComboBoxItem() = default;
    ComboBoxItem(const std::string& itemId, const std::string& itemText)
        : id(itemId), text(itemText) {}
    ComboBoxItem(const std::string& itemId, const std::string& itemText, const std::string& itemIcon)
        : id(itemId), text(itemText), icon(itemIcon) {}
};

/**
 * @class ComboBox
 * @brief Dropdown list widget with search filtering and custom item rendering
 * 
 * Supports searchable dropdown lists with customizable item rendering,
 * keyboard navigation, and selection callbacks.
 * 
 * Example:
 * @code
 * auto combo = ComboBox::create()
 *     .placeholder("Select an option...")
 *     .items({{"opt1", "Option 1"}, {"opt2", "Option 2"}})
 *     .searchable(true)
 *     .onSelect([](const ComboBoxItem& item) { 
 *         std::cout << "Selected: " << item.text; 
 *     });
 * @endcode
 */
class ComboBox : public Widget {
public:
    using ItemRenderer = std::function<void(const ComboBoxItem&, bool selected, bool hovered)>;

    virtual ~ComboBox() = default;

    /**
     * @brief Create a new ComboBox instance
     * @return New ComboBox with default properties
     */
    static ComboBox create();

    // =========================================================================
    // Items Management
    // =========================================================================

    /**
     * @brief Set items from a vector
     * @param items Vector of ComboBoxItem
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& items(const std::vector<ComboBoxItem>& items);

    /**
     * @brief Add a single item
     * @param item Item to add
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& addItem(const ComboBoxItem& item);

    /**
     * @brief Add item with id and text
     * @param id Item identifier
     * @param text Display text
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& addItem(const std::string& id, const std::string& text);

    /**
     * @brief Remove item by id
     * @param id Item identifier to remove
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& removeItem(const std::string& id);

    /**
     * @brief Clear all items
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& clearItems();

    /**
     * @brief Get all items
     * @return Vector of all items
     */
    [[nodiscard]] const std::vector<ComboBoxItem>& getItems() const;

    /**
     * @brief Get filtered items based on search text
     * @return Vector of items matching current filter
     */
    [[nodiscard]] std::vector<ComboBoxItem> getFilteredItems() const;

    // =========================================================================
    // Selection
    // =========================================================================

    /**
     * @brief Select item by id
     * @param id Item identifier to select
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& select(const std::string& id);

    /**
     * @brief Select item by index
     * @param index Index of item to select
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& selectIndex(int index);

    /**
     * @brief Clear selection
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& clearSelection();

    /**
     * @brief Get selected item
     * @return Pointer to selected item or nullptr if none selected
     */
    [[nodiscard]] const ComboBoxItem* getSelectedItem() const;

    /**
     * @brief Get selected item index
     * @return Index of selected item or -1 if none selected
     */
    [[nodiscard]] int getSelectedIndex() const;

    // =========================================================================
    // Dropdown State
    // =========================================================================

    /**
     * @brief Open the dropdown
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& open();

    /**
     * @brief Close the dropdown
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& close();

    /**
     * @brief Toggle dropdown open/closed
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& toggle();

    /**
     * @brief Check if dropdown is open
     * @return true if dropdown is open
     */
    [[nodiscard]] bool isOpen() const;

    // =========================================================================
    // Search/Filter
    // =========================================================================

    /**
     * @brief Enable or disable search filtering
     * @param enabled Whether search is enabled
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& searchable(bool enabled);

    /**
     * @brief Check if search is enabled
     * @return true if searchable
     */
    [[nodiscard]] bool isSearchable() const;

    /**
     * @brief Set search/filter text
     * @param text Search text
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& searchText(const std::string& text);

    /**
     * @brief Get current search text
     * @return Current search filter text
     */
    [[nodiscard]] const std::string& getSearchText() const;

    // =========================================================================
    // Appearance
    // =========================================================================

    /**
     * @brief Set placeholder text when nothing is selected
     * @param text Placeholder text
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& placeholder(const std::string& text);

    /**
     * @brief Get placeholder text
     * @return Placeholder text
     */
    [[nodiscard]] const std::string& getPlaceholder() const;

    /**
     * @brief Set maximum visible items in dropdown
     * @param count Maximum items to show
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& maxVisibleItems(int count);

    /**
     * @brief Get maximum visible items
     * @return Maximum visible items count
     */
    [[nodiscard]] int getMaxVisibleItems() const;

    /**
     * @brief Set dropdown item height
     * @param height Height in pixels
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& itemHeight(float height);

    /**
     * @brief Get dropdown item height
     * @return Item height in pixels
     */
    [[nodiscard]] float getItemHeight() const;

    /**
     * @brief Set custom item renderer
     * @param renderer Custom rendering function
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& itemRenderer(ItemRenderer renderer);

    // =========================================================================
    // Colors
    // =========================================================================

    /**
     * @brief Set dropdown background color
     * @param color Background color
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& dropdownColor(const Color& color);

    /**
     * @brief Get dropdown background color
     * @return Dropdown background color
     */
    [[nodiscard]] const Color& getDropdownColor() const;

    /**
     * @brief Set item hover color
     * @param color Hover color
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& hoverColor(const Color& color);

    /**
     * @brief Get item hover color
     * @return Hover color
     */
    [[nodiscard]] const Color& getHoverColor() const;

    /**
     * @brief Set selected item color
     * @param color Selected color
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& selectedColor(const Color& color);

    /**
     * @brief Get selected item color
     * @return Selected color
     */
    [[nodiscard]] const Color& getSelectedColor() const;

    // =========================================================================
    // Callbacks
    // =========================================================================

    /**
     * @brief Set selection change callback
     * @param callback Function called when selection changes
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& onSelect(std::function<void(const ComboBoxItem&)> callback);

    /**
     * @brief Set dropdown open/close callback
     * @param callback Function called when dropdown opens or closes
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& onDropdownToggle(std::function<void(bool)> callback);

    /**
     * @brief Set search text change callback
     * @param callback Function called when search text changes
     * @return Reference to this ComboBox for chaining
     */
    ComboBox& onSearch(std::function<void(const std::string&)> callback);

    // =========================================================================
    // Keyboard Navigation
    // =========================================================================

    /**
     * @brief Get currently highlighted item index (for keyboard navigation)
     * @return Highlighted item index or -1
     */
    [[nodiscard]] int getHighlightedIndex() const;

    /**
     * @brief Move highlight up
     */
    void highlightPrevious();

    /**
     * @brief Move highlight down
     */
    void highlightNext();

    /**
     * @brief Select currently highlighted item
     */
    void selectHighlighted();

protected:
    ComboBox();

    struct ComboBoxData;
    std::shared_ptr<ComboBoxData> m_comboData;
};

} // namespace KillerGK
