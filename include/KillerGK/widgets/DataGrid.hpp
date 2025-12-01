/**
 * @file DataGrid.hpp
 * @brief DataGrid widget for KillerGK with sorting, filtering, and virtual scrolling
 */

#pragma once

#include "Widget.hpp"
#include "../core/Types.hpp"
#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <variant>

namespace KillerGK {

/**
 * @enum SortDirection
 * @brief Sort direction for columns
 */
enum class SortDirection {
    None,
    Ascending,
    Descending
};

/**
 * @enum ColumnType
 * @brief Data type for column values
 */
enum class ColumnType {
    String,
    Number,
    Boolean,
    Date,
    Custom
};

/**
 * @struct DataGridColumn
 * @brief Column definition for DataGrid
 */
struct DataGridColumn {
    std::string id;              ///< Unique column identifier
    std::string header;          ///< Column header text
    float width = 100.0f;        ///< Column width in pixels
    float minWidth = 50.0f;      ///< Minimum column width
    float maxWidth = 500.0f;     ///< Maximum column width
    bool resizable = true;       ///< Whether column can be resized
    bool sortable = true;        ///< Whether column can be sorted
    bool visible = true;         ///< Whether column is visible
    ColumnType type = ColumnType::String;
    SortDirection sortDirection = SortDirection::None;

    DataGridColumn() = default;
    DataGridColumn(const std::string& colId, const std::string& colHeader, float colWidth = 100.0f)
        : id(colId), header(colHeader), width(colWidth) {}
};

/**
 * @typedef CellValue
 * @brief Variant type for cell values
 */
using CellValue = std::variant<std::string, double, bool, int64_t>;

/**
 * @struct DataGridRow
 * @brief Row data for DataGrid
 */
struct DataGridRow {
    std::string id;                              ///< Unique row identifier
    std::map<std::string, CellValue> cells;      ///< Cell values by column id
    bool selected = false;                       ///< Whether row is selected
    bool enabled = true;                         ///< Whether row is interactive
    std::any userData;                           ///< Custom user data

    DataGridRow() = default;
    DataGridRow(const std::string& rowId) : id(rowId) {}
    
    void setCell(const std::string& columnId, const CellValue& value) {
        cells[columnId] = value;
    }
    
    CellValue getCell(const std::string& columnId) const {
        auto it = cells.find(columnId);
        return it != cells.end() ? it->second : CellValue{std::string{}};
    }
};

/**
 * @struct DataGridFilter
 * @brief Filter configuration for a column
 */
struct DataGridFilter {
    std::string columnId;
    std::string filterText;
    std::function<bool(const CellValue&)> customFilter;
};


/**
 * @class DataGrid
 * @brief Table widget with sorting, filtering, column resizing, and virtual scrolling
 * 
 * Supports large datasets through virtual scrolling, multi-column sorting,
 * text filtering, and customizable cell rendering.
 * 
 * Example:
 * @code
 * auto grid = DataGrid::create()
 *     .columns({
 *         {"name", "Name", 150.0f},
 *         {"age", "Age", 80.0f},
 *         {"email", "Email", 200.0f}
 *     })
 *     .onSort([](const std::string& col, SortDirection dir) {
 *         std::cout << "Sorting by " << col;
 *     });
 * @endcode
 */
class DataGrid : public Widget {
public:
    using CellRenderer = std::function<void(const DataGridRow&, const DataGridColumn&, const CellValue&)>;
    using RowRenderer = std::function<void(const DataGridRow&, bool selected, bool hovered)>;

    virtual ~DataGrid() = default;

    /**
     * @brief Create a new DataGrid instance
     * @return New DataGrid with default properties
     */
    static DataGrid create();

    // =========================================================================
    // Column Management
    // =========================================================================

    /**
     * @brief Set columns from a vector
     * @param columns Vector of DataGridColumn
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& columns(const std::vector<DataGridColumn>& columns);

    /**
     * @brief Add a column
     * @param column Column to add
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& addColumn(const DataGridColumn& column);

    /**
     * @brief Add column with basic parameters
     * @param id Column identifier
     * @param header Header text
     * @param width Column width
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& addColumn(const std::string& id, const std::string& header, float width = 100.0f);

    /**
     * @brief Remove column by id
     * @param id Column identifier
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& removeColumn(const std::string& id);

    /**
     * @brief Get all columns
     * @return Vector of columns
     */
    [[nodiscard]] const std::vector<DataGridColumn>& getColumns() const;

    /**
     * @brief Get column by id
     * @param id Column identifier
     * @return Pointer to column or nullptr
     */
    [[nodiscard]] DataGridColumn* getColumn(const std::string& id);

    /**
     * @brief Set column width
     * @param id Column identifier
     * @param width New width
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& setColumnWidth(const std::string& id, float width);

    // =========================================================================
    // Row/Data Management
    // =========================================================================

    /**
     * @brief Set rows from a vector
     * @param rows Vector of DataGridRow
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& rows(const std::vector<DataGridRow>& rows);

    /**
     * @brief Add a row
     * @param row Row to add
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& addRow(const DataGridRow& row);

    /**
     * @brief Remove row by id
     * @param id Row identifier
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& removeRow(const std::string& id);

    /**
     * @brief Clear all rows
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& clearRows();

    /**
     * @brief Get all rows (unfiltered, unsorted)
     * @return Vector of all rows
     */
    [[nodiscard]] const std::vector<DataGridRow>& getRows() const;

    /**
     * @brief Get displayed rows (filtered and sorted)
     * @return Vector of displayed rows
     */
    [[nodiscard]] std::vector<DataGridRow> getDisplayedRows() const;

    /**
     * @brief Get row by id
     * @param id Row identifier
     * @return Pointer to row or nullptr
     */
    [[nodiscard]] DataGridRow* getRow(const std::string& id);

    /**
     * @brief Get total row count
     * @return Number of rows
     */
    [[nodiscard]] size_t getRowCount() const;

    /**
     * @brief Get filtered row count
     * @return Number of rows after filtering
     */
    [[nodiscard]] size_t getFilteredRowCount() const;

    // =========================================================================
    // Sorting
    // =========================================================================

    /**
     * @brief Sort by column
     * @param columnId Column to sort by
     * @param direction Sort direction
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& sortBy(const std::string& columnId, SortDirection direction);

    /**
     * @brief Clear sorting
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& clearSort();

    /**
     * @brief Get current sort column
     * @return Column id being sorted or empty string
     */
    [[nodiscard]] const std::string& getSortColumn() const;

    /**
     * @brief Get current sort direction
     * @return Current sort direction
     */
    [[nodiscard]] SortDirection getSortDirection() const;

    // =========================================================================
    // Filtering
    // =========================================================================

    /**
     * @brief Set filter for a column
     * @param columnId Column to filter
     * @param filterText Text to filter by
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& setFilter(const std::string& columnId, const std::string& filterText);

    /**
     * @brief Set custom filter for a column
     * @param columnId Column to filter
     * @param filter Custom filter function
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& setFilter(const std::string& columnId, std::function<bool(const CellValue&)> filter);

    /**
     * @brief Clear filter for a column
     * @param columnId Column to clear filter for
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& clearFilter(const std::string& columnId);

    /**
     * @brief Clear all filters
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& clearAllFilters();

    /**
     * @brief Get active filters
     * @return Vector of active filters
     */
    [[nodiscard]] const std::vector<DataGridFilter>& getFilters() const;

    // =========================================================================
    // Selection
    // =========================================================================

    /**
     * @brief Enable or disable multi-selection
     * @param enabled Whether multi-select is enabled
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& multiSelect(bool enabled);

    /**
     * @brief Check if multi-select is enabled
     * @return true if multi-select enabled
     */
    [[nodiscard]] bool isMultiSelect() const;

    /**
     * @brief Select row by id
     * @param id Row identifier
     * @param addToSelection Whether to add to existing selection (multi-select)
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& selectRow(const std::string& id, bool addToSelection = false);

    /**
     * @brief Deselect row by id
     * @param id Row identifier
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& deselectRow(const std::string& id);

    /**
     * @brief Clear all selection
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& clearSelection();

    /**
     * @brief Get selected row ids
     * @return Vector of selected row ids
     */
    [[nodiscard]] std::vector<std::string> getSelectedRowIds() const;

    /**
     * @brief Get selected rows
     * @return Vector of pointers to selected rows
     */
    [[nodiscard]] std::vector<const DataGridRow*> getSelectedRows() const;

    // =========================================================================
    // Virtual Scrolling
    // =========================================================================

    /**
     * @brief Set row height for virtual scrolling
     * @param height Row height in pixels
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& rowHeight(float height);

    /**
     * @brief Get row height
     * @return Row height in pixels
     */
    [[nodiscard]] float getRowHeight() const;

    /**
     * @brief Set header height
     * @param height Header height in pixels
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& headerHeight(float height);

    /**
     * @brief Get header height
     * @return Header height in pixels
     */
    [[nodiscard]] float getHeaderHeight() const;

    /**
     * @brief Get scroll offset
     * @return Current scroll offset in pixels
     */
    [[nodiscard]] float getScrollOffset() const;

    /**
     * @brief Set scroll offset
     * @param offset Scroll offset in pixels
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& scrollTo(float offset);

    /**
     * @brief Scroll to row by id
     * @param id Row identifier
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& scrollToRow(const std::string& id);

    /**
     * @brief Get visible row range for virtual scrolling
     * @param startIndex Output: first visible row index
     * @param endIndex Output: last visible row index (exclusive)
     */
    void getVisibleRowRange(int& startIndex, int& endIndex) const;

    // =========================================================================
    // Appearance
    // =========================================================================

    /**
     * @brief Set header background color
     * @param color Header color
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& headerColor(const Color& color);

    /**
     * @brief Get header background color
     * @return Header color
     */
    [[nodiscard]] const Color& getHeaderColor() const;

    /**
     * @brief Set alternating row colors
     * @param even Even row color
     * @param odd Odd row color
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& alternatingRowColors(const Color& even, const Color& odd);

    /**
     * @brief Set row hover color
     * @param color Hover color
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& hoverColor(const Color& color);

    /**
     * @brief Get row hover color
     * @return Hover color
     */
    [[nodiscard]] const Color& getHoverColor() const;

    /**
     * @brief Set selected row color
     * @param color Selection color
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& selectionColor(const Color& color);

    /**
     * @brief Get selected row color
     * @return Selection color
     */
    [[nodiscard]] const Color& getSelectionColor() const;

    /**
     * @brief Show or hide grid lines
     * @param show Whether to show grid lines
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& showGridLines(bool show);

    /**
     * @brief Check if grid lines are shown
     * @return true if grid lines visible
     */
    [[nodiscard]] bool hasGridLines() const;

    // =========================================================================
    // Custom Rendering
    // =========================================================================

    /**
     * @brief Set custom cell renderer
     * @param renderer Cell rendering function
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& cellRenderer(CellRenderer renderer);

    /**
     * @brief Set custom row renderer
     * @param renderer Row rendering function
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& rowRenderer(RowRenderer renderer);

    // =========================================================================
    // Callbacks
    // =========================================================================

    /**
     * @brief Set sort change callback
     * @param callback Function called when sort changes
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& onSort(std::function<void(const std::string&, SortDirection)> callback);

    /**
     * @brief Set selection change callback
     * @param callback Function called when selection changes
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& onSelectionChange(std::function<void(const std::vector<std::string>&)> callback);

    /**
     * @brief Set row double-click callback
     * @param callback Function called on row double-click
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& onRowDoubleClick(std::function<void(const DataGridRow&)> callback);

    /**
     * @brief Set column resize callback
     * @param callback Function called when column is resized
     * @return Reference to this DataGrid for chaining
     */
    DataGrid& onColumnResize(std::function<void(const std::string&, float)> callback);

protected:
    DataGrid();

    struct DataGridData;
    std::shared_ptr<DataGridData> m_gridData;
};

} // namespace KillerGK
