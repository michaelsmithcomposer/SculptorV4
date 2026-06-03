#pragma once

#include <Geode/Geode.hpp>
#include "lib/Utilities.hpp"

using namespace geode::prelude;

namespace Sculptor {

	class UINode : public SculptorNode<UINode, CCLayer> {
	public:

		enum class Type {
			Vertex,
			Bezier
		};

		Type type;

		void setup(CCPoint position, Type type, bool canDrag = true);
		void onExit();

		void setType(Type type);

		void setMoveCallback(std::function<void(UINode*, CCPoint)> callback) { onMove = callback; }
		void setClickCallback(std::function<void(UINode*, CCPoint)> callback) { onClick = callback; }
		void setRightClickCallback(std::function<void(UINode*, CCPoint)> callback) { onRightClick = callback; }
		void setMiddleClickCallback(std::function<void(UINode*, CCPoint)> callback) { onMiddleClick = callback; }

		void startDrag();
		void stopDrag();

		void setActive(bool active);
		bool active = true;

	protected:


		CCSprite* sprite;

		bool canDrag;
		bool inDrag = false;
		CCPoint offset;
		CCPoint lastPosition;

		ListenerHandle mouseListener;
		std::function<void(UINode*, CCPoint)> onMove;
		std::function<void(UINode*, CCPoint)> onClick;
		std::function<void(UINode*, CCPoint)> onRightClick;
		std::function<void(UINode*, CCPoint)> onMiddleClick;

		void update(float dt);

		ListenerResult handleMouseData(MouseInputData data);
		bool ccTouchBegan(CCTouch* touch, CCEvent* event);

		inline static std::unordered_map<UINode::Type, std::string> textures = {
			{Type::Vertex, "warpBtn_02_001.png"},
			{Type::Bezier, "warpBtn_01_001.png"}
		};

	};
}