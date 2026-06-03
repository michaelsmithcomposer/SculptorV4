#pragma once

#include <Geode/Geode.hpp>
#include "lib/Utilities.hpp"

using namespace geode::prelude;

namespace Sculptor {

	class SelectionPopup : public geode::Popup {
	public:
		static SelectionPopup* create(CCPoint position, CCSize size, std::vector<CCSprite*> sprites, std::function<void(int)> callback) {
			auto node = new SelectionPopup();
			if (node->init(position, size, sprites, callback)) {
				node->autorelease();
				return node;
			}
			delete node;
			return nullptr;
		}

		bool init(CCPoint position, CCSize size, std::vector<CCSprite*> sprites, std::function<void(int)> callback);

		void onButton(CCObject* sender) {
			if (callback) {
				callback(sender->getTag());
			}
			onClose(nullptr);
		}

	private:

		std::function<void(int)> callback;

	};

}