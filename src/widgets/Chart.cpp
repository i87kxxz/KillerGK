/**
 * @file Chart.cpp
 * @brief Chart widget implementation
 */

#include "KillerGK/widgets/Chart.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace KillerGK {

// =============================================================================
// ChartData - Internal data structure
// =============================================================================

struct Chart::ChartData {
    ChartType type = ChartType::Line;
    std::vector<ChartSeries> seriesList;
    ChartAxis xAxis;
    ChartAxis yAxis;
    ChartLegend legend;
    ChartTooltip tooltip;
    
    // Animation
    bool animateOnLoad = true;
    float animationDuration = 500.0f;
    float animationProgress = 0.0f;
    bool animating = false;
    
    // Pie/Donut specific
    float innerRadius = 0.0f;
    float startAngle = -90.0f;
    
    // Bar specific
    float barWidth = 0.8f;
    float barSpacing = 4.0f;
    bool stacked = false;
    
    // Appearance
    float paddingTop = 20.0f;
    float paddingRight = 20.0f;
    float paddingBottom = 40.0f;
    float paddingLeft = 50.0f;
    
    std::vector<Color> colorPalette = {
        Color(0.25f, 0.47f, 0.85f, 1.0f),  // Blue
        Color(0.85f, 0.25f, 0.25f, 1.0f),  // Red
        Color(0.25f, 0.75f, 0.35f, 1.0f),  // Green
        Color(0.95f, 0.65f, 0.15f, 1.0f),  // Orange
        Color(0.55f, 0.25f, 0.75f, 1.0f),  // Purple
        Color(0.15f, 0.75f, 0.75f, 1.0f),  // Cyan
        Color(0.85f, 0.45f, 0.55f, 1.0f),  // Pink
        Color(0.45f, 0.55f, 0.35f, 1.0f)   // Olive
    };
    
    // Callbacks
    std::function<void(const ChartSeries&, const DataPoint&)> onPointClickCallback;
    std::function<void(const ChartSeries&, const DataPoint&, bool)> onPointHoverCallback;
    std::function<void(const ChartSeries&)> onLegendClickCallback;
    
    // Get color for series index
    Color getSeriesColor(size_t index) const {
        if (index < seriesList.size() && seriesList[index].color.a > 0) {
            return seriesList[index].color;
        }
        return colorPalette[index % colorPalette.size()];
    }
};

// =============================================================================
// Chart Implementation
// =============================================================================

Chart::Chart()
    : Widget()
    , m_chartData(std::make_shared<ChartData>())
{
    backgroundColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
}

Chart Chart::create() {
    return Chart();
}

// Chart Type
Chart& Chart::chartType(ChartType type) {
    m_chartData->type = type;
    return *this;
}

ChartType Chart::getChartType() const {
    return m_chartData->type;
}

// Data Series
Chart& Chart::series(const std::vector<ChartSeries>& series) {
    m_chartData->seriesList = series;
    return *this;
}

Chart& Chart::addSeries(const ChartSeries& series) {
    m_chartData->seriesList.push_back(series);
    // Assign color if not set
    if (m_chartData->seriesList.back().color.a == 0) {
        m_chartData->seriesList.back().color = 
            m_chartData->getSeriesColor(m_chartData->seriesList.size() - 1);
    }
    return *this;
}

Chart& Chart::removeSeries(const std::string& id) {
    auto it = std::remove_if(m_chartData->seriesList.begin(), m_chartData->seriesList.end(),
        [&id](const ChartSeries& s) { return s.id == id; });
    m_chartData->seriesList.erase(it, m_chartData->seriesList.end());
    return *this;
}

Chart& Chart::clearSeries() {
    m_chartData->seriesList.clear();
    return *this;
}

const std::vector<ChartSeries>& Chart::getSeries() const {
    return m_chartData->seriesList;
}

ChartSeries* Chart::getSeriesById(const std::string& id) {
    for (auto& s : m_chartData->seriesList) {
        if (s.id == id) return &s;
    }
    return nullptr;
}

Chart& Chart::updateSeriesData(const std::string& id, const std::vector<DataPoint>& data) {
    if (auto* s = getSeriesById(id)) {
        s->data = data;
    }
    return *this;
}

// Axes Configuration
Chart& Chart::xAxis(const ChartAxis& axis) {
    m_chartData->xAxis = axis;
    return *this;
}

const ChartAxis& Chart::getXAxis() const {
    return m_chartData->xAxis;
}

ChartAxis& Chart::getXAxis() {
    return m_chartData->xAxis;
}

Chart& Chart::yAxis(const ChartAxis& axis) {
    m_chartData->yAxis = axis;
    return *this;
}

const ChartAxis& Chart::getYAxis() const {
    return m_chartData->yAxis;
}

ChartAxis& Chart::getYAxis() {
    return m_chartData->yAxis;
}

// Legend
Chart& Chart::legend(const ChartLegend& legend) {
    m_chartData->legend = legend;
    return *this;
}

const ChartLegend& Chart::getLegend() const {
    return m_chartData->legend;
}

Chart& Chart::showLegend(bool show) {
    m_chartData->legend.visible = show;
    return *this;
}

Chart& Chart::legendPosition(ChartLegend::Position position) {
    m_chartData->legend.position = position;
    return *this;
}

// Tooltip
Chart& Chart::tooltip(const ChartTooltip& tooltip) {
    m_chartData->tooltip = tooltip;
    return *this;
}

const ChartTooltip& Chart::getTooltip() const {
    return m_chartData->tooltip;
}

Chart& Chart::enableTooltip(bool enabled) {
    m_chartData->tooltip.enabled = enabled;
    return *this;
}

Chart& Chart::tooltipFormatter(std::function<std::string(const ChartSeries&, const DataPoint&)> formatter) {
    m_chartData->tooltip.formatter = std::move(formatter);
    return *this;
}


// Animation
Chart& Chart::animateOnLoad(bool enabled) {
    m_chartData->animateOnLoad = enabled;
    return *this;
}

bool Chart::hasLoadAnimation() const {
    return m_chartData->animateOnLoad;
}

Chart& Chart::animationDuration(float duration) {
    m_chartData->animationDuration = duration;
    return *this;
}

float Chart::getAnimationDuration() const {
    return m_chartData->animationDuration;
}

void Chart::animate() {
    m_chartData->animating = true;
    m_chartData->animationProgress = 0.0f;
}

// Pie/Donut Specific
Chart& Chart::innerRadius(float radius) {
    m_chartData->innerRadius = std::clamp(radius, 0.0f, 0.95f);
    return *this;
}

float Chart::getInnerRadius() const {
    return m_chartData->innerRadius;
}

Chart& Chart::startAngle(float angle) {
    m_chartData->startAngle = angle;
    return *this;
}

float Chart::getStartAngle() const {
    return m_chartData->startAngle;
}

// Bar Chart Specific
Chart& Chart::barWidth(float width) {
    m_chartData->barWidth = std::clamp(width, 0.1f, 1.0f);
    return *this;
}

float Chart::getBarWidth() const {
    return m_chartData->barWidth;
}

Chart& Chart::barSpacing(float spacing) {
    m_chartData->barSpacing = spacing;
    return *this;
}

float Chart::getBarSpacing() const {
    return m_chartData->barSpacing;
}

Chart& Chart::stacked(bool stacked) {
    m_chartData->stacked = stacked;
    return *this;
}

bool Chart::isStacked() const {
    return m_chartData->stacked;
}

// Appearance
Chart& Chart::chartPadding(float top, float right, float bottom, float left) {
    m_chartData->paddingTop = top;
    m_chartData->paddingRight = right;
    m_chartData->paddingBottom = bottom;
    m_chartData->paddingLeft = left;
    return *this;
}

Chart& Chart::colorPalette(const std::vector<Color>& colors) {
    m_chartData->colorPalette = colors;
    return *this;
}

const std::vector<Color>& Chart::getColorPalette() const {
    return m_chartData->colorPalette;
}

// Callbacks
Chart& Chart::onPointClick(std::function<void(const ChartSeries&, const DataPoint&)> callback) {
    m_chartData->onPointClickCallback = std::move(callback);
    return *this;
}

Chart& Chart::onPointHover(std::function<void(const ChartSeries&, const DataPoint&, bool)> callback) {
    m_chartData->onPointHoverCallback = std::move(callback);
    return *this;
}

Chart& Chart::onLegendClick(std::function<void(const ChartSeries&)> callback) {
    m_chartData->onLegendClickCallback = std::move(callback);
    return *this;
}

// Data Helpers
void Chart::getDataRange(double& minX, double& maxX, double& minY, double& maxY) const {
    minX = std::numeric_limits<double>::max();
    maxX = std::numeric_limits<double>::lowest();
    minY = std::numeric_limits<double>::max();
    maxY = std::numeric_limits<double>::lowest();
    
    for (const auto& series : m_chartData->seriesList) {
        for (const auto& point : series.data) {
            minX = std::min(minX, point.x);
            maxX = std::max(maxX, point.x);
            minY = std::min(minY, point.y);
            maxY = std::max(maxY, point.y);
        }
    }
    
    // Handle empty data
    if (minX > maxX) {
        minX = 0.0;
        maxX = 100.0;
    }
    if (minY > maxY) {
        minY = 0.0;
        maxY = 100.0;
    }
    
    // Add some padding to Y axis
    double yRange = maxY - minY;
    if (yRange > 0) {
        minY = std::max(0.0, minY - yRange * 0.1);
        maxY = maxY + yRange * 0.1;
    }
}

double Chart::getTotalValue() const {
    double total = 0.0;
    if (!m_chartData->seriesList.empty()) {
        for (const auto& point : m_chartData->seriesList[0].data) {
            total += std::abs(point.y);
        }
    }
    return total;
}

} // namespace KillerGK
