#include <Geode/Geode.hpp>
#include "mod/UI/PoolInput.hpp"
#include "mod/Property.hpp"
#include "mod/UI/ModulatablePropertyInput.hpp"
#include "mod/UI/UI.hpp"
#include "External/validObjectIDs.hpp"

using namespace geode::prelude;

namespace Sculptor {

	void PoolInput::setup(Property* property, CCSize size) {

		CCSize gridSize = { size.width - UI::elementSize.width, size.height };

		this->property = property;		
		
		setContentSize(size);
		setLayout(RowLayout::create()->setGap(3)->setAutoScale(false));
		
		auto node = CCNode::create();
		node->setContentSize(gridSize);
		base = createBase(gridSize);
		node->addChildAtPosition(base, Anchor::Center);

		grid = Grid::create(gridSize, Direction::Horizontal, 1, 0);
		node->addChild(grid);

		addChild(node);

		label->setString(property->info.label.c_str());
		label->setScale(0.6);
		node->addChildAtPosition(label, Anchor::Top, { 0, -7 });

		indexInput = ModulatablePropertyInput::create(property, UI::elementSize);
		addChild(indexInput);
		updateLayout();


		std::vector<CCNode*> elements;

		for (const auto& value : property->info.valuePool) {
			auto button = createButton(static_cast<int>(value), elements.size());			
			elements.push_back(button);
		}

		elements.push_back(CCMenuItemSpriteExtra::create(CCSprite::create("create.png"_spr), this, menu_selector(PoolInput::onNewIDButton)));

		grid->addElements(elements);

	}


	CCNode* PoolInput::createButton(int ID, int tag) {
		auto sprite = property->info.poolType == PoolType::ID ? objectButtonSpriteFromID(ID) : colorSpriteFromID(ID);
		auto menu = CCMenu::create();
		menu->setContentSize(sprite->getContentSize());
		auto button = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(PoolInput::onDeleteIDButton));
		button->setTag(tag);		
		menu->addChildAtPosition(button, Anchor::Center);
		return menu;
	}



	void PoolInput::onNewIDButton(CCObject* sender) {		
		int count = property->info.poolType == PoolType::ID ? validObjectIDs.size() : 20;
		selectPopup = SelectionPopup::create(CCDirector::sharedDirector()->getWinSize() / 2, { 250, 250 }, 7, 7, count,
			[&](int i) {
				return property->info.poolType == PoolType::ID ? objectButtonSpriteFromID(validObjectIDs.at(i)) : colorSpriteFromID(i + 1);
			},
			[&](int i) { 
				this->property->info.valuePool.push_back(property->info.poolType == PoolType::ID ? validObjectIDs.at(i) : (i + 1));
				UI::get()->updateUI();
			});
		selectPopup->show();
	}
	void PoolInput::onDeleteIDButton(CCObject* sender) {
		int i = sender->getTag();
		property->info.valuePool.erase(property->info.valuePool.begin() + i);
		UI::get()->updateUI();
	}

}