#include <Geode/Geode.hpp>
#include "mod/Form/VectorEditor.hpp"
#include "mod/Form/Form.hpp"
#include "mod/Form/Modulator.hpp"
#include "mod/Serialization.hpp"
#include "mod/Manager.hpp"

using namespace geode::prelude;

namespace Sculptor {

	void VectorEditor::setup() {

		scheduleUpdate();
		setBlendFunc({ GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR });		

		createMouseNode();	

	}

	VectorEditor* VectorEditor::createDefault(CCPoint position) {
		auto editor = VectorEditor::create();

		editor->root = TreeNode::create(position, editor);

		return editor;
	}

	void VectorEditor::onExit() {
		CCNode::onExit();
	}

	void VectorEditor::createMouseNode() {
		mouseNode = UINode::create(ccp(0, 0), UINode::Type::Vertex, false);
		mouseNode->setClickCallback([this](auto node, auto position) {
			if (node->type == UINode::Type::Bezier) {
				addBezierNodeAtPosition(position);
			}
			else {
				addTreeNodeAtPosition(position);
			}
			});
		addChild(mouseNode);
	}

	void VectorEditor::update(float dt) {

		CCPoint projection;
		bool condition = false;
		bool isBezier = false;

		if (nodes.size() > 1) {
			auto mouse = convertToNodeSpace(getMousePos());

			auto pathProjection = projectOntoLines(mouse, sequencesToLines(getPaths()));
			auto ctrlProjection = projectOntoLines(mouse, sequencesToLines(getControlPaths()));

			isBezier = (ctrlProjection.getDistance(mouse) < pathProjection.getDistance(mouse)) || CCKeyboardDispatcher::get()->m_bControlPressed;
			projection = isBezier ? ctrlProjection : pathProjection;
			condition =	(form == Manager::get()->selectedForm &&
						mouse.getDistance(projection) < visibilityRadius) && 
						std::ranges::none_of(getAllUINodes(), [projection](UINode* n) { return n->getPosition().getDistance(projection) < visibilityRadius; });
		}

		if (!mouseNode->active && condition) {
			mouseNode->setActive(true);
		}
		else if (mouseNode->active && !condition) {
			mouseNode->setActive(false);
		}
		if (mouseNode->active) {
			if (isBezier && mouseNode->type == UINode::Type::Vertex) {
				mouseNode->setType(UINode::Type::Bezier);
			}
			else if (!isBezier && mouseNode->type == UINode::Type::Bezier) {
				mouseNode->setType(UINode::Type::Vertex);
			}
			mouseNode->setPosition(projection);
		}
		mouseNode->setScale(1 / LevelEditorLayer::get()->m_objectLayer->getScale());

		redraw();
	}

	void VectorEditor::redraw() {
		clear();
		if (nodes.size() <= 1) return;

		float scale = 1 / LevelEditorLayer::get()->m_objectLayer->getScale();
		

		if (showControlPath) {
			auto path = getControlPaths();
			for (const auto& seq : path) {
				if (seq.size() > 2) {					
					Points dashes = Sequence::dashed(seq, 4 * scale, 0.35).points();
					drawLines(dashes.data(), dashes.size(), 0.1 * scale, { 1.0, 1.0, 1.0, 0.15 });					
				}
			}
		}

		if (showPath) {
			for (const auto& poly : getDecomposition(0.1)) {
				Points points = poly.points();
				drawPolygon(points.data(), points.size(), { 0.0, 0.0, 0.0, 0.0 }, 0.1 * scale, { 1.0, 1.0, 1.0, 0.35 });
			}
		}

		if (showHitbox) {
			for (const auto& poly : getDecomposition(form->mode == FormMode::Open ? 10 : 1)) {				
				Points dashes = Sequence::dashed(poly, 4 * scale, 0.35).points();
				drawLines(dashes.data(), dashes.size(), 0.25 * scale, { 1.0, 1.0, 1.0, 0.15 });				
			}
		}	

	}

	Sequences VectorEditor::getPaths() {
		Sequences result;

		if (form->mode == FormMode::Closed) {		

			Poly poly;
			TreeNode* current = root;
			do {
				auto path = current->asBezierCurve().approximation();
				poly.append(path | std::views::take(path.size() - 1));
				current = current->children.front();
			} while (current != root);
			result = { poly.asSequence() };

		} else {

			for (const auto& n : nodes) {
				if (!n->parent) continue;
				result.push_back(n->asBezierCurve().approximation());
			}			
		}
		
		return result;
	}

	Sequences VectorEditor::getControlPaths() {
		Sequences result;
		for (const auto& n : nodes) {		
			if (!n->parent) continue;
			result.push_back(n->asBezierCurve());
		}		
		return result;
	}

	Polys VectorEditor::getDecomposition(float inflate) {
		using namespace Clipper2Lib;

		Polys result;
		PathsD unionPaths;

		if (form->mode == FormMode::Closed) {			
			unionPaths = InflatePaths(Union({ getPaths().front().path() }, FillRule::EvenOdd), inflate, JoinType::Miter, EndType::Polygon);
		}
		else {
			if (!(inflate > 0)) return {};	
			float tolerance = inflate * (1 - cos(M_PI / 4));
			unionPaths = Union(InflatePaths({ getSpanningPath() }, inflate, JoinType::Round, EndType::Round, 2, 2, 0.1), FillRule::EvenOdd);
		}

		for (const auto& path : unionPaths) {			
			result.push_back(Sequence::fromPath(path).points());
		}

		return result;

	}

	Triangles VectorEditor::getTriangulation(float inflate) {
		using namespace Clipper2Lib;

		Triangles result;
		PathsD triangles;

		if (form->mode == FormMode::Closed) {			
			Triangulate(InflatePaths(Union({ getPaths().front().path() }, FillRule::EvenOdd), inflate, JoinType::Miter, EndType::Polygon), 6, triangles);
		}
		else {
			if (!(inflate > 0)) return {};
			float tolerance = inflate * (1 - cos(M_PI / 4));
			Triangulate(Union(InflatePaths({ getSpanningPath() }, inflate, JoinType::Round, EndType::Round, 2, 2, tolerance), FillRule::EvenOdd), 6, triangles);
		}	

		for (const auto& path : triangles) {
			Triangle triangle{ Sequence::fromPath(path).points() };
			result.push_back(triangle);
			if (triangle.area() > 60) {
				result.push_back(Triangle({ triangle.b(), triangle.c(), triangle.a() }));
			}			
		}

		return result;

	}

	std::optional<std::pair<TreeNode*, float>> VectorEditor::nodeFromPathPosition(CCPoint position) {
		for (const auto& n : nodes) {
			if (!n->parent) continue;
			auto curve = n->asBezierCurve().approximation();
			if (curve.containsEdgePoint(position)) {
				return std::make_pair(n, *curve.inverseLerp(position));
			}
		}
		return std::nullopt;
	}

	std::optional<std::pair<TreeNode*, int>> VectorEditor::nodeFromControlPathPosition(CCPoint position) {
		for (const auto& n : nodes) {
			if (!n->parent) continue;
			auto curve = n->asBezierCurve();
			if (curve.containsEdgePoint(position)) {
				return std::make_pair(n, curve.edgeIndicesContaining(position).front());
			}
		}
		return std::nullopt;
	}

	void VectorEditor::addTreeNodeAtPosition(CCPoint position) {

		mouseNode->setActive(false);
		auto [node, t] = *nodeFromPathPosition(position);
		auto [l, r] = node->asBezierCurve().split(t);
		auto newNode = TreeNode::create(position, this, node->parent);

		
		node->setNewParent(newNode);		
		node->clearControlNodes();
		
		if (!alt()) {
			for (int i = 1; i < l.size() - 1; i++) {
				newNode->addControlNode(l.at(i), UINode::Type::Bezier);
				node->addControlNode(r.at(i), UINode::Type::Bezier);
			}
		}

		newNode->node->startDrag();	

	}

	void VectorEditor::addBezierNodeAtPosition(CCPoint position) {
		mouseNode->setActive(false);				
		auto [node, i] = *nodeFromControlPathPosition(position);	
		auto newNode = node->addControlNode(position, UINode::Type::Bezier, i);			
		newNode->startDrag();	
	}

	void VectorEditor::removeTreeNode(TreeNode* treeNode) {		
		if (root == treeNode && form->mode == FormMode::Closed) {
			root = treeNode->children.front();
		}
		treeNode->clearControlNodes();
		std::erase(treeNode->parent->children, treeNode);
		std::erase(nodes, treeNode);
		treeNode->node->removeFromParent();
		return;	
	}

	void VectorEditor::dissolveTreeNode(TreeNode* treeNode) {		
		auto child = treeNode->children.front();		
		child->clearControlNodes();
		child->parent = treeNode->parent;		
		treeNode->parent->children.push_back(child);		
		removeTreeNode(treeNode);	
	}	

	void VectorEditor::removeSubtree(TreeNode* subRoot) {
		while (!subRoot->children.empty()) {
			removeSubtree(subRoot->children.front());
		}	
		removeTreeNode(subRoot);
	}

	void VectorEditor::convertTreeNode(TreeNode* treeNode) {
		auto child = treeNode->children.front();
		child->setNewParent(treeNode->parent);		
		child->addControlNode(treeNode->node->getPosition(), UINode::Type::Bezier, 0);
		for (const auto& controlNode : treeNode->controlNodes) {
			child->addControlNode(controlNode->getPosition(), UINode::Type::Bezier , 0);
		}		
		removeTreeNode(treeNode);		
	}

	void VectorEditor::convertBezierNode(TreeNode* treeNode, UINode* node) {				
		int index = std::distance(treeNode->controlNodes.begin(), std::ranges::find(treeNode->controlNodes, node));
		auto before = treeNode->controlNodes | std::views::take(index) | std::ranges::to<std::vector>();
		auto after = treeNode->controlNodes | std::views::drop(index + 1) | std::ranges::to<std::vector>();
		
		auto newNode = TreeNode::create(node->getPosition(), this, treeNode->parent);
		newNode->controlNodes = before;

		treeNode->setNewParent(newNode);
		treeNode->controlNodes = after;

		node->removeFromParent();
	}

	void VectorEditor::startDragAll() {
		for (auto& treeNode : nodes) {
			treeNode->node->startDrag();
			for (auto& controlNode : treeNode->controlNodes) {
				controlNode->startDrag();
			}
		}
	}

	void VectorEditor::setAllVisible(bool visible) {
		for (auto& node : getAllUINodes()) {
			node->setVisible(visible);
		}
	}

	std::vector<UINode*> VectorEditor::getAllUINodes() {
		std::vector<UINode*> result;
		for (const auto& treeNode : nodes) {
			result.push_back(treeNode->node);
			for (const auto& controlNode : treeNode->controlNodes) {
				result.push_back(controlNode);
			}
		}
		return result;
	}

	PathD VectorEditor::getSpanningPath(TreeNode* subRoot) {
		PathD result;

		if (!subRoot) {
			subRoot = root;
		}

		for (TreeNode* child : subRoot->children) {
			
			PathD curve = child->asBezierCurve().approximation().path();
			result.append_range(curve);
				
			PathD subtree = getSpanningPath(child);
			result.append_range(subtree);
			
			result.append_range(std::views::reverse(curve));
		}

		return result;
	}

	int VectorEditor::getTreeNodeID(TreeNode* treeNode) const {
		int index = 0;
		for (auto current : nodes) {
			if (current == treeNode) {
				return index;
			}
			++index;
		}
		return -1;
	}

}