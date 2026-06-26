#pragma once

#include <Geode/Geode.hpp>
#include "lib/Utilities.hpp"
#include "lib/Geometry2.hpp"
#include "lib/UINode.hpp"

using namespace geode::prelude;

namespace Sculptor {

class VectorEditor;

class TreeNode : public CCObject {
public:

    static TreeNode* create(CCPoint position, VectorEditor* editor, TreeNode* parent = nullptr);

    TreeNode(CCPoint position, VectorEditor* editor, TreeNode* parent = nullptr);

    UINode* node;
    std::vector<UINode*> controlNodes;
    TreeNode* parent;
    std::vector<Ref<TreeNode>> children;
    VectorEditor* editor;

    void setNewParent(TreeNode* newParent);

    int getDegree();
    int getDepth();
    BezierCurve asBezierCurve();

    int getControlNodeID(UINode* node) const {
        return std::ranges::contains(controlNodes, node) ? std::ranges::find(controlNodes, node) - controlNodes.begin() : -1;
    }
    UINode* addControlNode(CCPoint position, UINode::Type type, std::optional<int> index = std::nullopt);
    void clearControlNodes();		

    void startDragRecursive(std::optional<int> index = std::nullopt);		

};

}