#pragma once

#include <Geode/Geode.hpp>
#include "Utilities.hpp"

using namespace geode::prelude;

namespace Sculptor {

	class Grid : public SculptorNode<Grid> {
	public:

		void setup(CCSize size, Direction direction, int lanes, float gap = 0, std::optional<std::string> label = std::nullopt);
		void addElement(CCNode* element, bool scale = true);
		void addElements(std::vector<CCNode*> elements, bool scale = true);
		void removeElement(CCNode* element);
		void clear();

		int count = 0;
		Direction direction;
		CCNode* node;
		std::vector<CCMenu*> lanes;
		CCLabelBMFont* label;

		static constexpr float labelHeight = 11;

	};

}