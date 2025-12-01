/**
 * @file Chart.hpp
 * @brief Chart widget for KillerGK - Line, Bar, Pie, Area charts
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
 * @enum ChartType
 * @brief Type of chart to render
 */
enum class ChartType {
    Line,
    Bar,
    Pie,
    Area,
    Scatter,
    Donut
};

/**
 * @struct DataPoint
 * @brief A single data point in a chart series
 */
struct DataPoint {
    double x = 0.0;
    double y = 0.0;
    std::string label;
    std::any userData;

    DataPoint() = default;
    DataPoint(double xVal, double yVal) : x(xVal), y(yVal) {}
    DataPoint(double xVal, double yVal, const std::string& lbl) 
        : x(xVal), y(yVal), label(lbl) {}
};

/**
 * @struct ChartSeries
 * @brief A data series in a chart
 */
struct ChartSeries {
    std::string id;                      ///< Unique series identifier
    std::string name;                    ///< Display name for legend
    std::vector<DataPoint> data;         ///< Data points
    Color color;                         ///< Series color
    float lineWidth = 2.0f;              ///< Line width for line/area charts
    bool showPoints = true;              ///< Show data point markers
    float pointRadius = 4.0f;            ///< Data point marker radius
    bool filled = false;                 ///< Fill area under line
    float fillOpacity = 0.3f;            ///< Fill opacity for area charts

    ChartSeries() = default;
    ChartSeries(const std::string& seriesId, const std::string& seriesName)
        : id(seriesId), name(seriesName) {}
    ChartSeries(const std::string& seriesId, const std::string& seriesName, const Color& seriesColor)
        : id(seriesId), name(seriesName), color(seriesColor) {}

    /**
     * @brief Add a data point
     * @param point Data point to add
     * @return Reference to this series for chaining
     */
    ChartSeries& addPoint(const DataPoint& point) {
        data.push_back(point);
        return *this;
    }

    /**
     * @brief Add a data point with x,y values
     * @param x X value
     * @param y Y value
     * @return Reference to this series for chaining
     */
    ChartSeries& addPoint(double x, double y) {
        data.emplace_back(x, y);
        return *this;
    }
};

/**
 * @struct ChartAxis
 * @brief Configuration for a chart axis
 */
struct ChartAxis {
    std::string title;
    double min = 0.0;
    double max = 100.0;
    bool autoScale = true;
    int tickCount = 5;
    std::string format;                  ///< Number format (e.g., "%.2f")
    bool showGrid = true;
    Color gridColor = Color(0.9f, 0.9f, 0.9f, 1.0f);
    Color labelColor = Color(0.3f, 0.3f, 0.3f, 1.0f);
};

/**
 * @struct ChartLegend
 * @brief Configuration for chart legend
 */
struct ChartLegend {
    bool visible = true;
    enum class Position { Top, Bottom, Left, Right } position = Position::Bottom;
    float itemSpacing = 20.0f;
};

/**
 * @struct ChartTooltip
 * @brief Configuration for chart tooltips
 */
struct ChartTooltip {
    bool enabled = true;
    Color backgroundColor = Color(0.2f, 0.2f, 0.2f, 0.9f);
    Color textColor = Color::White;
    float padding = 8.0f;
    float borderRadius = 4.0f;
    std::function<std::string(const ChartSeries&, const DataPoint&)> formatter;
};


/**
 * @class Chart
 * @brief Chart widget supporting line, bar, pie, and area charts
 * 
 * Example:
 * @code
 * ChartSeries sales("sales", "Sales", Color::Blue);
 * sales.addPoint(1, 100).addPoint(2, 150).addPoint(3, 120);
 * 
 * auto chart = Chart::create()
 *     .chartType(ChartType::Line)
 *     .addSeries(sales)
 *     .xAxis({"Month"})
 *     .yAxis({"Revenue ($)"})
 *     .animateOnLoad(true);
 * @endcode
 */
class Chart : public Widget {
public:
    virtual ~Chart() = default;

    /**
     * @brief Create a new Chart instance
     * @return New Chart with default properties
     */
    static Chart create();

    // =========================================================================
    // Chart Type
    // =========================================================================

    /**
     * @brief Set chart type
     * @param type Chart type
     * @return Reference to this Chart for chaining
     */
    Chart& chartType(ChartType type);

    /**
     * @brief Get chart type
     * @return Current chart type
     */
    [[nodiscard]] ChartType getChartType() const;

    // =========================================================================
    // Data Series
    // =========================================================================

    /**
     * @brief Set all series
     * @param series Vector of series
     * @return Reference to this Chart for chaining
     */
    Chart& series(const std::vector<ChartSeries>& series);

    /**
     * @brief Add a data series
     * @param series Series to add
     * @return Reference to this Chart for chaining
     */
    Chart& addSeries(const ChartSeries& series);

    /**
     * @brief Remove series by id
     * @param id Series identifier
     * @return Reference to this Chart for chaining
     */
    Chart& removeSeries(const std::string& id);

    /**
     * @brief Clear all series
     * @return Reference to this Chart for chaining
     */
    Chart& clearSeries();

    /**
     * @brief Get all series
     * @return Vector of series
     */
    [[nodiscard]] const std::vector<ChartSeries>& getSeries() const;

    /**
     * @brief Get series by id
     * @param id Series identifier
     * @return Pointer to series or nullptr
     */
    [[nodiscard]] ChartSeries* getSeriesById(const std::string& id);

    /**
     * @brief Update series data
     * @param id Series identifier
     * @param data New data points
     * @return Reference to this Chart for chaining
     */
    Chart& updateSeriesData(const std::string& id, const std::vector<DataPoint>& data);

    // =========================================================================
    // Axes Configuration
    // =========================================================================

    /**
     * @brief Set X axis configuration
     * @param axis Axis configuration
     * @return Reference to this Chart for chaining
     */
    Chart& xAxis(const ChartAxis& axis);

    /**
     * @brief Get X axis configuration
     * @return X axis configuration
     */
    [[nodiscard]] const ChartAxis& getXAxis() const;

    /**
     * @brief Get mutable X axis configuration
     * @return X axis configuration
     */
    [[nodiscard]] ChartAxis& getXAxis();

    /**
     * @brief Set Y axis configuration
     * @param axis Axis configuration
     * @return Reference to this Chart for chaining
     */
    Chart& yAxis(const ChartAxis& axis);

    /**
     * @brief Get Y axis configuration
     * @return Y axis configuration
     */
    [[nodiscard]] const ChartAxis& getYAxis() const;

    /**
     * @brief Get mutable Y axis configuration
     * @return Y axis configuration
     */
    [[nodiscard]] ChartAxis& getYAxis();

    // =========================================================================
    // Legend
    // =========================================================================

    /**
     * @brief Set legend configuration
     * @param legend Legend configuration
     * @return Reference to this Chart for chaining
     */
    Chart& legend(const ChartLegend& legend);

    /**
     * @brief Get legend configuration
     * @return Legend configuration
     */
    [[nodiscard]] const ChartLegend& getLegend() const;

    /**
     * @brief Show or hide legend
     * @param show Whether to show legend
     * @return Reference to this Chart for chaining
     */
    Chart& showLegend(bool show);

    /**
     * @brief Set legend position
     * @param position Legend position
     * @return Reference to this Chart for chaining
     */
    Chart& legendPosition(ChartLegend::Position position);

    // =========================================================================
    // Tooltip
    // =========================================================================

    /**
     * @brief Set tooltip configuration
     * @param tooltip Tooltip configuration
     * @return Reference to this Chart for chaining
     */
    Chart& tooltip(const ChartTooltip& tooltip);

    /**
     * @brief Get tooltip configuration
     * @return Tooltip configuration
     */
    [[nodiscard]] const ChartTooltip& getTooltip() const;

    /**
     * @brief Enable or disable tooltips
     * @param enabled Whether tooltips are enabled
     * @return Reference to this Chart for chaining
     */
    Chart& enableTooltip(bool enabled);

    /**
     * @brief Set custom tooltip formatter
     * @param formatter Formatter function
     * @return Reference to this Chart for chaining
     */
    Chart& tooltipFormatter(std::function<std::string(const ChartSeries&, const DataPoint&)> formatter);

    // =========================================================================
    // Animation
    // =========================================================================

    /**
     * @brief Enable or disable load animation
     * @param enabled Whether to animate on load
     * @return Reference to this Chart for chaining
     */
    Chart& animateOnLoad(bool enabled);

    /**
     * @brief Check if load animation is enabled
     * @return true if animation enabled
     */
    [[nodiscard]] bool hasLoadAnimation() const;

    /**
     * @brief Set animation duration
     * @param duration Duration in milliseconds
     * @return Reference to this Chart for chaining
     */
    Chart& animationDuration(float duration);

    /**
     * @brief Get animation duration
     * @return Duration in milliseconds
     */
    [[nodiscard]] float getAnimationDuration() const;

    /**
     * @brief Trigger animation (replay)
     */
    void animate();

    // =========================================================================
    // Pie/Donut Specific
    // =========================================================================

    /**
     * @brief Set inner radius for donut charts (0 for pie)
     * @param radius Inner radius as percentage of outer radius (0-1)
     * @return Reference to this Chart for chaining
     */
    Chart& innerRadius(float radius);

    /**
     * @brief Get inner radius
     * @return Inner radius percentage
     */
    [[nodiscard]] float getInnerRadius() const;

    /**
     * @brief Set start angle for pie/donut charts
     * @param angle Start angle in degrees
     * @return Reference to this Chart for chaining
     */
    Chart& startAngle(float angle);

    /**
     * @brief Get start angle
     * @return Start angle in degrees
     */
    [[nodiscard]] float getStartAngle() const;

    // =========================================================================
    // Bar Chart Specific
    // =========================================================================

    /**
     * @brief Set bar width
     * @param width Bar width as percentage of available space (0-1)
     * @return Reference to this Chart for chaining
     */
    Chart& barWidth(float width);

    /**
     * @brief Get bar width
     * @return Bar width percentage
     */
    [[nodiscard]] float getBarWidth() const;

    /**
     * @brief Set bar spacing
     * @param spacing Spacing between bars in pixels
     * @return Reference to this Chart for chaining
     */
    Chart& barSpacing(float spacing);

    /**
     * @brief Get bar spacing
     * @return Bar spacing in pixels
     */
    [[nodiscard]] float getBarSpacing() const;

    /**
     * @brief Enable or disable stacked bars
     * @param stacked Whether bars are stacked
     * @return Reference to this Chart for chaining
     */
    Chart& stacked(bool stacked);

    /**
     * @brief Check if bars are stacked
     * @return true if stacked
     */
    [[nodiscard]] bool isStacked() const;

    // =========================================================================
    // Appearance
    // =========================================================================

    /**
     * @brief Set chart padding
     * @param top Top padding
     * @param right Right padding
     * @param bottom Bottom padding
     * @param left Left padding
     * @return Reference to this Chart for chaining
     */
    Chart& chartPadding(float top, float right, float bottom, float left);

    /**
     * @brief Set default color palette for series
     * @param colors Vector of colors
     * @return Reference to this Chart for chaining
     */
    Chart& colorPalette(const std::vector<Color>& colors);

    /**
     * @brief Get color palette
     * @return Vector of colors
     */
    [[nodiscard]] const std::vector<Color>& getColorPalette() const;

    // =========================================================================
    // Callbacks
    // =========================================================================

    /**
     * @brief Set data point click callback
     * @param callback Function called when data point is clicked
     * @return Reference to this Chart for chaining
     */
    Chart& onPointClick(std::function<void(const ChartSeries&, const DataPoint&)> callback);

    /**
     * @brief Set data point hover callback
     * @param callback Function called when data point is hovered
     * @return Reference to this Chart for chaining
     */
    Chart& onPointHover(std::function<void(const ChartSeries&, const DataPoint&, bool)> callback);

    /**
     * @brief Set legend item click callback
     * @param callback Function called when legend item is clicked
     * @return Reference to this Chart for chaining
     */
    Chart& onLegendClick(std::function<void(const ChartSeries&)> callback);

    // =========================================================================
    // Data Helpers
    // =========================================================================

    /**
     * @brief Get data range for all series
     * @param minX Output: minimum X value
     * @param maxX Output: maximum X value
     * @param minY Output: minimum Y value
     * @param maxY Output: maximum Y value
     */
    void getDataRange(double& minX, double& maxX, double& minY, double& maxY) const;

    /**
     * @brief Get total value (for pie charts)
     * @return Sum of all Y values in first series
     */
    [[nodiscard]] double getTotalValue() const;

protected:
    Chart();

    struct ChartData;
    std::shared_ptr<ChartData> m_chartData;
};

} // namespace KillerGK
