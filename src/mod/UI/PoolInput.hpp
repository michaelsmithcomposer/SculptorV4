#pragma once

#include <Geode/Geode.hpp>
#include "mod/UI/SelectionPopup.hpp"


using namespace geode::prelude;

namespace Sculptor {

	class ModulatablePropertyInput;
	class Property;
	class Grid;

	class PoolInput : public SculptorNode<PoolInput> {
	public:

		void setup(Property* property, CCSize size);

	private:

		Grid* grid;
		NineSlice* base;
		Property* property;
		CCLabelBMFont* label = CCLabelBMFont::create("", "chatFont.fnt");
		ModulatablePropertyInput* indexInput;
		SelectionPopup* selectPopup;

		CCNode* createButton(int ID, int tag);

		

		
		void onNewIDButton(CCObject* sender);
		void onDeleteIDButton(CCObject* sender);

	};

}