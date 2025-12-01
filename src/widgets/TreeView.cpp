/**
 * @file TreeView.cpp
 * @brief TreeView widget implementation
 */

#include "KillerGK/widgets/TreeView.hpp"
#include <algorithm>

namespace KillerGK {

// =============================================================================
// TreeViewData - Internal data structure
// =============================================================================

struct TreeView::TreeViewData {
    std::vector<TreeNode> nodes;
    std::vector<std::string> selectedIds;
    bool multiSelectEnabled = false;
    bool dragDropEnabled = true;
    
    // Appearance
    float nodeHeight = 28.0f;
    float indentation = 20.0f;
    bool showExpandIcons = true;
    bool showLines = false;
    bool animateExpand = true;
    Color hoverColor = Color(0.9f, 0.95f, 1.0f, 1.0f);
    Color selectionColor = Color(0.85f, 0.9f, 1.0f, 1.0f);
    
    // Scrolling
    float scrollOffset = 0.0f;
    
    // Custom renderer
    NodeRenderer customRenderer;
    
    // Callbacks
    std::function<void(const TreeNode&)> onSelectCallback;
    std::function<void(const TreeNode&, bool)> onExpandCallback;
    std::function<void(const TreeNode&)> onDoubleClickCallback;
    std::function<void(const TreeNode&)> onDragStartCallback;
    std::function<void(const TreeDragData&)> onDropCallback;
    
    // Helper to find node recursively
    TreeNode* findNodeRecursive(std::vector<TreeNode>& nodeList, const std::string& id) {
        for (auto& node : nodeList) {
            if (node.id == id) return &node;
            if (auto* found = findNodeRecursive(node.children, id)) return found;
        }
        return nullptr;
    }
    
    const TreeNode* findNodeRecursive(const std::vector<TreeNode>& nodeList, const std::string& id) const {
        for (const auto& node : nodeList) {
            if (node.id == id) return &node;
            if (const auto* found = findNodeRecursive(node.children, id)) return found;
        }
        return nullptr;
    }
    
    // Helper to find parent
    TreeNode* findParentRecursive(std::vector<TreeNode>& nodeList, const std::string& childId) {
        for (auto& node : nodeList) {
            for (const auto& child : node.children) {
                if (child.id == childId) return &node;
            }
            if (auto* found = findParentRecursive(node.children, childId)) return found;
        }
        return nullptr;
    }
    
    // Helper to remove node
    bool removeNodeRecursive(std::vector<TreeNode>& nodeList, const std::string& id) {
        auto it = std::remove_if(nodeList.begin(), nodeList.end(),
            [&id](const TreeNode& node) { return node.id == id; });
        if (it != nodeList.end()) {
            nodeList.erase(it, nodeList.end());
            return true;
        }
        for (auto& node : nodeList) {
            if (removeNodeRecursive(node.children, id)) return true;
        }
        return false;
    }
    
    // Helper to expand/collapse recursively
    void setExpandedRecursive(std::vector<TreeNode>& nodeList, bool expanded) {
        for (auto& node : nodeList) {
            node.expanded = expanded;
            setExpandedRecursive(node.children, expanded);
        }
    }
    
    // Helper to clear selection
    void clearSelectionRecursive(std::vector<TreeNode>& nodeList) {
        for (auto& node : nodeList) {
            node.selected = false;
            clearSelectionRecursive(node.children);
        }
    }
};

// =============================================================================
// TreeView Implementation
// =============================================================================

TreeView::TreeView()
    : Widget()
    , m_treeData(std::make_shared<TreeViewData>())
{
    backgroundColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
}

TreeView TreeView::create() {
    return TreeView();
}

// Node Management
TreeView& TreeView::nodes(const std::vector<TreeNode>& nodes) {
    m_treeData->nodes = nodes;
    m_treeData->selectedIds.clear();
    return *this;
}

TreeView& TreeView::addNode(const TreeNode& node) {
    m_treeData->nodes.push_back(node);
    return *this;
}

TreeView& TreeView::removeNode(const std::string& id) {
    m_treeData->removeNodeRecursive(m_treeData->nodes, id);
    
    // Remove from selection
    auto it = std::find(m_treeData->selectedIds.begin(), m_treeData->selectedIds.end(), id);
    if (it != m_treeData->selectedIds.end()) {
        m_treeData->selectedIds.erase(it);
    }
    
    return *this;
}

TreeView& TreeView::clearNodes() {
    m_treeData->nodes.clear();
    m_treeData->selectedIds.clear();
    return *this;
}

const std::vector<TreeNode>& TreeView::getNodes() const {
    return m_treeData->nodes;
}

std::vector<TreeNode>& TreeView::getNodes() {
    return m_treeData->nodes;
}

TreeNode* TreeView::findNode(const std::string& id) {
    return m_treeData->findNodeRecursive(m_treeData->nodes, id);
}

const TreeNode* TreeView::findNode(const std::string& id) const {
    return m_treeData->findNodeRecursive(m_treeData->nodes, id);
}

TreeNode* TreeView::getParent(const std::string& id) {
    return m_treeData->findParentRecursive(m_treeData->nodes, id);
}


// Expand/Collapse
TreeView& TreeView::expand(const std::string& id, bool recursive) {
    if (auto* node = findNode(id)) {
        node->expanded = true;
        if (recursive) {
            m_treeData->setExpandedRecursive(node->children, true);
        }
        if (m_treeData->onExpandCallback) {
            m_treeData->onExpandCallback(*node, true);
        }
    }
    return *this;
}

TreeView& TreeView::collapse(const std::string& id, bool recursive) {
    if (auto* node = findNode(id)) {
        node->expanded = false;
        if (recursive) {
            m_treeData->setExpandedRecursive(node->children, false);
        }
        if (m_treeData->onExpandCallback) {
            m_treeData->onExpandCallback(*node, false);
        }
    }
    return *this;
}

TreeView& TreeView::toggle(const std::string& id) {
    if (auto* node = findNode(id)) {
        if (node->expanded) {
            collapse(id);
        } else {
            expand(id);
        }
    }
    return *this;
}

TreeView& TreeView::expandAll() {
    m_treeData->setExpandedRecursive(m_treeData->nodes, true);
    return *this;
}

TreeView& TreeView::collapseAll() {
    m_treeData->setExpandedRecursive(m_treeData->nodes, false);
    return *this;
}

bool TreeView::isExpanded(const std::string& id) const {
    if (const auto* node = findNode(id)) {
        return node->expanded;
    }
    return false;
}

// Selection
TreeView& TreeView::multiSelect(bool enabled) {
    m_treeData->multiSelectEnabled = enabled;
    if (!enabled && m_treeData->selectedIds.size() > 1) {
        m_treeData->selectedIds.resize(1);
    }
    return *this;
}

bool TreeView::isMultiSelect() const {
    return m_treeData->multiSelectEnabled;
}

TreeView& TreeView::select(const std::string& id, bool addToSelection) {
    if (!addToSelection || !m_treeData->multiSelectEnabled) {
        m_treeData->clearSelectionRecursive(m_treeData->nodes);
        m_treeData->selectedIds.clear();
    }
    
    if (auto* node = findNode(id)) {
        node->selected = true;
        
        auto it = std::find(m_treeData->selectedIds.begin(), m_treeData->selectedIds.end(), id);
        if (it == m_treeData->selectedIds.end()) {
            m_treeData->selectedIds.push_back(id);
        }
        
        if (m_treeData->onSelectCallback) {
            m_treeData->onSelectCallback(*node);
        }
    }
    
    return *this;
}

TreeView& TreeView::deselect(const std::string& id) {
    if (auto* node = findNode(id)) {
        node->selected = false;
        
        auto it = std::find(m_treeData->selectedIds.begin(), m_treeData->selectedIds.end(), id);
        if (it != m_treeData->selectedIds.end()) {
            m_treeData->selectedIds.erase(it);
        }
    }
    return *this;
}

TreeView& TreeView::clearSelection() {
    m_treeData->clearSelectionRecursive(m_treeData->nodes);
    m_treeData->selectedIds.clear();
    return *this;
}

std::vector<std::string> TreeView::getSelectedIds() const {
    return m_treeData->selectedIds;
}

std::vector<const TreeNode*> TreeView::getSelectedNodes() const {
    std::vector<const TreeNode*> result;
    for (const auto& id : m_treeData->selectedIds) {
        if (const auto* node = findNode(id)) {
            result.push_back(node);
        }
    }
    return result;
}

// Drag and Drop
TreeView& TreeView::dragDropEnabled(bool enabled) {
    m_treeData->dragDropEnabled = enabled;
    return *this;
}

bool TreeView::isDragDropEnabled() const {
    return m_treeData->dragDropEnabled;
}

TreeView& TreeView::moveNode(const std::string& nodeId, const std::string& newParentId, int index) {
    // Find and copy the node
    TreeNode* sourceNode = findNode(nodeId);
    if (!sourceNode) return *this;
    
    TreeNode nodeCopy = *sourceNode;
    
    // Remove from current location
    m_treeData->removeNodeRecursive(m_treeData->nodes, nodeId);
    
    // Add to new location
    if (newParentId.empty()) {
        // Add to root
        if (index < 0 || index >= static_cast<int>(m_treeData->nodes.size())) {
            m_treeData->nodes.push_back(nodeCopy);
        } else {
            m_treeData->nodes.insert(m_treeData->nodes.begin() + index, nodeCopy);
        }
    } else {
        // Add to parent
        if (auto* parent = findNode(newParentId)) {
            if (index < 0 || index >= static_cast<int>(parent->children.size())) {
                parent->children.push_back(nodeCopy);
            } else {
                parent->children.insert(parent->children.begin() + index, nodeCopy);
            }
        }
    }
    
    return *this;
}

// Appearance
TreeView& TreeView::nodeHeight(float height) {
    m_treeData->nodeHeight = height;
    return *this;
}

float TreeView::getNodeHeight() const {
    return m_treeData->nodeHeight;
}

TreeView& TreeView::indentation(float indent) {
    m_treeData->indentation = indent;
    return *this;
}

float TreeView::getIndentation() const {
    return m_treeData->indentation;
}

TreeView& TreeView::showExpandIcons(bool show) {
    m_treeData->showExpandIcons = show;
    return *this;
}

bool TreeView::hasExpandIcons() const {
    return m_treeData->showExpandIcons;
}

TreeView& TreeView::showLines(bool show) {
    m_treeData->showLines = show;
    return *this;
}

bool TreeView::hasLines() const {
    return m_treeData->showLines;
}

TreeView& TreeView::hoverColor(const Color& color) {
    m_treeData->hoverColor = color;
    return *this;
}

const Color& TreeView::getHoverColor() const {
    return m_treeData->hoverColor;
}

TreeView& TreeView::selectionColor(const Color& color) {
    m_treeData->selectionColor = color;
    return *this;
}

const Color& TreeView::getSelectionColor() const {
    return m_treeData->selectionColor;
}

TreeView& TreeView::animateExpand(bool enabled) {
    m_treeData->animateExpand = enabled;
    return *this;
}

bool TreeView::hasExpandAnimation() const {
    return m_treeData->animateExpand;
}

// Custom Rendering
TreeView& TreeView::nodeRenderer(NodeRenderer renderer) {
    m_treeData->customRenderer = std::move(renderer);
    return *this;
}

// Callbacks
TreeView& TreeView::onSelect(std::function<void(const TreeNode&)> callback) {
    m_treeData->onSelectCallback = std::move(callback);
    return *this;
}

TreeView& TreeView::onExpand(std::function<void(const TreeNode&, bool)> callback) {
    m_treeData->onExpandCallback = std::move(callback);
    return *this;
}

TreeView& TreeView::onDoubleClick(std::function<void(const TreeNode&)> callback) {
    m_treeData->onDoubleClickCallback = std::move(callback);
    return *this;
}

TreeView& TreeView::onDragStart(std::function<void(const TreeNode&)> callback) {
    m_treeData->onDragStartCallback = std::move(callback);
    return *this;
}

TreeView& TreeView::onDrop(std::function<void(const TreeDragData&)> callback) {
    m_treeData->onDropCallback = std::move(callback);
    return *this;
}

// Scrolling
TreeView& TreeView::scrollToNode(const std::string& id) {
    // Calculate node position and scroll to it
    // This would require traversing the tree and counting visible nodes
    // For now, just find the node depth
    if (findNode(id)) {
        // Implementation would calculate actual position
    }
    return *this;
}

float TreeView::getScrollOffset() const {
    return m_treeData->scrollOffset;
}

TreeView& TreeView::scrollTo(float offset) {
    m_treeData->scrollOffset = std::max(0.0f, offset);
    return *this;
}

} // namespace KillerGK
