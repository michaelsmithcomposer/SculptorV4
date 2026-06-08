#include <Geode/Geode.hpp>
#include <ranges>
#include "mod/UI/SelectionPopup.hpp"
#include "lib/Grid.hpp"

using namespace geode::prelude;

namespace Sculptor {

	bool SelectionPopup::init(CCPoint position, CCSize size, int columns, int rows, int totalElementCount, std::function<CCSprite*(int)> spriteFunc, std::function<void(int)> callback) {
		if (!Popup::init(size))	return false;

		this->columns = columns;
		this->rows = rows;
		this->totalElementCount = totalElementCount;
		this->spriteFunc = spriteFunc;

		this->callback = callback;
		setCloseButtonSpr(CCSprite::create(), 0);
		m_mainLayer->setPosition(position);	

		grid = Grid::create(size, Direction::Vertical, columns);		
		
		m_mainLayer->addChild(grid);

		auto menu = CCMenu::create();
		menu->setContentSize(size);
		m_mainLayer->addChildAtPosition(menu, Anchor::Center);		

		leftButton = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png"), this, menu_selector(SelectionPopup::onPageLeftButton));		
		menu->addChildAtPosition(leftButton, Anchor::Left, {-25, 0});

		auto sprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
		sprite->setFlipX(true);
		rightButton = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(SelectionPopup::onPageRightButton));		
		menu->addChildAtPosition(rightButton, Anchor::Right, { 25, 0 });

		updateUI();

		return true;
	}

	void SelectionPopup::updateUI() {
		grid->clear();

		for (int i = page * (rows * columns); i < std::min((page + 1) * (rows * columns), totalElementCount); i++) {
			auto button = CCMenuItemSpriteExtra::create(spriteFunc(i), this, menu_selector(SelectionPopup::onButton));
			button->setTag(i);
			grid->addElement(button);
		}
		
		leftButton->setVisible(page > 0);
		rightButton->setVisible(page < floor(totalElementCount / (rows * columns)));
		
	}

	void SelectionPopup::onPageLeftButton(CCObject* sender) {
		page--;
		updateUI();
	}

	void SelectionPopup::onPageRightButton(CCObject* sender) {
		page++;
		updateUI();
	}
}
