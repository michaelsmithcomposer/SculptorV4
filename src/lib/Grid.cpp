#include <Geode/Geode.hpp>
#include "lib/Grid.hpp"

using namespace geode::prelude;

namespace Sculptor {

	void Grid::setup(CCSize size, Direction direction, int lanes, float gap, std::optional<std::string> label) {
		setContentSize(size);
		setLayout(ColumnLayout::create()->setGap(0)->setAxisReverse(true)->setAxisAlignment(AxisAlignment::End)->setAutoScale(false));
		this->direction = direction;
		
		node = CCNode::create();
		CCSize nodeSize;

		if (label) {
			nodeSize = CCSize{ size.width, size.height - labelHeight };
			this->label = CCLabelBMFont::create((*label).c_str(), "chatFont.fnt");
			this->label->setScale(labelHeight / this->label->getContentHeight());
			addChild(this->label);
		}
		else {
			nodeSize = size;
		}		
		node->setContentSize(nodeSize);
		addChild(node);
		updateLayout();

		if (direction == Direction::Horizontal) {
			node->setLayout(ColumnLayout::create()->setGap(0)->setAxisReverse(true)->setAxisAlignment(AxisAlignment::Even));
		}
		else {
			node->setLayout(RowLayout::create()->setGap(0)->setAxisAlignment(AxisAlignment::Even));
		}
		for (int i = 0; i < lanes; i++) {
			auto lane = CCMenu::create();
			if (direction == Direction::Horizontal) {
				lane->setContentSize({ nodeSize.width, nodeSize.height / lanes });
				lane->setLayout(RowLayout::create()->setGap(gap)->setAxisAlignment(AxisAlignment::Start)->setAutoScale(false));
			}
			else {
				lane->setContentSize({ nodeSize.width / lanes, nodeSize.height });
				lane->setLayout(ColumnLayout::create()->setGap(gap)->setAxisReverse(true)->setAxisAlignment(AxisAlignment::End)->setAutoScale(false));
			}
			this->lanes.push_back(lane);
			node->addChild(lane);
		}
		node->updateLayout();

		
	}

	void Grid::addElement(CCNode* element, bool scale) {
		if (scale) {
			if (direction == Direction::Horizontal) {
				element->setScale((node->getContentHeight() / lanes.size()) / element->getContentHeight());
			}
			else {
				element->setScale((node->getContentWidth() / lanes.size()) / element->getContentWidth());
			}
		}
		auto lane = lanes[count % lanes.size()];
		lane->addChild(element);
		lane->updateLayout();
		node->updateLayout();
		count++;
	}

	void Grid::addElements(std::vector<CCNode*> elements, bool scale) {

		int perLane = std::ceil(static_cast<float>(elements.size()) / lanes.size());
		float w, h;
		if (direction == Direction::Horizontal) {
			w = node->getContentWidth() / perLane;
			h = node->getContentHeight() / lanes.size();
		}
		else {
			w = node->getContentWidth() / lanes.size();
			h = node->getContentHeight() / perLane;
		}
		for (auto& element : elements) {
			if (scale) {
				element->setScale(std::min(w / element->getContentWidth(), h / element->getContentHeight()));
			}
			addElement(element, false);
		}
	}

	void Grid::removeElement(CCNode* element) {
		for (auto& lane : lanes) {
			if (element->hasAncestor(lane)) {
				element->removeFromParent();
				lane->updateLayout();
			}
		}
		count--;
	}

	void Grid::clear() {
		for (auto& lane : lanes) {
			lane->removeAllChildren();
		}
		count = 0;
	}
}
