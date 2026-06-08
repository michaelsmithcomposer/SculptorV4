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
		setZOrder(200);
		keyboardListener = KeyboardInputEvent().listen([this](KeyboardInputData data) { this->handleKeyboardData(data); });

		createMouseNode();	

	}

	VectorEditor* VectorEditor::createDefault(CCPoint position) {
		auto editor = VectorEditor::create();

		editor->root = new TreeNode(position, editor);

		return editor;
	}



	void VectorEditor::onExit() {
		for (auto node : nodes) {
			delete node;
		}
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
						std::ranges::none_of(nodes, [projection](TreeNode* n) { return n->node->getPosition().getDistance(projection) < visibilityRadius; });
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

		redraw();
	}

	void VectorEditor::redraw() {
		clear();
		if (nodes.size() <= 1) return;

		if (showControlPath) {
			auto path = getControlPaths();
			for (const auto& seq : path) {
				if (seq.size() > 2) {					
					Points dashes = Sequence::dashed(seq, 4, 0.35).points();
					drawLines(dashes.data(), dashes.size(), 0.1, { 1.0, 1.0, 1.0, 0.15 });					
				}
			}
		}

		if (showPath) {
			for (const auto& poly : getDecomposition(0.1)) {
				Points points = poly.points();
				drawPolygon(points.data(), points.size(), { 0.0, 0.0, 0.0, 0.0 }, 0.1, { 1.0, 1.0, 1.0, 0.35 });
			}
		}

		if (showHitbox) {
			for (const auto& poly : getDecomposition(form->mode == FormMode::Open ? 10 : 1)) {				
				Points dashes = Sequence::dashed(poly, 4, 0.35).points();
				drawLines(dashes.data(), dashes.size(), 0.25, { 1.0, 1.0, 1.0, 0.15 });				
			}
		}
	}


	ListenerResult VectorEditor::handleKeyboardData(KeyboardInputData data) {
		
		if (data.action == KeyboardInputData::Action::Press) {
			if (data.modifiers & KeyboardModifier::Shift) {
				for (const auto& node : getAllUINodes()) {
					proxyObjects.push_back(EditorUI::get()->createObject(objectIDs[GDObject::QuarterSquare], toEditorSpace(convertToWorldSpace(node->getPosition()))));
				}
			}			
		}
		else if (data.action == KeyboardInputData::Action::Release) {
			for (auto& object : proxyObjects) {
				EditorUI::get()->deleteObject(object, false);
			}
			proxyObjects.clear();
		}
		

		return ListenerResult::Propagate;
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
		auto newNode = new TreeNode(position, this, node->parent);

		
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
		delete treeNode;
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
		
		auto newNode = new TreeNode(node->getPosition(), this, treeNode->parent);
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

	//

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
					auto newNode = new TreeNode(position, editor, this);
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
		controlNodes.insert(controlNodes.begin() + index.value_or(controlNodes.size()), newNode);
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
		for (int i = index.value_or(controlNodes.size()); i < controlNodes.size(); i++) {
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