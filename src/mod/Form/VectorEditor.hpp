#pragma once

#include <Geode/Geode.hpp>
#include "lib/Utilities.hpp"
#include "lib/Geometry2.hpp"
#include "lib/UINode.hpp"

using namespace geode::prelude;

namespace Sculptor {

	class Form;

	class TreeNode;

	class VectorEditor : public SculptorNode<VectorEditor, CCDrawNode> {
	public:
			
		void setup();
		static VectorEditor* createDefault(CCPoint position);		
		void onExit();		

		std::vector<TreeNode*> nodes;
		UINode* mouseNode;
		TreeNode* root;
		Form* form;			
		std::vector<GameObject*> proxyObjects;

		bool showPath = false;
		bool showControlPath = false;
		bool showHitbox = true;

		Sequences getPaths();
		Sequences getControlPaths();
		Polys getDecomposition(float inflate);
		Triangles getTriangulation(float inflate);

		void removeTreeNode(TreeNode* treeNode);
		void dissolveTreeNode(TreeNode* treeNode);
		void removeSubtree(TreeNode* subRoot);

		void convertTreeNode(TreeNode* treeNode);
		void convertBezierNode(TreeNode* treeNode, UINode* node);

		int getTreeNodeID(TreeNode* node) const {
			return std::ranges::contains(nodes, node) ? std::ranges::find(nodes, node) - nodes.begin() : -1;
		}

		void startDragAll();
		void setAllVisible(bool visible);
		void redraw();
		

	private:

		void update(float dt);		

		void createMouseNode();

		std::optional<std::pair<TreeNode*, float>> nodeFromPathPosition(CCPoint position);
		std::optional<std::pair<TreeNode*, int>> nodeFromControlPathPosition(CCPoint position);
		
		void addTreeNodeAtPosition(CCPoint position);
		void addBezierNodeAtPosition(CCPoint position);		
		
		std::vector<UINode*> getAllUINodes();		

		PathD getSpanningPath(TreeNode* subRoot = nullptr);

		

		static constexpr float visibilityRadius = 4.25;

	};

	class TreeNode {
	public:

		TreeNode(CCPoint position, VectorEditor* editor, TreeNode* parent = nullptr);

		UINode* node;
		std::vector<UINode*> controlNodes;
		TreeNode* parent;
		std::vector<TreeNode*> children;
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