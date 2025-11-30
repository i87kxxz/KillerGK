/**
 * @file Layout.hpp
 * @brief Layout system for KillerGK
 */

#pragma once

#include "../widgets/Widget.hpp"
#include <vector>
#include <initializer_list>

namespace KillerGK {

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

/**
 * @brief Handle to a built flex container
 */
using FlexHandle = std::shared_ptr<class FlexImpl>;

/**
 * @class Flex
 * @brief Flexbox layout container
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

private:
    Flex();
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

/**
 * @brief Handle to a built grid container
 */
using GridHandle = std::shared_ptr<class GridImpl>;

/**
 * @class Grid
 * @brief Grid layout container
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

private:
    Grid();
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

/**
 * @brief Handle to a built stack container
 */
using StackHandle = std::shared_ptr<class StackImpl>;

/**
 * @class Stack
 * @brief Stack layout container (z-index ordering)
 */
class Stack : public Widget {
public:
    static Stack create();

    Stack& children(std::initializer_list<Widget*> widgets);

    StackHandle build();

private:
    Stack();
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

/**
 * @brief Handle to a built absolute container
 */
using AbsoluteHandle = std::shared_ptr<class AbsoluteImpl>;

/**
 * @class Absolute
 * @brief Absolute positioning container
 */
class Absolute : public Widget {
public:
    static Absolute create();

    Absolute& children(std::initializer_list<Widget*> widgets);

    AbsoluteHandle build();

private:
    Absolute();
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

} // namespace KillerGK
