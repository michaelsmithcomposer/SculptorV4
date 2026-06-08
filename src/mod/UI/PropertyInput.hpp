#pragma once

#include <Geode/Geode.hpp>
#include "lib/Utilities.hpp"

using namespace geode::prelude;

namespace Sculptor {
	
	class Property;

	class PropertyInput : public SculptorNode<PropertyInput> {
	public:

		void setup(Property* property, CCSize size);

		void setProperty(Property* property);
		
		void setColor(const ccColor3B color) { baseColor = color; }
		void setHighlightColor(const ccColor3B color) { highlightColor = color; }
		void setBaseVisible(bool visible) { base->setVisible(visible); }

	private:
		
		Property* property;

		TextInput* valueInput;
		NineSlice* base;
		CCLabelBMFont* label = CCLabelBMFont::create("", "chatFont.fnt");
		
		std::optional<int> focusIndex;
		bool typing = false;

		ListenerHandle mouseListener;
		ListenerHandle scrollListener;

		ccColor3B baseColor = { 255, 255, 255 };
		ccColor3B highlightColor = { 155, 155, 155 };

		void update(float dt);

		void reportValue(float value);
		void setStringFromValue();
		void setValueFromString();

		void updateFocusIndex();

		ListenerResult handleMouseData(MouseInputData data);
		ListenerResult handleScroll(double x, double y);

		int digitToStringIndex(int i);
		float magnitudeAtIndex(int i);
		CCFontSprite* spriteAtIndex(int i);


	};

}


