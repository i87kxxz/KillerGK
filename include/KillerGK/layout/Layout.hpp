/**
 * @file Layout.hpp
 * @brief Layout system for KillerGK
 */

#pragma once

#include "../widgets/Widget.hpp"
#include "../core/Types.hpp"
#include <vector>
#include <initializer_list>
#include <chrono>
#include <functional>
#include <limits>

namespace KillerGK {

/**
 * @struct LayoutConstraints
 * @brief Constraints for layout calculation
 * 
 * Defines the minimum and maximum bounds within which a widget
 * can be sized during layout calculation.
 */
struct LayoutConstraints {
    float minWidth = 0.0f;
    float maxWidth = std::numeric_limits<float>::max();
    float minHeight = 0.0f;
    float maxHeight = std::numeric_limits<float>::max();

    constexpr LayoutConstraints() = default;
    constexpr LayoutConstraints(float minW, float maxW, float minH, float maxH)
        : minWidth(minW), maxWidth(maxW), minHeight(minH), maxHeight(maxH) {}

    /**
     * @brief Create tight constraints (exact size)
     */
    static constexpr LayoutConstraints tight(float width, float height) {
        return LayoutConstraints(width, width, height, height);
    }

    /**
     * @brief Create loose constraints (0 to max)
     */
    static constexpr LayoutConstraints loose(float maxWidth, float maxHeight) {
        return LayoutConstraints(0.0f, maxWidth, 0.0f, maxHeight);
    }

    /**
     * @brief Create unbounded constraints
     */
    static constexpr LayoutConstraints unbounded() {
        return LayoutConstraints();
    }

    /**
     * @brief Constrain a width value to be within bounds
     */
    [[nodiscard]] constexpr float constrainWidth(float width) const {
        if (width < minWidth) return minWidth;
        if (width > maxWidth) return maxWidth;
        return width;
    }

    /**
     * @brief Constrain a height value to be within bounds
     */
    [[nodiscard]] constexpr float constrainHeight(float height) const {
        if (height < minHeight) return minHeight;
        if (height > maxHeight) return maxHeight;
        return height;
    }

    /**
     * @brief Constrain a size to be within bounds
     */
    [[nodiscard]] constexpr Size constrain(const Size& size) const {
        return Size(constrainWidth(size.width), constrainHeight(size.height));
    }

    /**
     * @brief Check if constraints are satisfied by a size
     */
    [[nodiscard]] constexpr bool isSatisfiedBy(const Size& size) const {
        return size.width >= minWidth && size.width <= maxWidth &&
               size.height >= minHeight && size.height <= maxHeight;
    }

    /**
     * @brief Check if constraints are tight (exact size)
     */
    [[nodiscard]] constexpr bool isTight() const {
        return minWidth == maxWidth && minHeight == maxHeight;
    }

    /**
     * @brief Check if constraints are bounded
     */
    [[nodiscard]] constexpr bool isBounded() const {
        return maxWidth < std::numeric_limits<float>::max() &&
               maxHeight < std::numeric_limits<float>::max();
    }

    bool operator==(const LayoutConstraints& other) const = default;
};

/**
 * @struct LayoutResult
 * @brief Result of a layout calculation
 */
struct LayoutResult {
    Rect bounds;                    ///< Computed bounds for the widget
    bool constraintsSatisfied;      ///< Whether all constraints were satisfied
    std::chrono::microseconds computeTime{0}; ///< Time taken to compute layout

    LayoutResult() : constraintsSatisfied(true) {}
    LayoutResult(const Rect& b, bool satisfied = true) 
        : bounds(b), constraintsSatisfied(satisfied) {}
};

/**
 * @class ILayout
 * @brief Interface for layout algorithms
 * 
 * All layout containers implement this interface to provide
 * consistent layout calculation behavior.
 */
class ILayout {
public:
    virtual ~ILayout() = default;

    /**
     * @brief Calculate layout for all children
     * @param constraints Parent constraints to respect
     * @return Computed size of this layout
     */
    virtual Size layout(const LayoutConstraints& constraints) = 0;

    /**
     * @brief Get the computed bounds for a child at index
     * @param index Child index
     * @return Computed bounds or empty rect if invalid
     */
    virtual Rect getChildBounds(size_t index) const = 0;

    /**
     * @brief Get number of children in this layout
     */
    virtual size_t getChildCount() const = 0;

    /**
     * @brief Mark layout as needing recalculation
     */
    virtual void invalidate() = 0;

    /**
     * @brief Check if layout needs recalculation
     */
    virtual bool needsLayout() const = 0;

    /**
     * @brief Get the last layout computation time
     */
    virtual std::chrono::microseconds getLastComputeTime() const = 0;
};

/**
 * @enum FlexDirection
 * @brief Direction for flex layout
 */
enum class FlexDirection {
    Row,
    Column,
    RowReverse,
    ColumnReverse
};

/**
 * @enum JustifyContent
 * @brief Main axis alignment
 */
enum class JustifyContent {
    Start,
    End,
    Center,
    SpaceBetween,
    SpaceAround,
    SpaceEvenly
};

/**
 * @enum AlignItems
 * @brief Cross axis alignment
 */
enum class AlignItems {
    Start,
    End,
    Center,
    Stretch,
    Baseline
};

/**
 * @enum FlexWrap
 * @brief Flex wrap behavior
 */
enum class FlexWrap {
    NoWrap,
    Wrap,
    WrapReverse
};

// Forward declarations for implementation classes
class FlexImpl;
class GridImpl;
class StackImpl;
class AbsoluteImpl;

/**
 * @brief Handle to a built flex container
 */
using FlexHandle = std::shared_ptr<FlexImpl>;

/**
 * @brief Handle to a built grid container
 */
using GridHandle = std::shared_ptr<GridImpl>;

/**
 * @brief Handle to a built stack container
 */
using StackHandle = std::shared_ptr<StackImpl>;

/**
 * @brief Handle to a built absolute container
 */
using AbsoluteHandle = std::shared_ptr<AbsoluteImpl>;



/**
 * @class FlexImpl
 * @brief Implementation class for Flex layout
 */
class FlexImpl : public ILayout {
public:
    FlexImpl();

    // ILayout interface
    Size layout(const LayoutConstraints& constraints) override;
    Rect getChildBounds(size_t index) const override;
    size_t getChildCount() const override;
    void invalidate() override;
    bool needsLayout() const override;
    std::chrono::microseconds getLastComputeTime() const override;

    // Configuration
    void setDirection(FlexDirection dir);
    void setJustify(JustifyContent justify);
    void setAlign(AlignItems align);
    void setWrap(FlexWrap wrap);
    void setGap(float gap);
    void setChildren(const std::vector<Widget*>& children);
    void setBounds(const Rect& bounds);

    // Getters
    FlexDirection getDirection() const { return m_direction; }
    JustifyContent getJustify() const { return m_justify; }
    AlignItems getAlign() const { return m_align; }
    FlexWrap getWrap() const { return m_wrap; }
    float getGap() const { return m_gap; }
    const Rect& getBounds() const { return m_bounds; }

private:
    FlexDirection m_direction = FlexDirection::Row;
    JustifyContent m_justify = JustifyContent::Start;
    AlignItems m_align = AlignItems::Start;
    FlexWrap m_wrap = FlexWrap::NoWrap;
    float m_gap = 0.0f;
    Rect m_bounds;
    std::vector<Widget*> m_children;
    std::vector<Rect> m_childBounds;
    bool m_needsLayout = true;
    std::chrono::microseconds m_lastComputeTime{0};
};

/**
 * @class GridImpl
 * @brief Implementation class for Grid layout
 */
class GridImpl : public ILayout {
public:
    GridImpl();

    // ILayout interface
    Size layout(const LayoutConstraints& constraints) override;
    Rect getChildBounds(size_t index) const override;
    size_t getChildCount() const override;
    void invalidate() override;
    bool needsLayout() const override;
    std::chrono::microseconds getLastComputeTime() const override;

    // Configuration
    void setColumns(int count);
    void setRows(int count);
    void setColumnGap(float gap);
    void setRowGap(float gap);
    void setTemplateColumns(const std::string& tmpl);
    void setTemplateRows(const std::string& tmpl);
    void setChildren(const std::vector<Widget*>& children);
    void setBounds(const Rect& bounds);

    // Getters
    int getColumns() const { return m_columns; }
    int getRows() const { return m_rows; }
    float getColumnGap() const { return m_columnGap; }
    float getRowGap() const { return m_rowGap; }
    const std::string& getTemplateColumns() const { return m_templateColumns; }
    const std::string& getTemplateRows() const { return m_templateRows; }
    const Rect& getBounds() const { return m_bounds; }

private:
    std::vector<float> parseTemplate(const std::string& tmpl, float totalSize, int count) const;

    int m_columns = 1;
    int m_rows = 1;
    float m_columnGap = 0.0f;
    float m_rowGap = 0.0f;
    std::string m_templateColumns;
    std::string m_templateRows;
    Rect m_bounds;
    std::vector<Widget*> m_children;
    std::vector<Rect> m_childBounds;
    bool m_needsLayout = true;
    std::chrono::microseconds m_lastComputeTime{0};
};

/**
 * @class StackImpl
 * @brief Implementation class for Stack layout (z-index ordering)
 */
class StackImpl : public ILayout {
public:
    StackImpl();

    // ILayout interface
    Size layout(const LayoutConstraints& constraints) override;
    Rect getChildBounds(size_t index) const override;
    size_t getChildCount() const override;
    void invalidate() override;
    bool needsLayout() const override;
    std::chrono::microseconds getLastComputeTime() const override;

    // Configuration
    void setChildren(const std::vector<Widget*>& children);
    void setBounds(const Rect& bounds);

    // Getters
    const Rect& getBounds() const { return m_bounds; }

private:
    Rect m_bounds;
    std::vector<Widget*> m_children;
    std::vector<Rect> m_childBounds;
    bool m_needsLayout = true;
    std::chrono::microseconds m_lastComputeTime{0};
};

/**
 * @class AbsoluteImpl
 * @brief Implementation class for Absolute positioning layout
 */
class AbsoluteImpl : public ILayout {
public:
    AbsoluteImpl();

    // ILayout interface
    Size layout(const LayoutConstraints& constraints) override;
    Rect getChildBounds(size_t index) const override;
    size_t getChildCount() const override;
    void invalidate() override;
    bool needsLayout() const override;
    std::chrono::microseconds getLastComputeTime() const override;

    // Configuration
    void setChildren(const std::vector<Widget*>& children);
    void setBounds(const Rect& bounds);

    // Getters
    const Rect& getBounds() const { return m_bounds; }

private:
    Rect m_bounds;
    std::vector<Widget*> m_children;
    std::vector<Rect> m_childBounds;
    bool m_needsLayout = true;
    std::chrono::microseconds m_lastComputeTime{0};
};

/**
 * @class Flex
 * @brief Flexbox layout container with Builder Pattern API
 */
class Flex : public Widget {
public:
    static Flex create();

    Flex& direction(FlexDirection dir);
    Flex& justify(JustifyContent justify);
    Flex& align(AlignItems align);
    Flex& wrap(FlexWrap wrap);
    Flex& gap(float gap);

    Flex& children(std::initializer_list<Widget*> widgets);

    FlexHandle build();

    // Access to implementation for layout operations
    FlexImpl* getImpl() const { return m_impl.get(); }

private:
    Flex();
    std::shared_ptr<FlexImpl> m_impl;
};

/**
 * @class Grid
 * @brief Grid layout container with Builder Pattern API
 */
class Grid : public Widget {
public:
    static Grid create();

    Grid& columns(int count);
    Grid& rows(int count);
    Grid& columnGap(float gap);
    Grid& rowGap(float gap);
    Grid& templateColumns(const std::string& tmpl);
    Grid& templateRows(const std::string& tmpl);

    Grid& children(std::initializer_list<Widget*> widgets);

    GridHandle build();

    // Access to implementation for layout operations
    GridImpl* getImpl() const { return m_impl.get(); }

private:
    Grid();
    std::shared_ptr<GridImpl> m_impl;
};

/**
 * @class Stack
 * @brief Stack layout container (z-index ordering) with Builder Pattern API
 */
class Stack : public Widget {
public:
    static Stack create();

    Stack& children(std::initializer_list<Widget*> widgets);

    StackHandle build();

    // Access to implementation for layout operations
    StackImpl* getImpl() const { return m_impl.get(); }

private:
    Stack();
    std::shared_ptr<StackImpl> m_impl;
};

/**
 * @class Absolute
 * @brief Absolute positioning container with Builder Pattern API
 */
class Absolute : public Widget {
public:
    static Absolute create();

    Absolute& children(std::initializer_list<Widget*> widgets);

    AbsoluteHandle build();

    // Access to implementation for layout operations
    AbsoluteImpl* getImpl() const { return m_impl.get(); }

private:
    Absolute();
    std::shared_ptr<AbsoluteImpl> m_impl;
};

/**
 * @class LayoutManager
 * @brief Manages layout recalculation and responsive updates
 */
class LayoutManager {
public:
    using ResizeCallback = std::function<void(int, int)>;

    static LayoutManager& instance();

    /**
     * @brief Register a layout for management
     */
    void registerLayout(ILayout* layout);

    /**
     * @brief Unregister a layout
     */
    void unregisterLayout(ILayout* layout);

    /**
     * @brief Handle window resize event
     * @param width New window width
     * @param height New window height
     */
    void onWindowResize(int width, int height);

    /**
     * @brief Recalculate all registered layouts
     * @return Total time taken for recalculation
     */
    std::chrono::microseconds recalculateAll();

    /**
     * @brief Set callback for resize events
     */
    void setResizeCallback(ResizeCallback callback);

    /**
     * @brief Get the last recalculation time
     */
    std::chrono::microseconds getLastRecalculationTime() const;

    /**
     * @brief Check if last recalculation was within target time (16ms)
     */
    bool isWithinTargetTime() const;

    /**
     * @brief Get current window dimensions
     */
    int getWindowWidth() const { return m_windowWidth; }
    int getWindowHeight() const { return m_windowHeight; }

    /**
     * @brief Get target recalculation time in microseconds (16ms = 16000us)
     */
    static constexpr int64_t TARGET_RECALC_TIME_US = 16000;

private:
    LayoutManager() = default;
    std::vector<ILayout*> m_layouts;
    ResizeCallback m_resizeCallback;
    std::chrono::microseconds m_lastRecalculationTime{0};
    int m_windowWidth = 0;
    int m_windowHeight = 0;
};

/**
 * @brief Connect a platform window's resize callback to the LayoutManager
 * @param window The platform window to connect
 * 
 * This is a convenience function that sets up the window's resize callback
 * to automatically trigger layout recalculation when the window is resized.
 */
void connectWindowToLayoutManager(class IPlatformWindow* window);

} // namespace KillerGK
