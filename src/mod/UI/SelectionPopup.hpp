#pragma once

#include <Geode/Geode.hpp>
#include "lib/Utilities.hpp"

using namespace geode::prelude;

namespace Sculptor {

	class Grid;

	class SelectionPopup : public geode::Popup {
	public:
		static SelectionPopup* create(CCPoint position, CCSize size, int columns, int rows, int totalElementCount, std::function<CCSprite*(int)> spriteFunc, std::function<void(int)> callback) {
			auto node = new SelectionPopup();
			if (node->init(position, size, columns, rows, totalElementCount, spriteFunc, callback)) {
				node->autorelease();
				return node;
			}
			delete node;
			return nullptr;
		}

		bool init(CCPoint position, CCSize size, int columns, int rows, int totalElementCount, std::function<CCSprite*(int)> spriteFunc, std::function<void(int)> callback);

		void onButton(CCObject* sender) {
			if (callback) {
				callback(sender->getTag());
			}
			onClose(nullptr);
		}

	private:

		std::function<void(int)> callback;

		void updateUI();

		int page = 0;
		int columns;
		int rows;
		int totalElementCount;
		std::function<CCSprite*(int)> spriteFunc;

		Grid* grid;
		CCMenuItemSpriteExtra* leftButton;
		CCMenuItemSpriteExtra* rightButton;

		void onPageLeftButton(CCObject* sender);
		void onPageRightButton(CCObject* sender);

	};

}