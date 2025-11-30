/**
 * @file Layout.cpp
 * @brief Layout system implementation
 */

#include "KillerGK/layout/Layout.hpp"
#include "KillerGK/platform/Platform.hpp"
#include <algorithm>
#include <sstream>
#include <cmath>

namespace KillerGK {

// =============================================================================
// FlexImpl Implementation
// =============================================================================

FlexImpl::FlexImpl() = default;

Size FlexImpl::layout(const LayoutConstraints& constraints) {
    auto startTime = std::chrono::high_resolution_clock::now();

    m_childBounds.clear();
    m_childBounds.resize(m_children.size());

    if (m_children.empty()) {
        m_needsLayout = false;
        auto endTime = std::chrono::high_resolution_clock::now();
        m_lastComputeTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        return Size(constraints.constrainWidth(0), constraints.constrainHeight(0));
    }

    const bool isRow = (m_direction == FlexDirection::Row || m_direction == FlexDirection::RowReverse);
    const bool isReverse = (m_direction == FlexDirection::RowReverse || m_direction == FlexDirection::ColumnReverse);

    // Calculate total content size
    float totalMainSize = 0.0f;
    float maxCrossSize = 0.0f;

    for (const auto* child : m_children) {
        if (!child) continue;
        float childWidth = child->getWidth();
        float childHeight = child->getHeight();

        // Apply min/max constraints from child
        childWidth = std::max(child->getMinWidth(), std::min(childWidth, child->getMaxWidth()));
        childHeight = std::max(child->getMinHeight(), std::min(childHeight, child->getMaxHeight()));

        if (isRow) {
            totalMainSize += childWidth;
            maxCrossSize = std::max(maxCrossSize, childHeight);
        } else {
            totalMainSize += childHeight;
            maxCrossSize = std::max(maxCrossSize, childWidth);
        }
    }

    // Add gaps
    if (m_children.size() > 1) {
        totalMainSize += m_gap * (m_children.size() - 1);
    }

    // Determine container size
    float containerMainSize = isRow ? constraints.constrainWidth(totalMainSize) : constraints.constrainHeight(totalMainSize);
    float containerCrossSize = isRow ? constraints.constrainHeight(maxCrossSize) : constraints.constrainWidth(maxCrossSize);

    // Calculate spacing based on justify-content
    float extraSpace = containerMainSize - totalMainSize;
    float startOffset = 0.0f;
    float itemSpacing = m_gap;

    switch (m_justify) {
        case JustifyContent::Start:
            startOffset = 0.0f;
            break;
        case JustifyContent::End:
            startOffset = extraSpace;
            break;
        case JustifyContent::Center:
            startOffset = extraSpace / 2.0f;
            break;
        case JustifyContent::SpaceBetween:
            if (m_children.size() > 1) {
                itemSpacing = extraSpace / (m_children.size() - 1) + m_gap;
            }
            break;
        case JustifyContent::SpaceAround:
            if (!m_children.empty()) {
                float space = extraSpace / m_children.size();
                startOffset = space / 2.0f;
                itemSpacing = space + m_gap;
            }
            break;
        case JustifyContent::SpaceEvenly:
            if (!m_children.empty()) {
                float space = extraSpace / (m_children.size() + 1);
                startOffset = space;
                itemSpacing = space + m_gap;
            }
            break;
    }

    // Position children
    float mainPos = isReverse ? containerMainSize : startOffset;
    
    for (size_t i = 0; i < m_children.size(); ++i) {
        const auto* child = m_children[i];
        if (!child) continue;

        float childWidth = std::max(child->getMinWidth(), std::min(child->getWidth(), child->getMaxWidth()));
        float childHeight = std::max(child->getMinHeight(), std::min(child->getHeight(), child->getMaxHeight()));

        float childMainSize = isRow ? childWidth : childHeight;
        float childCrossSize = isRow ? childHeight : childWidth;

        // Calculate cross-axis position based on align-items
        float crossPos = 0.0f;
        float finalCrossSize = childCrossSize;

        switch (m_align) {
            case AlignItems::Start:
                crossPos = 0.0f;
                break;
            case AlignItems::End:
                crossPos = containerCrossSize - childCrossSize;
                break;
            case AlignItems::Center:
                crossPos = (containerCrossSize - childCrossSize) / 2.0f;
                break;
            case AlignItems::Stretch:
                crossPos = 0.0f;
                finalCrossSize = containerCrossSize;
                break;
            case AlignItems::Baseline:
                crossPos = 0.0f; // Simplified baseline alignment
                break;
        }

        // Set child bounds
        if (isReverse) {
            mainPos -= childMainSize;
        }

        if (isRow) {
            m_childBounds[i] = Rect(
                m_bounds.x + mainPos,
                m_bounds.y + crossPos,
                childWidth,
                (m_align == AlignItems::Stretch) ? finalCrossSize : childHeight
            );
        } else {
            m_childBounds[i] = Rect(
                m_bounds.x + crossPos,
                m_bounds.y + mainPos,
                (m_align == AlignItems::Stretch) ? finalCrossSize : childWidth,
                childHeight
            );
        }

        if (isReverse) {
            mainPos -= itemSpacing;
        } else {
            mainPos += childMainSize + itemSpacing;
        }
    }

    m_needsLayout = false;
    auto endTime = std::chrono::high_resolution_clock::now();
    m_lastComputeTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

    return Size(
        isRow ? containerMainSize : containerCrossSize,
        isRow ? containerCrossSize : containerMainSize
    );
}

Rect FlexImpl::getChildBounds(size_t index) const {
    if (index < m_childBounds.size()) {
        return m_childBounds[index];
    }
    return Rect();
}

size_t FlexImpl::getChildCount() const {
    return m_children.size();
}

void FlexImpl::invalidate() {
    m_needsLayout = true;
}

bool FlexImpl::needsLayout() const {
    return m_needsLayout;
}

std::chrono::microseconds FlexImpl::getLastComputeTime() const {
    return m_lastComputeTime;
}

void FlexImpl::setDirection(FlexDirection dir) {
    if (m_direction != dir) {
        m_direction = dir;
        m_needsLayout = true;
    }
}

void FlexImpl::setJustify(JustifyContent justify) {
    if (m_justify != justify) {
        m_justify = justify;
        m_needsLayout = true;
    }
}

void FlexImpl::setAlign(AlignItems align) {
    if (m_align != align) {
        m_align = align;
        m_needsLayout = true;
    }
}

void FlexImpl::setWrap(FlexWrap wrap) {
    if (m_wrap != wrap) {
        m_wrap = wrap;
        m_needsLayout = true;
    }
}

void FlexImpl::setGap(float gap) {
    if (m_gap != gap) {
        m_gap = gap;
        m_needsLayout = true;
    }
}

void FlexImpl::setChildren(const std::vector<Widget*>& children) {
    m_children = children;
    m_needsLayout = true;
}

void FlexImpl::setBounds(const Rect& bounds) {
    if (!(m_bounds == bounds)) {
        m_bounds = bounds;
        m_needsLayout = true;
    }
}

// =============================================================================
// GridImpl Implementation
// =============================================================================

GridImpl::GridImpl() = default;

std::vector<float> GridImpl::parseTemplate(const std::string& tmpl, float totalSize, int count) const {
    std::vector<float> sizes;
    
    if (tmpl.empty()) {
        // Equal distribution
        float size = totalSize / count;
        sizes.resize(count, size);
        return sizes;
    }

    std::istringstream iss(tmpl);
    std::string token;
    std::vector<std::pair<float, bool>> parsed; // value, isFr
    float totalFr = 0.0f;
    float totalFixed = 0.0f;

    while (iss >> token) {
        if (token.back() == 'r' && token.size() > 2 && token[token.size()-2] == 'f') {
            // Fractional unit (e.g., "1fr", "2fr")
            float fr = std::stof(token.substr(0, token.size() - 2));
            parsed.emplace_back(fr, true);
            totalFr += fr;
        } else if (token.back() == 'x' && token.size() > 2 && token[token.size()-2] == 'p') {
            // Pixel value (e.g., "100px")
            float px = std::stof(token.substr(0, token.size() - 2));
            parsed.emplace_back(px, false);
            totalFixed += px;
        } else {
            // Plain number (treat as pixels)
            float px = std::stof(token);
            parsed.emplace_back(px, false);
            totalFixed += px;
        }
    }

    float remainingSpace = totalSize - totalFixed;
    float frUnit = (totalFr > 0) ? (remainingSpace / totalFr) : 0.0f;

    for (const auto& [value, isFr] : parsed) {
        if (isFr) {
            sizes.push_back(value * frUnit);
        } else {
            sizes.push_back(value);
        }
    }

    // Pad with equal sizes if needed
    while (sizes.size() < static_cast<size_t>(count)) {
        sizes.push_back(totalSize / count);
    }

    return sizes;
}

Size GridImpl::layout(const LayoutConstraints& constraints) {
    auto startTime = std::chrono::high_resolution_clock::now();

    m_childBounds.clear();
    m_childBounds.resize(m_children.size());

    if (m_children.empty()) {
        m_needsLayout = false;
        auto endTime = std::chrono::high_resolution_clock::now();
        m_lastComputeTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        return Size(constraints.constrainWidth(0), constraints.constrainHeight(0));
    }

    // Calculate available space
    float availableWidth = m_bounds.width > 0 ? m_bounds.width : constraints.maxWidth;
    float availableHeight = m_bounds.height > 0 ? m_bounds.height : constraints.maxHeight;

    // Account for gaps
    float totalColumnGaps = (m_columns > 1) ? m_columnGap * (m_columns - 1) : 0.0f;
    float totalRowGaps = (m_rows > 1) ? m_rowGap * (m_rows - 1) : 0.0f;

    float contentWidth = availableWidth - totalColumnGaps;
    float contentHeight = availableHeight - totalRowGaps;

    // Parse templates or use equal distribution
    std::vector<float> columnSizes = parseTemplate(m_templateColumns, contentWidth, m_columns);
    std::vector<float> rowSizes = parseTemplate(m_templateRows, contentHeight, m_rows);

    // Position children in grid cells
    for (size_t i = 0; i < m_children.size(); ++i) {
        int col = static_cast<int>(i) % m_columns;
        int row = static_cast<int>(i) / m_columns;

        if (row >= m_rows) break; // Exceeded grid capacity

        // Calculate position
        float x = m_bounds.x;
        for (int c = 0; c < col; ++c) {
            x += columnSizes[c] + m_columnGap;
        }

        float y = m_bounds.y;
        for (int r = 0; r < row; ++r) {
            y += rowSizes[r] + m_rowGap;
        }

        m_childBounds[i] = Rect(x, y, columnSizes[col], rowSizes[row]);
    }

    m_needsLayout = false;
    auto endTime = std::chrono::high_resolution_clock::now();
    m_lastComputeTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

    return Size(
        constraints.constrainWidth(availableWidth),
        constraints.constrainHeight(availableHeight)
    );
}

Rect GridImpl::getChildBounds(size_t index) const {
    if (index < m_childBounds.size()) {
        return m_childBounds[index];
    }
    return Rect();
}

size_t GridImpl::getChildCount() const {
    return m_children.size();
}

void GridImpl::invalidate() {
    m_needsLayout = true;
}

bool GridImpl::needsLayout() const {
    return m_needsLayout;
}

std::chrono::microseconds GridImpl::getLastComputeTime() const {
    return m_lastComputeTime;
}

void GridImpl::setColumns(int count) {
    if (m_columns != count) {
        m_columns = std::max(1, count);
        m_needsLayout = true;
    }
}

void GridImpl::setRows(int count) {
    if (m_rows != count) {
        m_rows = std::max(1, count);
        m_needsLayout = true;
    }
}

void GridImpl::setColumnGap(float gap) {
    if (m_columnGap != gap) {
        m_columnGap = gap;
        m_needsLayout = true;
    }
}

void GridImpl::setRowGap(float gap) {
    if (m_rowGap != gap) {
        m_rowGap = gap;
        m_needsLayout = true;
    }
}

void GridImpl::setTemplateColumns(const std::string& tmpl) {
    if (m_templateColumns != tmpl) {
        m_templateColumns = tmpl;
        m_needsLayout = true;
    }
}

void GridImpl::setTemplateRows(const std::string& tmpl) {
    if (m_templateRows != tmpl) {
        m_templateRows = tmpl;
        m_needsLayout = true;
    }
}

void GridImpl::setChildren(const std::vector<Widget*>& children) {
    m_children = children;
    m_needsLayout = true;
}

void GridImpl::setBounds(const Rect& bounds) {
    if (!(m_bounds == bounds)) {
        m_bounds = bounds;
        m_needsLayout = true;
    }
}


// =============================================================================
// StackImpl Implementation
// =============================================================================

StackImpl::StackImpl() = default;

Size StackImpl::layout(const LayoutConstraints& constraints) {
    auto startTime = std::chrono::high_resolution_clock::now();

    m_childBounds.clear();
    m_childBounds.resize(m_children.size());

    if (m_children.empty()) {
        m_needsLayout = false;
        auto endTime = std::chrono::high_resolution_clock::now();
        m_lastComputeTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        return Size(constraints.constrainWidth(0), constraints.constrainHeight(0));
    }

    // Stack layout: all children are positioned at the same location
    // Children are rendered in order (later children on top)
    float maxWidth = 0.0f;
    float maxHeight = 0.0f;

    for (size_t i = 0; i < m_children.size(); ++i) {
        const auto* child = m_children[i];
        if (!child) continue;

        float childWidth = std::max(child->getMinWidth(), std::min(child->getWidth(), child->getMaxWidth()));
        float childHeight = std::max(child->getMinHeight(), std::min(child->getHeight(), child->getMaxHeight()));

        // All children start at the same position (top-left of container)
        m_childBounds[i] = Rect(m_bounds.x, m_bounds.y, childWidth, childHeight);

        maxWidth = std::max(maxWidth, childWidth);
        maxHeight = std::max(maxHeight, childHeight);
    }

    m_needsLayout = false;
    auto endTime = std::chrono::high_resolution_clock::now();
    m_lastComputeTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

    return Size(
        constraints.constrainWidth(maxWidth),
        constraints.constrainHeight(maxHeight)
    );
}

Rect StackImpl::getChildBounds(size_t index) const {
    if (index < m_childBounds.size()) {
        return m_childBounds[index];
    }
    return Rect();
}

size_t StackImpl::getChildCount() const {
    return m_children.size();
}

void StackImpl::invalidate() {
    m_needsLayout = true;
}

bool StackImpl::needsLayout() const {
    return m_needsLayout;
}

std::chrono::microseconds StackImpl::getLastComputeTime() const {
    return m_lastComputeTime;
}

void StackImpl::setChildren(const std::vector<Widget*>& children) {
    m_children = children;
    m_needsLayout = true;
}

void StackImpl::setBounds(const Rect& bounds) {
    if (!(m_bounds == bounds)) {
        m_bounds = bounds;
        m_needsLayout = true;
    }
}

// =============================================================================
// AbsoluteImpl Implementation
// =============================================================================

AbsoluteImpl::AbsoluteImpl() = default;

Size AbsoluteImpl::layout(const LayoutConstraints& constraints) {
    auto startTime = std::chrono::high_resolution_clock::now();

    m_childBounds.clear();
    m_childBounds.resize(m_children.size());

    if (m_children.empty()) {
        m_needsLayout = false;
        auto endTime = std::chrono::high_resolution_clock::now();
        m_lastComputeTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        return Size(constraints.constrainWidth(0), constraints.constrainHeight(0));
    }

    // Absolute layout: children are positioned at their specified x, y coordinates
    // relative to the container's origin
    float maxRight = 0.0f;
    float maxBottom = 0.0f;

    for (size_t i = 0; i < m_children.size(); ++i) {
        const auto* child = m_children[i];
        if (!child) continue;

        // Get child's position from custom properties or margin
        float childX = child->getPropertyFloat("x", child->getMargin().left);
        float childY = child->getPropertyFloat("y", child->getMargin().top);
        float childWidth = std::max(child->getMinWidth(), std::min(child->getWidth(), child->getMaxWidth()));
        float childHeight = std::max(child->getMinHeight(), std::min(child->getHeight(), child->getMaxHeight()));

        // Position relative to container
        m_childBounds[i] = Rect(
            m_bounds.x + childX,
            m_bounds.y + childY,
            childWidth,
            childHeight
        );

        maxRight = std::max(maxRight, childX + childWidth);
        maxBottom = std::max(maxBottom, childY + childHeight);
    }

    m_needsLayout = false;
    auto endTime = std::chrono::high_resolution_clock::now();
    m_lastComputeTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

    return Size(
        constraints.constrainWidth(maxRight),
        constraints.constrainHeight(maxBottom)
    );
}

Rect AbsoluteImpl::getChildBounds(size_t index) const {
    if (index < m_childBounds.size()) {
        return m_childBounds[index];
    }
    return Rect();
}

size_t AbsoluteImpl::getChildCount() const {
    return m_children.size();
}

void AbsoluteImpl::invalidate() {
    m_needsLayout = true;
}

bool AbsoluteImpl::needsLayout() const {
    return m_needsLayout;
}

std::chrono::microseconds AbsoluteImpl::getLastComputeTime() const {
    return m_lastComputeTime;
}

void AbsoluteImpl::setChildren(const std::vector<Widget*>& children) {
    m_children = children;
    m_needsLayout = true;
}

void AbsoluteImpl::setBounds(const Rect& bounds) {
    if (!(m_bounds == bounds)) {
        m_bounds = bounds;
        m_needsLayout = true;
    }
}

// =============================================================================
// Flex Builder Implementation
// =============================================================================

Flex::Flex() : m_impl(std::make_shared<FlexImpl>()) {}

Flex Flex::create() {
    return Flex();
}

Flex& Flex::direction(FlexDirection dir) {
    m_impl->setDirection(dir);
    return *this;
}

Flex& Flex::justify(JustifyContent justify) {
    m_impl->setJustify(justify);
    return *this;
}

Flex& Flex::align(AlignItems align) {
    m_impl->setAlign(align);
    return *this;
}

Flex& Flex::wrap(FlexWrap wrap) {
    m_impl->setWrap(wrap);
    return *this;
}

Flex& Flex::gap(float gap) {
    m_impl->setGap(gap);
    return *this;
}

Flex& Flex::children(std::initializer_list<Widget*> widgets) {
    m_impl->setChildren(std::vector<Widget*>(widgets));
    return *this;
}

FlexHandle Flex::build() {
    return m_impl;
}

// =============================================================================
// Grid Builder Implementation
// =============================================================================

Grid::Grid() : m_impl(std::make_shared<GridImpl>()) {}

Grid Grid::create() {
    return Grid();
}

Grid& Grid::columns(int count) {
    m_impl->setColumns(count);
    return *this;
}

Grid& Grid::rows(int count) {
    m_impl->setRows(count);
    return *this;
}

Grid& Grid::columnGap(float gap) {
    m_impl->setColumnGap(gap);
    return *this;
}

Grid& Grid::rowGap(float gap) {
    m_impl->setRowGap(gap);
    return *this;
}

Grid& Grid::templateColumns(const std::string& tmpl) {
    m_impl->setTemplateColumns(tmpl);
    return *this;
}

Grid& Grid::templateRows(const std::string& tmpl) {
    m_impl->setTemplateRows(tmpl);
    return *this;
}

Grid& Grid::children(std::initializer_list<Widget*> widgets) {
    m_impl->setChildren(std::vector<Widget*>(widgets));
    return *this;
}

GridHandle Grid::build() {
    return m_impl;
}

// =============================================================================
// Stack Builder Implementation
// =============================================================================

Stack::Stack() : m_impl(std::make_shared<StackImpl>()) {}

Stack Stack::create() {
    return Stack();
}

Stack& Stack::children(std::initializer_list<Widget*> widgets) {
    m_impl->setChildren(std::vector<Widget*>(widgets));
    return *this;
}

StackHandle Stack::build() {
    return m_impl;
}

// =============================================================================
// Absolute Builder Implementation
// =============================================================================

Absolute::Absolute() : m_impl(std::make_shared<AbsoluteImpl>()) {}

Absolute Absolute::create() {
    return Absolute();
}

Absolute& Absolute::children(std::initializer_list<Widget*> widgets) {
    m_impl->setChildren(std::vector<Widget*>(widgets));
    return *this;
}

AbsoluteHandle Absolute::build() {
    return m_impl;
}

// =============================================================================
// LayoutManager Implementation
// =============================================================================

LayoutManager& LayoutManager::instance() {
    static LayoutManager instance;
    return instance;
}

void LayoutManager::registerLayout(ILayout* layout) {
    if (layout && std::find(m_layouts.begin(), m_layouts.end(), layout) == m_layouts.end()) {
        m_layouts.push_back(layout);
    }
}

void LayoutManager::unregisterLayout(ILayout* layout) {
    auto it = std::find(m_layouts.begin(), m_layouts.end(), layout);
    if (it != m_layouts.end()) {
        m_layouts.erase(it);
    }
}

void LayoutManager::onWindowResize(int width, int height) {
    m_windowWidth = width;
    m_windowHeight = height;

    // Invalidate all layouts
    for (auto* layout : m_layouts) {
        if (layout) {
            layout->invalidate();
        }
    }

    // Recalculate all layouts
    recalculateAll();

    // Notify callback
    if (m_resizeCallback) {
        m_resizeCallback(width, height);
    }
}

std::chrono::microseconds LayoutManager::recalculateAll() {
    auto startTime = std::chrono::high_resolution_clock::now();

    LayoutConstraints constraints = LayoutConstraints::loose(
        static_cast<float>(m_windowWidth),
        static_cast<float>(m_windowHeight)
    );

    for (auto* layout : m_layouts) {
        if (layout && layout->needsLayout()) {
            layout->layout(constraints);
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    m_lastRecalculationTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

    return m_lastRecalculationTime;
}

void LayoutManager::setResizeCallback(ResizeCallback callback) {
    m_resizeCallback = std::move(callback);
}

std::chrono::microseconds LayoutManager::getLastRecalculationTime() const {
    return m_lastRecalculationTime;
}

bool LayoutManager::isWithinTargetTime() const {
    // Target is 16ms (60 FPS)
    return m_lastRecalculationTime.count() < TARGET_RECALC_TIME_US;
}

// =============================================================================
// Helper Functions
// =============================================================================

void connectWindowToLayoutManager(IPlatformWindow* window) {
    if (!window) return;
    
    // Set the resize callback to trigger layout recalculation
    window->setResizeCallback([](int width, int height) {
        LayoutManager::instance().onWindowResize(width, height);
    });
}

} // namespace KillerGK
