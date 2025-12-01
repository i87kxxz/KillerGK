/**
 * @file TreeView.hpp
 * @brief TreeView widget for KillerGK with hierarchical display and drag-drop
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
 * @struct TreeNode
 * @brief Represents a node in the TreeView hierarchy
 */
struct TreeNode {
    std::string id;                      ///< Unique node identifier
    std::string text;                    ///< Display text
    std::string icon;                    ///< Optional icon path
    std::string expandedIcon;            ///< Icon when expanded (optional)
    bool expanded = false;               ///< Whether node is expanded
    bool selected = false;               ///< Whether node is selected
    bool enabled = true;                 ///< Whether node is interactive
    bool draggable = true;               ///< Whether node can be dragged
    bool droppable = true;               ///< Whether items can be dropped on this node
    std::vector<TreeNode> children;      ///< Child nodes
    std::any userData;                   ///< Custom user data

    TreeNode() = default;
    TreeNode(const std::string& nodeId, const std::string& nodeText)
        : id(nodeId), text(nodeText) {}
    TreeNode(const std::string& nodeId, const std::string& nodeText, const std::string& nodeIcon)
        : id(nodeId), text(nodeText), icon(nodeIcon) {}

    /**
     * @brief Add a child node
     * @param child Child node to add
     * @return Reference to this node for chaining
     */
    TreeNode& addChild(const TreeNode& child) {
        children.push_back(child);
        return *this;
    }

    /**
     * @brief Check if node has children
     * @return true if node has children
     */
    [[nodiscard]] bool hasChildren() const { return !children.empty(); }

    /**
     * @brief Find child by id (recursive)
     * @param childId Child identifier
     * @return Pointer to child or nullptr
     */
    TreeNode* findChild(const std::string& childId) {
        for (auto& child : children) {
            if (child.id == childId) return &child;
            if (auto* found = child.findChild(childId)) return found;
        }
        return nullptr;
    }

    /**
     * @brief Find child by id (const version)
     * @param childId Child identifier
     * @return Pointer to child or nullptr
     */
    [[nodiscard]] const TreeNode* findChild(const std::string& childId) const {
        for (const auto& child : children) {
            if (child.id == childId) return &child;
            if (const auto* found = child.findChild(childId)) return found;
        }
        return nullptr;
    }
};

/**
 * @struct TreeDragData
 * @brief Data for drag-drop operations
 */
struct TreeDragData {
    std::string sourceNodeId;
    std::string targetNodeId;
    enum class DropPosition { Before, After, Inside } position = DropPosition::Inside;
};


/**
 * @class TreeView
 * @brief Hierarchical tree widget with expand/collapse and drag-drop support
 * 
 * Displays hierarchical data with animated expand/collapse, selection,
 * and drag-drop reordering capabilities.
 * 
 * Example:
 * @code
 * TreeNode root("root", "Root");
 * root.addChild(TreeNode("child1", "Child 1"))
 *     .addChild(TreeNode("child2", "Child 2"));
 * 
 * auto tree = TreeView::create()
 *     .nodes({root})
 *     .onSelect([](const TreeNode& node) {
 *         std::cout << "Selected: " << node.text;
 *     });
 * @endcode
 */
class TreeView : public Widget {
public:
    using NodeRenderer = std::function<void(const TreeNode&, int depth, bool selected, bool hovered)>;

    virtual ~TreeView() = default;

    /**
     * @brief Create a new TreeView instance
     * @return New TreeView with default properties
     */
    static TreeView create();

    // =========================================================================
    // Node Management
    // =========================================================================

    /**
     * @brief Set root nodes
     * @param nodes Vector of root nodes
     * @return Reference to this TreeView for chaining
     */
    TreeView& nodes(const std::vector<TreeNode>& nodes);

    /**
     * @brief Add a root node
     * @param node Node to add
     * @return Reference to this TreeView for chaining
     */
    TreeView& addNode(const TreeNode& node);

    /**
     * @brief Remove node by id (searches entire tree)
     * @param id Node identifier
     * @return Reference to this TreeView for chaining
     */
    TreeView& removeNode(const std::string& id);

    /**
     * @brief Clear all nodes
     * @return Reference to this TreeView for chaining
     */
    TreeView& clearNodes();

    /**
     * @brief Get root nodes
     * @return Vector of root nodes
     */
    [[nodiscard]] const std::vector<TreeNode>& getNodes() const;

    /**
     * @brief Get mutable root nodes
     * @return Vector of root nodes
     */
    [[nodiscard]] std::vector<TreeNode>& getNodes();

    /**
     * @brief Find node by id (searches entire tree)
     * @param id Node identifier
     * @return Pointer to node or nullptr
     */
    [[nodiscard]] TreeNode* findNode(const std::string& id);

    /**
     * @brief Find node by id (const version)
     * @param id Node identifier
     * @return Pointer to node or nullptr
     */
    [[nodiscard]] const TreeNode* findNode(const std::string& id) const;

    /**
     * @brief Get parent of a node
     * @param id Node identifier
     * @return Pointer to parent node or nullptr if root
     */
    [[nodiscard]] TreeNode* getParent(const std::string& id);

    // =========================================================================
    // Expand/Collapse
    // =========================================================================

    /**
     * @brief Expand node by id
     * @param id Node identifier
     * @param recursive Whether to expand children too
     * @return Reference to this TreeView for chaining
     */
    TreeView& expand(const std::string& id, bool recursive = false);

    /**
     * @brief Collapse node by id
     * @param id Node identifier
     * @param recursive Whether to collapse children too
     * @return Reference to this TreeView for chaining
     */
    TreeView& collapse(const std::string& id, bool recursive = false);

    /**
     * @brief Toggle expand/collapse state
     * @param id Node identifier
     * @return Reference to this TreeView for chaining
     */
    TreeView& toggle(const std::string& id);

    /**
     * @brief Expand all nodes
     * @return Reference to this TreeView for chaining
     */
    TreeView& expandAll();

    /**
     * @brief Collapse all nodes
     * @return Reference to this TreeView for chaining
     */
    TreeView& collapseAll();

    /**
     * @brief Check if node is expanded
     * @param id Node identifier
     * @return true if expanded
     */
    [[nodiscard]] bool isExpanded(const std::string& id) const;

    // =========================================================================
    // Selection
    // =========================================================================

    /**
     * @brief Enable or disable multi-selection
     * @param enabled Whether multi-select is enabled
     * @return Reference to this TreeView for chaining
     */
    TreeView& multiSelect(bool enabled);

    /**
     * @brief Check if multi-select is enabled
     * @return true if multi-select enabled
     */
    [[nodiscard]] bool isMultiSelect() const;

    /**
     * @brief Select node by id
     * @param id Node identifier
     * @param addToSelection Whether to add to existing selection
     * @return Reference to this TreeView for chaining
     */
    TreeView& select(const std::string& id, bool addToSelection = false);

    /**
     * @brief Deselect node by id
     * @param id Node identifier
     * @return Reference to this TreeView for chaining
     */
    TreeView& deselect(const std::string& id);

    /**
     * @brief Clear all selection
     * @return Reference to this TreeView for chaining
     */
    TreeView& clearSelection();

    /**
     * @brief Get selected node ids
     * @return Vector of selected node ids
     */
    [[nodiscard]] std::vector<std::string> getSelectedIds() const;

    /**
     * @brief Get selected nodes
     * @return Vector of pointers to selected nodes
     */
    [[nodiscard]] std::vector<const TreeNode*> getSelectedNodes() const;

    // =========================================================================
    // Drag and Drop
    // =========================================================================

    /**
     * @brief Enable or disable drag-drop reordering
     * @param enabled Whether drag-drop is enabled
     * @return Reference to this TreeView for chaining
     */
    TreeView& dragDropEnabled(bool enabled);

    /**
     * @brief Check if drag-drop is enabled
     * @return true if drag-drop enabled
     */
    [[nodiscard]] bool isDragDropEnabled() const;

    /**
     * @brief Move node to new parent
     * @param nodeId Node to move
     * @param newParentId New parent id (empty for root)
     * @param index Position in new parent's children (-1 for end)
     * @return Reference to this TreeView for chaining
     */
    TreeView& moveNode(const std::string& nodeId, const std::string& newParentId, int index = -1);

    // =========================================================================
    // Appearance
    // =========================================================================

    /**
     * @brief Set node height
     * @param height Height in pixels
     * @return Reference to this TreeView for chaining
     */
    TreeView& nodeHeight(float height);

    /**
     * @brief Get node height
     * @return Node height in pixels
     */
    [[nodiscard]] float getNodeHeight() const;

    /**
     * @brief Set indentation per level
     * @param indent Indentation in pixels
     * @return Reference to this TreeView for chaining
     */
    TreeView& indentation(float indent);

    /**
     * @brief Get indentation per level
     * @return Indentation in pixels
     */
    [[nodiscard]] float getIndentation() const;

    /**
     * @brief Show or hide expand/collapse icons
     * @param show Whether to show icons
     * @return Reference to this TreeView for chaining
     */
    TreeView& showExpandIcons(bool show);

    /**
     * @brief Check if expand icons are shown
     * @return true if expand icons visible
     */
    [[nodiscard]] bool hasExpandIcons() const;

    /**
     * @brief Show or hide connecting lines
     * @param show Whether to show lines
     * @return Reference to this TreeView for chaining
     */
    TreeView& showLines(bool show);

    /**
     * @brief Check if connecting lines are shown
     * @return true if lines visible
     */
    [[nodiscard]] bool hasLines() const;

    /**
     * @brief Set hover color
     * @param color Hover color
     * @return Reference to this TreeView for chaining
     */
    TreeView& hoverColor(const Color& color);

    /**
     * @brief Get hover color
     * @return Hover color
     */
    [[nodiscard]] const Color& getHoverColor() const;

    /**
     * @brief Set selection color
     * @param color Selection color
     * @return Reference to this TreeView for chaining
     */
    TreeView& selectionColor(const Color& color);

    /**
     * @brief Get selection color
     * @return Selection color
     */
    [[nodiscard]] const Color& getSelectionColor() const;

    /**
     * @brief Enable or disable expand/collapse animation
     * @param enabled Whether animation is enabled
     * @return Reference to this TreeView for chaining
     */
    TreeView& animateExpand(bool enabled);

    /**
     * @brief Check if expand animation is enabled
     * @return true if animation enabled
     */
    [[nodiscard]] bool hasExpandAnimation() const;

    // =========================================================================
    // Custom Rendering
    // =========================================================================

    /**
     * @brief Set custom node renderer
     * @param renderer Node rendering function
     * @return Reference to this TreeView for chaining
     */
    TreeView& nodeRenderer(NodeRenderer renderer);

    // =========================================================================
    // Callbacks
    // =========================================================================

    /**
     * @brief Set selection change callback
     * @param callback Function called when selection changes
     * @return Reference to this TreeView for chaining
     */
    TreeView& onSelect(std::function<void(const TreeNode&)> callback);

    /**
     * @brief Set expand/collapse callback
     * @param callback Function called when node expands or collapses
     * @return Reference to this TreeView for chaining
     */
    TreeView& onExpand(std::function<void(const TreeNode&, bool expanded)> callback);

    /**
     * @brief Set node double-click callback
     * @param callback Function called on node double-click
     * @return Reference to this TreeView for chaining
     */
    TreeView& onDoubleClick(std::function<void(const TreeNode&)> callback);

    /**
     * @brief Set drag start callback
     * @param callback Function called when drag starts
     * @return Reference to this TreeView for chaining
     */
    TreeView& onDragStart(std::function<void(const TreeNode&)> callback);

    /**
     * @brief Set drop callback
     * @param callback Function called when drop occurs
     * @return Reference to this TreeView for chaining
     */
    TreeView& onDrop(std::function<void(const TreeDragData&)> callback);

    // =========================================================================
    // Scrolling
    // =========================================================================

    /**
     * @brief Scroll to make node visible
     * @param id Node identifier
     * @return Reference to this TreeView for chaining
     */
    TreeView& scrollToNode(const std::string& id);

    /**
     * @brief Get scroll offset
     * @return Current scroll offset
     */
    [[nodiscard]] float getScrollOffset() const;

    /**
     * @brief Set scroll offset
     * @param offset Scroll offset in pixels
     * @return Reference to this TreeView for chaining
     */
    TreeView& scrollTo(float offset);

protected:
    TreeView();

    struct TreeViewData;
    std::shared_ptr<TreeViewData> m_treeData;
};

} // namespace KillerGK
