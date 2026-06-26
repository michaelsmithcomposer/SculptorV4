#include "TreeNode.hpp"
#include "VectorEditor.hpp"
#include "Form.hpp"
#include "Modulator.hpp"

namespace Sculptor {

TreeNode* TreeNode::create(CCPoint position, VectorEditor* editor, TreeNode* parent) {
    auto ret = new TreeNode(position, editor, parent);
    ret->autorelease();
    return ret;
}

TreeNode::TreeNode(CCPoint position, VectorEditor* editor, TreeNode* parent) : editor(editor), parent(parent) {
    node = UINode::create(position, UINode::Type::Vertex);
    if (parent) {
        parent->children.push_back(this);
    }
    editor->nodes.push_back(this);
    editor->addChild(node);

    node->setMoveCallback([this, editor](auto uiNode, auto position) {
        editor->form->dirty = true;
    });

    node->setClickCallback([this, editor](auto uiNode, auto position) {
        if (alt()) {
            if (editor->form->mode == FormMode::Open) {
                auto newNode = TreeNode::create(position, editor, this);
                uiNode->stopDrag();
                newNode->node->startDrag();
            }
            else {
                editor->convertTreeNode(this);
                editor->form->dirty = true;
            }
        }
    });
    node->setRightClickCallback([this, editor](auto uiNode, auto position) {
        if (!this->parent) return;
        if (editor->root == this && editor->form->mode == FormMode::Open) return;
        switch (this->getDegree()) {
        case 1:	editor->removeTreeNode(this); break;
        case 2:	editor->dissolveTreeNode(this); break;
        default: editor->removeSubtree(this); break;
        }
    });
    node->setMiddleClickCallback([this, editor](auto uiNode, auto position) {
        if (editor->form->mode == FormMode::Open) {
            this->startDragRecursive();
        }
        else {
            editor->startDragAll();
        }
    });
}

int TreeNode::getDegree() {
    return children.size() + 1;
}

int TreeNode::getDepth() {
    if (parent) {
        return parent->getDepth() + 1;
    }
    else {
        return 0;
    }
    
}

UINode* TreeNode::addControlNode(CCPoint position, UINode::Type type, std::optional<int> index) {
    auto newNode = UINode::create(position, type);
    editor->addChild(newNode);

    newNode->setMoveCallback([this](auto uiNode, auto position) {
        editor->form->dirty = true;
    });
    newNode->setClickCallback([this](UINode* uiNode, auto position) {
        if (alt()) {
            editor->convertBezierNode(this, uiNode);
            editor->form->dirty = true;
        }
    });
    newNode->setRightClickCallback([this](UINode* uiNode, auto position) {
        uiNode->removeFromParent();
        std::erase(controlNodes, uiNode);
        editor->form->dirty = true;
    });	
    newNode->setMiddleClickCallback([this](auto uiNode, auto position) {
        if (editor->form->mode == FormMode::Open) {
            this->startDragRecursive(this->getControlNodeID(uiNode));
        }
        else {
            editor->startDragAll();
        }
    });
    controlNodes.insert(controlNodes.begin() + (index.has_value() ? index.value() : controlNodes.size()), newNode);
    return newNode;
}

void TreeNode::setNewParent(TreeNode* newParent) {
    if (parent) {
        std::erase(parent->children, this);
    }		
    newParent->children.push_back(this);
    this->parent = newParent;
}

void TreeNode::startDragRecursive(std::optional<int> index) {
    node->startDrag();
    for (auto& child : children) {
        child->startDragRecursive();
        for (auto& controlNode : child->controlNodes) {
            controlNode->startDrag();
        }
    }
    for (int i = (index.has_value() ? index.value() : controlNodes.size()); i < controlNodes.size(); i++) {
        controlNodes[i]->startDrag();
    }
}

BezierCurve TreeNode::asBezierCurve() {
    if (!parent) return {};
    BezierCurve curve;
    curve.push_back(parent->node->getPosition());
    for (const auto& controlNode : controlNodes) {
        curve.push_back(controlNode->getPosition());
    }
    curve.push_back(node->getPosition());
    curve.quality = editor->form->quality.evaluate({});
    return curve;
}

void TreeNode::clearControlNodes() {
    for (auto& node : controlNodes) {
        node->removeFromParent();
    }
    controlNodes.clear();
}

}