#include <Geode/Geode.hpp>
#include <ranges>
#include "mod/SelectionPopup.hpp"
#include "lib/Grid.hpp"

using namespace geode::prelude;

namespace Sculptor {

	bool SelectionPopup::init(CCPoint position, CCSize size, std::vector<CCSprite*> sprites, std::function<void(int)> callback) {
		if (!Popup::init(size))	return false;

		this->callback = callback;
		setCloseButtonSpr(CCSprite::create(), 0);
		m_mainLayer->setPosition(position);

		auto grid = Grid::create(size, Direction::Vertical, 3);

		for (const auto& [i, sprite] : std::views::enumerate(sprites)) {
			auto button = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(SelectionPopup::onButton));
			button->setTag(i);
			grid->addElement(button);
		}

		m_mainLayer->addChild(grid);

		return true;
	}

}