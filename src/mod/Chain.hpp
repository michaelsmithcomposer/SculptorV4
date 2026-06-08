#pragma once

#include <Geode/Geode.hpp>
#include "lib/Geometry2.hpp"
#include "lib/Utilities.hpp"

using namespace geode::prelude;

namespace Sculptor {

	float getChainEndT(const Sequence& sequence, float t, float length);

	void writeObjectConnectionsJson(std::unordered_map<int, std::vector<CCPoint>> map);

	geode::Result<std::unordered_map<int, std::vector<CCPoint>>> readObjectConnectionsJson();

	//class ConnectionTagger : public SculptorNode<ConnectionTagger> {
	//public:
	//	void setup();

	//	CCDrawNode* drawNode;
	//	CCSprite* sprite;
	//	CCLabelBMFont* label = CCLabelBMFont::create("1", "chatFont.fnt");

	//	ListenerHandle mouseListener;
	//	ListenerHandle keyboardListener;

	//	float scale = 3;
	//	int ID = 1;
	//	std::unordered_map<int, std::vector<CCPoint>> map;

	//	ListenerResult handleMouseData(MouseInputData data);
	//	ListenerResult handleKeyboardData(KeyboardInputData data);

	//	void updateUI();
	//};

}