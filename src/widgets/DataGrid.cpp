/**
 * @file DataGrid.cpp
 * @brief DataGrid widget implementation
 */

#include "KillerGK/widgets/DataGrid.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>

namespace KillerGK {

// =============================================================================
// DataGridData - Internal data structure
// =============================================================================

struct DataGrid::DataGridData {
    std::vector<DataGridColumn> columns;
    std::vector<DataGridRow> rows;
    std::vector<DataGridFilter> filters;
    
    // Sorting
    std::string sortColumnId;
    SortDirection sortDirection = SortDirection::None;
    
    // Selection
    bool multiSelectEnabled = false;
    std::vector<std::string> selectedRowIds;
    
    // Virtual scrolling
    float rowHeight = 32.0f;
    float headerHeight = 40.0f;
    float scrollOffset = 0.0f;
    
    // Appearance
    Color headerColor = Color(0.95f, 0.95f, 0.95f, 1.0f);
    Color evenRowColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
    Color oddRowColor = Color(0.98f, 0.98f, 0.98f, 1.0f);
    Color hoverColor = Color(0.9f, 0.95f, 1.0f, 1.0f);
    Color selectionColor = Color(0.85f, 0.9f, 1.0f, 1.0f);
    bool gridLinesVisible = true;
    
    // Custom renderers
    CellRenderer customCellRenderer;
    RowRenderer customRowRenderer;
    
    // Callbacks
    std::function<void(const std::string&, SortDirection)> onSortCallback;
    std::function<void(const std::vector<std::string>&)> onSelectionChangeCallback;
    std::function<void(const DataGridRow&)> onRowDoubleClickCallback;
    std::function<void(const std::string&, float)> onColumnResizeCallback;
    
    // Cached sorted/filtered data
    mutable std::vector<size_t> displayedIndices;
    mutable bool cacheValid = false;
    
    void invalidateCache() { cacheValid = false; }
    
    void updateCache() const {
        if (cacheValid) return;
        
        displayedIndices.clear();
        
        // Apply filters
        for (size_t i = 0; i < rows.size(); ++i) {
            bool passesAllFilters = true;
            
            for (const auto& filter : filters) {
                auto cellIt = rows[i].cells.find(filter.columnId);
                if (cellIt == rows[i].cells.end()) continue;
                
                if (filter.customFilter) {
                    if (!filter.customFilter(cellIt->second)) {
                        passesAllFilters = false;
                        break;
                    }
                } else if (!filter.filterText.empty()) {
                    // Text filter - case insensitive contains
                    std::string cellText;
                    if (std::holds_alternative<std::string>(cellIt->second)) {
                        cellText = std::get<std::string>(cellIt->second);
                    } else if (std::holds_alternative<double>(cellIt->second)) {
                        cellText = std::to_string(std::get<double>(cellIt->second));
                    } else if (std::holds_alternative<int64_t>(cellIt->second)) {
                        cellText = std::to_string(std::get<int64_t>(cellIt->second));
                    } else if (std::holds_alternative<bool>(cellIt->second)) {
                        cellText = std::get<bool>(cellIt->second) ? "true" : "false";
                    }
                    
                    std::string filterLower = filter.filterText;
                    std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(),
                        [](unsigned char c) { return std::tolower(c); });
                    std::transform(cellText.begin(), cellText.end(), cellText.begin(),
                        [](unsigned char c) { return std::tolower(c); });
                    
                    if (cellText.find(filterLower) == std::string::npos) {
                        passesAllFilters = false;
                        break;
                    }
                }
            }
            
            if (passesAllFilters) {
                displayedIndices.push_back(i);
            }
        }
        
        // Apply sorting
        if (!sortColumnId.empty() && sortDirection != SortDirection::None) {
            std::sort(displayedIndices.begin(), displayedIndices.end(),
                [this](size_t a, size_t b) {
                    const auto& rowA = rows[a];
                    const auto& rowB = rows[b];
                    
                    auto cellItA = rowA.cells.find(sortColumnId);
                    auto cellItB = rowB.cells.find(sortColumnId);
                    
                    if (cellItA == rowA.cells.end() && cellItB == rowB.cells.end()) return false;
                    if (cellItA == rowA.cells.end()) return sortDirection == SortDirection::Ascending;
                    if (cellItB == rowB.cells.end()) return sortDirection == SortDirection::Descending;
                    
                    const auto& valA = cellItA->second;
                    const auto& valB = cellItB->second;
                    
                    // For descending order, we swap the comparison operands instead of negating the result
                    // This maintains strict weak ordering (returns false when a == b)
                    if (sortDirection == SortDirection::Descending) {
                        // Swap: compare B < A for descending
                        if (std::holds_alternative<std::string>(valA) && std::holds_alternative<std::string>(valB)) {
                            return std::get<std::string>(valB) < std::get<std::string>(valA);
                        } else if (std::holds_alternative<double>(valA) && std::holds_alternative<double>(valB)) {
                            return std::get<double>(valB) < std::get<double>(valA);
                        } else if (std::holds_alternative<int64_t>(valA) && std::holds_alternative<int64_t>(valB)) {
                            return std::get<int64_t>(valB) < std::get<int64_t>(valA);
                        } else if (std::holds_alternative<bool>(valA) && std::holds_alternative<bool>(valB)) {
                            return !std::get<bool>(valB) && std::get<bool>(valA);
                        }
                    } else {
                        // Ascending: compare A < B
                        if (std::holds_alternative<std::string>(valA) && std::holds_alternative<std::string>(valB)) {
                            return std::get<std::string>(valA) < std::get<std::string>(valB);
                        } else if (std::holds_alternative<double>(valA) && std::holds_alternative<double>(valB)) {
                            return std::get<double>(valA) < std::get<double>(valB);
                        } else if (std::holds_alternative<int64_t>(valA) && std::holds_alternative<int64_t>(valB)) {
                            return std::get<int64_t>(valA) < std::get<int64_t>(valB);
                        } else if (std::holds_alternative<bool>(valA) && std::holds_alternative<bool>(valB)) {
                            return !std::get<bool>(valA) && std::get<bool>(valB);
                        }
                    }
                    
                    return false;
                });
        }
        
        cacheValid = true;
    }
};


// =============================================================================
// DataGrid Implementation
// =============================================================================

DataGrid::DataGrid()
    : Widget()
    , m_gridData(std::make_shared<DataGridData>())
{
    backgroundColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
    borderWidth(1.0f);
    borderColor(Color(0.8f, 0.8f, 0.8f, 1.0f));
}

DataGrid DataGrid::create() {
    return DataGrid();
}

// Column Management
DataGrid& DataGrid::columns(const std::vector<DataGridColumn>& columns) {
    m_gridData->columns = columns;
    return *this;
}

DataGrid& DataGrid::addColumn(const DataGridColumn& column) {
    m_gridData->columns.push_back(column);
    return *this;
}

DataGrid& DataGrid::addColumn(const std::string& id, const std::string& header, float width) {
    m_gridData->columns.emplace_back(id, header, width);
    return *this;
}

DataGrid& DataGrid::removeColumn(const std::string& id) {
    auto it = std::remove_if(m_gridData->columns.begin(), m_gridData->columns.end(),
        [&id](const DataGridColumn& col) { return col.id == id; });
    m_gridData->columns.erase(it, m_gridData->columns.end());
    return *this;
}

const std::vector<DataGridColumn>& DataGrid::getColumns() const {
    return m_gridData->columns;
}

DataGridColumn* DataGrid::getColumn(const std::string& id) {
    for (auto& col : m_gridData->columns) {
        if (col.id == id) return &col;
    }
    return nullptr;
}

DataGrid& DataGrid::setColumnWidth(const std::string& id, float width) {
    if (auto* col = getColumn(id)) {
        col->width = std::clamp(width, col->minWidth, col->maxWidth);
        if (m_gridData->onColumnResizeCallback) {
            m_gridData->onColumnResizeCallback(id, col->width);
        }
    }
    return *this;
}

// Row/Data Management
DataGrid& DataGrid::rows(const std::vector<DataGridRow>& rows) {
    m_gridData->rows = rows;
    m_gridData->invalidateCache();
    return *this;
}

DataGrid& DataGrid::addRow(const DataGridRow& row) {
    m_gridData->rows.push_back(row);
    m_gridData->invalidateCache();
    return *this;
}

DataGrid& DataGrid::removeRow(const std::string& id) {
    auto it = std::remove_if(m_gridData->rows.begin(), m_gridData->rows.end(),
        [&id](const DataGridRow& row) { return row.id == id; });
    m_gridData->rows.erase(it, m_gridData->rows.end());
    
    // Remove from selection
    auto selIt = std::find(m_gridData->selectedRowIds.begin(), m_gridData->selectedRowIds.end(), id);
    if (selIt != m_gridData->selectedRowIds.end()) {
        m_gridData->selectedRowIds.erase(selIt);
    }
    
    m_gridData->invalidateCache();
    return *this;
}

DataGrid& DataGrid::clearRows() {
    m_gridData->rows.clear();
    m_gridData->selectedRowIds.clear();
    m_gridData->invalidateCache();
    return *this;
}

const std::vector<DataGridRow>& DataGrid::getRows() const {
    return m_gridData->rows;
}

std::vector<DataGridRow> DataGrid::getDisplayedRows() const {
    m_gridData->updateCache();
    std::vector<DataGridRow> result;
    result.reserve(m_gridData->displayedIndices.size());
    for (size_t idx : m_gridData->displayedIndices) {
        result.push_back(m_gridData->rows[idx]);
    }
    return result;
}

DataGridRow* DataGrid::getRow(const std::string& id) {
    for (auto& row : m_gridData->rows) {
        if (row.id == id) return &row;
    }
    return nullptr;
}

size_t DataGrid::getRowCount() const {
    return m_gridData->rows.size();
}

size_t DataGrid::getFilteredRowCount() const {
    m_gridData->updateCache();
    return m_gridData->displayedIndices.size();
}

// Sorting
DataGrid& DataGrid::sortBy(const std::string& columnId, SortDirection direction) {
    // Clear previous sort indicator
    for (auto& col : m_gridData->columns) {
        col.sortDirection = SortDirection::None;
    }
    
    m_gridData->sortColumnId = columnId;
    m_gridData->sortDirection = direction;
    
    // Set sort indicator on column
    if (auto* col = getColumn(columnId)) {
        col->sortDirection = direction;
    }
    
    m_gridData->invalidateCache();
    
    if (m_gridData->onSortCallback) {
        m_gridData->onSortCallback(columnId, direction);
    }
    
    return *this;
}

DataGrid& DataGrid::clearSort() {
    for (auto& col : m_gridData->columns) {
        col.sortDirection = SortDirection::None;
    }
    m_gridData->sortColumnId.clear();
    m_gridData->sortDirection = SortDirection::None;
    m_gridData->invalidateCache();
    return *this;
}

const std::string& DataGrid::getSortColumn() const {
    return m_gridData->sortColumnId;
}

SortDirection DataGrid::getSortDirection() const {
    return m_gridData->sortDirection;
}

// Filtering
DataGrid& DataGrid::setFilter(const std::string& columnId, const std::string& filterText) {
    // Remove existing filter for this column
    auto it = std::remove_if(m_gridData->filters.begin(), m_gridData->filters.end(),
        [&columnId](const DataGridFilter& f) { return f.columnId == columnId; });
    m_gridData->filters.erase(it, m_gridData->filters.end());
    
    if (!filterText.empty()) {
        DataGridFilter filter;
        filter.columnId = columnId;
        filter.filterText = filterText;
        m_gridData->filters.push_back(filter);
    }
    
    m_gridData->invalidateCache();
    return *this;
}

DataGrid& DataGrid::setFilter(const std::string& columnId, std::function<bool(const CellValue&)> filter) {
    auto it = std::remove_if(m_gridData->filters.begin(), m_gridData->filters.end(),
        [&columnId](const DataGridFilter& f) { return f.columnId == columnId; });
    m_gridData->filters.erase(it, m_gridData->filters.end());
    
    if (filter) {
        DataGridFilter f;
        f.columnId = columnId;
        f.customFilter = std::move(filter);
        m_gridData->filters.push_back(f);
    }
    
    m_gridData->invalidateCache();
    return *this;
}

DataGrid& DataGrid::clearFilter(const std::string& columnId) {
    auto it = std::remove_if(m_gridData->filters.begin(), m_gridData->filters.end(),
        [&columnId](const DataGridFilter& f) { return f.columnId == columnId; });
    m_gridData->filters.erase(it, m_gridData->filters.end());
    m_gridData->invalidateCache();
    return *this;
}

DataGrid& DataGrid::clearAllFilters() {
    m_gridData->filters.clear();
    m_gridData->invalidateCache();
    return *this;
}

const std::vector<DataGridFilter>& DataGrid::getFilters() const {
    return m_gridData->filters;
}


// Selection
DataGrid& DataGrid::multiSelect(bool enabled) {
    m_gridData->multiSelectEnabled = enabled;
    if (!enabled && m_gridData->selectedRowIds.size() > 1) {
        // Keep only first selection
        m_gridData->selectedRowIds.resize(1);
    }
    return *this;
}

bool DataGrid::isMultiSelect() const {
    return m_gridData->multiSelectEnabled;
}

DataGrid& DataGrid::selectRow(const std::string& id, bool addToSelection) {
    if (!addToSelection || !m_gridData->multiSelectEnabled) {
        m_gridData->selectedRowIds.clear();
    }
    
    // Check if already selected
    auto it = std::find(m_gridData->selectedRowIds.begin(), m_gridData->selectedRowIds.end(), id);
    if (it == m_gridData->selectedRowIds.end()) {
        m_gridData->selectedRowIds.push_back(id);
    }
    
    // Update row selection state
    for (auto& row : m_gridData->rows) {
        row.selected = std::find(m_gridData->selectedRowIds.begin(), 
                                  m_gridData->selectedRowIds.end(), 
                                  row.id) != m_gridData->selectedRowIds.end();
    }
    
    if (m_gridData->onSelectionChangeCallback) {
        m_gridData->onSelectionChangeCallback(m_gridData->selectedRowIds);
    }
    
    return *this;
}

DataGrid& DataGrid::deselectRow(const std::string& id) {
    auto it = std::find(m_gridData->selectedRowIds.begin(), m_gridData->selectedRowIds.end(), id);
    if (it != m_gridData->selectedRowIds.end()) {
        m_gridData->selectedRowIds.erase(it);
        
        if (auto* row = getRow(id)) {
            row->selected = false;
        }
        
        if (m_gridData->onSelectionChangeCallback) {
            m_gridData->onSelectionChangeCallback(m_gridData->selectedRowIds);
        }
    }
    return *this;
}

DataGrid& DataGrid::clearSelection() {
    for (auto& row : m_gridData->rows) {
        row.selected = false;
    }
    m_gridData->selectedRowIds.clear();
    
    if (m_gridData->onSelectionChangeCallback) {
        m_gridData->onSelectionChangeCallback(m_gridData->selectedRowIds);
    }
    
    return *this;
}

std::vector<std::string> DataGrid::getSelectedRowIds() const {
    return m_gridData->selectedRowIds;
}

std::vector<const DataGridRow*> DataGrid::getSelectedRows() const {
    std::vector<const DataGridRow*> result;
    for (const auto& id : m_gridData->selectedRowIds) {
        for (const auto& row : m_gridData->rows) {
            if (row.id == id) {
                result.push_back(&row);
                break;
            }
        }
    }
    return result;
}

// Virtual Scrolling
DataGrid& DataGrid::rowHeight(float height) {
    m_gridData->rowHeight = height;
    return *this;
}

float DataGrid::getRowHeight() const {
    return m_gridData->rowHeight;
}

DataGrid& DataGrid::headerHeight(float height) {
    m_gridData->headerHeight = height;
    return *this;
}

float DataGrid::getHeaderHeight() const {
    return m_gridData->headerHeight;
}

float DataGrid::getScrollOffset() const {
    return m_gridData->scrollOffset;
}

DataGrid& DataGrid::scrollTo(float offset) {
    m_gridData->updateCache();
    float maxScroll = std::max(0.0f, 
        static_cast<float>(m_gridData->displayedIndices.size()) * m_gridData->rowHeight - 
        (getHeight() - m_gridData->headerHeight));
    m_gridData->scrollOffset = std::clamp(offset, 0.0f, maxScroll);
    return *this;
}

DataGrid& DataGrid::scrollToRow(const std::string& id) {
    m_gridData->updateCache();
    
    for (size_t i = 0; i < m_gridData->displayedIndices.size(); ++i) {
        if (m_gridData->rows[m_gridData->displayedIndices[i]].id == id) {
            float rowTop = static_cast<float>(i) * m_gridData->rowHeight;
            scrollTo(rowTop);
            break;
        }
    }
    
    return *this;
}

void DataGrid::getVisibleRowRange(int& startIndex, int& endIndex) const {
    m_gridData->updateCache();
    
    float viewHeight = getHeight() - m_gridData->headerHeight;
    startIndex = static_cast<int>(m_gridData->scrollOffset / m_gridData->rowHeight);
    endIndex = static_cast<int>((m_gridData->scrollOffset + viewHeight) / m_gridData->rowHeight) + 1;
    
    startIndex = std::max(0, startIndex);
    endIndex = std::min(static_cast<int>(m_gridData->displayedIndices.size()), endIndex);
}

// Appearance
DataGrid& DataGrid::headerColor(const Color& color) {
    m_gridData->headerColor = color;
    return *this;
}

const Color& DataGrid::getHeaderColor() const {
    return m_gridData->headerColor;
}

DataGrid& DataGrid::alternatingRowColors(const Color& even, const Color& odd) {
    m_gridData->evenRowColor = even;
    m_gridData->oddRowColor = odd;
    return *this;
}

DataGrid& DataGrid::hoverColor(const Color& color) {
    m_gridData->hoverColor = color;
    return *this;
}

const Color& DataGrid::getHoverColor() const {
    return m_gridData->hoverColor;
}

DataGrid& DataGrid::selectionColor(const Color& color) {
    m_gridData->selectionColor = color;
    return *this;
}

const Color& DataGrid::getSelectionColor() const {
    return m_gridData->selectionColor;
}

DataGrid& DataGrid::showGridLines(bool show) {
    m_gridData->gridLinesVisible = show;
    return *this;
}

bool DataGrid::hasGridLines() const {
    return m_gridData->gridLinesVisible;
}

// Custom Rendering
DataGrid& DataGrid::cellRenderer(CellRenderer renderer) {
    m_gridData->customCellRenderer = std::move(renderer);
    return *this;
}

DataGrid& DataGrid::rowRenderer(RowRenderer renderer) {
    m_gridData->customRowRenderer = std::move(renderer);
    return *this;
}

// Callbacks
DataGrid& DataGrid::onSort(std::function<void(const std::string&, SortDirection)> callback) {
    m_gridData->onSortCallback = std::move(callback);
    return *this;
}

DataGrid& DataGrid::onSelectionChange(std::function<void(const std::vector<std::string>&)> callback) {
    m_gridData->onSelectionChangeCallback = std::move(callback);
    return *this;
}

DataGrid& DataGrid::onRowDoubleClick(std::function<void(const DataGridRow&)> callback) {
    m_gridData->onRowDoubleClickCallback = std::move(callback);
    return *this;
}

DataGrid& DataGrid::onColumnResize(std::function<void(const std::string&, float)> callback) {
    m_gridData->onColumnResizeCallback = std::move(callback);
    return *this;
}

} // namespace KillerGK
