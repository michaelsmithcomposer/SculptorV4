#include <Geode/Geode.hpp>
#include <alphalaneous.editortab_api/include/EditorTabAPI.hpp>
#include "mod/UI.hpp"
#include "mod/Manager.hpp"
#include "mod/Modulator.hpp"
#include "mod/Layer.hpp"
#include "mod/VectorEditor.hpp"
#include "mod/Form.hpp"

using namespace geode::prelude;

namespace Sculptor {


	void UI::setup() {		
		instance = this;	

		mouseListener = MouseInputEvent().listen([this](MouseInputData data) { return this->handleMouseData(data); });

		sculptorPanel = SculptorPanel::create();


		updateUI();
	}

	void UI::onModeEnter() {
		if (!UI::get()->page) {
			UI::get()->createTab();
		}
		UI::get()->setGameUIVisible(false);
		updateUI();
	}

	void UI::onModeExit() {
		Manager::get()->deselect();
		UI::get()->setGameUIVisible(true);
		updateUI();
	}

	void UI::createTab() {
		{
			tab = EditorUI::get()->getChildByIDRecursive("sculptor-tab-bar"_spr)->getChildByType<BoomScrollLayer>(0);

			page = CCNode::create();
			page->setID("page"_spr);
			page->setAnchorPoint({ 0.f, 0.f });
			page->setPositionX(-96);
			page->setContentSize(pageSize);

			columns = CCNode::create();
			columns->setContentSize(pageSize);
			columns->setLayout(RowLayout::create()->setGap(0)->setAxisAlignment(AxisAlignment::Start)->setAutoScale(false)->setCrossAxisOverflow(false));
			page->addChild(columns);


			tab->addChild(page);
		}	

		layersPanel = LayersPanel::create();
		layerTabsPanel = LayerTabsPanel::create();
		layerPropertiesPanel = LayerPropertiesPanel::create();
		modulatorsPanel = ModulatorsPanel::create();
		modulatorPropertiesPanel = ModulatorPropertiesPanel::create();
		//formSettingsPanel = FormSettingsPanel::create();			

		columns->updateLayout();
	}

	CCMenu* UI::createComponentButton(CCSprite* sprite, bool selected, CCObject* target, SEL_MenuHandler selectCallback, SEL_MenuHandler deleteCallback, int tag) {
		auto menu = CCMenu::create();

		auto button = CCMenuItemSpriteExtra::create(sprite, target, selectCallback);
		button->setTag(tag);

		menu->setContentSize(button->getContentSize());
		menu->setTouchPriority(0);

		if (selected) {
			auto deleteMenu = CCMenu::create();

			auto spr = CCSprite::create("delete.png"_spr);
			spr->setScale(0.3);

			deleteMenu->setContentSize(spr->getScaledContentSize());
			deleteMenu->setAnchorPoint({ 1, 1 });
			deleteMenu->setTouchPriority(-1);
			deleteMenu->setZOrder(1);
			deleteMenu->addChildAtPosition(CCMenuItemSpriteExtra::create(spr, target, deleteCallback), Anchor::Center);

			menu->addChildAtPosition(deleteMenu, Anchor::TopRight);
		}
		else {
			button->setOpacity(50);
		}
		menu->addChildAtPosition(button, Anchor::Center);
		return menu;
	}

	void UI::updateUI() {

		sculptorPanel->updateUI();

		if (page) {			
			layersPanel->updateUI();
			layerTabsPanel->updateUI();
			layerPropertiesPanel->updateUI();
			modulatorsPanel->updateUI();
			modulatorPropertiesPanel->updateUI();
			//formSettingsPanel->updateUI();
		}
	

	}

	ListenerResult UI::handleMouseData(MouseInputData data) {
		if (!inTab()) return ListenerResult::Propagate;

		if (data.action == MouseInputData::Action::Press) {
			auto selected = Manager::get()->selectedForm;
			if (selected && selected->hitboxContains(selected->convertToNodeSpace(getMousePos()))) return ListenerResult::Propagate;
			for (const auto& form : Manager::get()->forms) {
				if (form->hitboxContains(form->convertToNodeSpace(getMousePos()))) {
					Manager::get()->selectForm(form);
					break;
				}
			}
		}
		return ListenerResult::Propagate;
	}

	
	#pragma region SculptorPanel

	void SculptorPanel::setup() {			

		grid = Grid::create(size, Direction::Horizontal, 1, 3);		
		addChildAtPosition(grid, Anchor::Center);

		EditorUI::get()->addChild(this);
		setPosition(60, 240);
		setScale(0.6);
	}

	void SculptorPanel::updateUI() {

		grid->clear();

		if (alpha::editor_tabs::getCurrentMode().unwrap() == "sculptor") {			
			grid->addElement(CCMenuItemSpriteExtra::create(CCSprite::create("return.png"_spr), this, menu_selector(SculptorPanel::onReturnButton)));
			grid->addElement(CCMenuItemSpriteExtra::create(CCSprite::create("form_closed.png"_spr), this, menu_selector(SculptorPanel::onNewClosedFormButton)));
			grid->addElement(CCMenuItemSpriteExtra::create(CCSprite::create("form_open.png"_spr), this, menu_selector(SculptorPanel::onNewOpenFormButton)));

			auto sprite = CCSprite::create("delete.png"_spr);
			if (Manager::get()->selectedForm) {
				grid->addElement(CCMenuItemSpriteExtra::create(sprite, this, menu_selector(SculptorPanel::onDeleteFormButton)));
			}
			else {
				sprite->setOpacity(50);
				grid->addElement(sprite);
			}
			
		}
		else {
			grid->addElement(CCMenuItemSpriteExtra::create(CCSprite::create("logo.png"_spr), this, menu_selector(SculptorPanel::onModeButton)));
		}

	}

	void SculptorPanel::onModeButton(CCObject* sender) {		
		alpha::editor_tabs::switchMode("sculptor");
		UI::get()->onModeEnter();
	}

	void SculptorPanel::onReturnButton(CCObject* sender) {
		alpha::editor_tabs::switchMode(alpha::editor_tabs::BUILD);
		UI::get()->onModeExit();
	}
	void SculptorPanel::onNewClosedFormButton(CCObject* sender) {
		auto form = Form::createDefault(FormMode::Closed, toEditorSpace(UI::midpoint()));
		Manager::get()->registerForm(form);
		Manager::get()->selectForm(form);
	}
	void SculptorPanel::onNewOpenFormButton(CCObject* sender) {
		auto form = Form::createDefault(FormMode::Open, toEditorSpace(UI::midpoint()));
		Manager::get()->registerForm(form);
		Manager::get()->selectForm(form);
	}
	void SculptorPanel::onDeleteFormButton(CCObject* sender) {
		auto form = Manager::get()->selectedForm;
		Manager::get()->deselect();
		std::erase(Manager::get()->forms, form);
		form->removeFromParent();			
	}

	#pragma endregion

	#pragma region LayersPanel

	void LayersPanel::setup() {		
		setContentSize(size);

		base = createBase(size, {-1, 0});
		addChildAtPosition(base, Anchor::Center);

		grid = Grid::create(size, Direction::Vertical, 2, 0, "Layers");
		addChild(grid);

		UI::get()->columns->addChild(this);
	}

	void LayersPanel::updateUI() {
		grid->clear();
		if (!Manager::get()->selectedLayer) return;

		for (const auto& [i, layer] : std::views::enumerate(Manager::get()->selectedForm->layers)) {
			grid->addElement(UI::createComponentButton(
				CCSprite::create(layer->spritePath.c_str()), 
				(layer == Manager::get()->selectedLayer),
				this, 
				menu_selector(LayersPanel::onSelectLayerButton), 
				menu_selector(LayersPanel::onDeleteLayerButton),
				i
			));
		}

		auto menu = CCMenu::create();
		auto createSprite = CCSprite::create("create.png"_spr);
		menu->setContentSize(createSprite->getContentSize());
		createSprite->setScale(0.66);
		menu->addChildAtPosition(CCMenuItemSpriteExtra::create(createSprite, this, menu_selector(LayersPanel::onNewLayerButton)), Anchor::Center);
		grid->addElement(menu);
	}

	void LayersPanel::onNewLayerButton(CCObject* sender) {
		std::vector<CCSprite*> sprites;
		for (const auto& type : Layer::prototypes()) {
			sprites.push_back(CCSprite::create(type->spritePath.c_str()));
		}
		selectPopup = SelectionPopup::create(CCDirector::sharedDirector()->getWinSize() / 2, { 100, 150 }, sprites, [&](int i) {
			auto layer = static_cast<Layer*>(Layer::prototypes()[i]->clone());
			Manager::get()->selectedForm->registerLayer(layer);
			Manager::get()->selectLayer(layer);
		});
		selectPopup->show();
	}

	void LayersPanel::onDeleteLayerButton(CCObject* sender) {
		auto form = Manager::get()->selectedForm;
		auto layer = Manager::get()->selectedLayer;
		//if (form->layers.size() <= 1) return;
		form->removeLayer(layer);
		
		if (!form->layers.empty()) Manager::get()->selectLayer(form->layers[0]);
	}

	void LayersPanel::onSelectLayerButton(CCObject* sender) {
		Manager::get()->selectLayer(Manager::get()->selectedForm->layers[sender->getTag()]);
	}

	#pragma endregion

	#pragma region LayerTabsPanel

	void LayerTabsPanel::setup() {
		setContentSize(size);

		base = createBase(size, { -1, 0 });
		addChildAtPosition(base, Anchor::Center);

		grid = Grid::create(size, Direction::Vertical, 1, 0);
		addChild(grid);

		UI::get()->columns->addChild(this);
	}

	void LayerTabsPanel::updateUI() {
		grid->clear();
		if (!Manager::get()->selectedLayer) return;

		std::vector<CCNode*> elements;
		for (int i = 0; i < 3; i++) {			
			auto button = CCMenuItemSpriteExtra::create(CCSprite::create("tab_symbol.png"_spr), this, menu_selector(LayerTabsPanel::onSelectTabButton));
			button->setTag(i);
			elements.push_back(button);			
		}
		grid->addElements(elements);
	}

	void LayerTabsPanel::onSelectTabButton(CCObject* sender) {
		tab = static_cast<LayerTab>(sender->getTag());
		UI::get()->updateUI();
	}

	#pragma endregion

	#pragma region LayerPropertiesPanel

	void LayerPropertiesPanel::setup() {
		setContentSize(size);

		base = createBase(size, { -1, 0 });
		addChildAtPosition(base, Anchor::Center);

		grid = Grid::create(size, Direction::Vertical, 6, 0);
		addChild(grid);

		UI::get()->columns->addChild(this);
	}

	void LayerPropertiesPanel::updateUI() {
		grid->clear();
		if (!Manager::get()->selectedLayer) return;

		auto layer = Manager::get()->selectedLayer;
		std::vector<CCNode*> elements;

		switch (UI::get()->layerTabsPanel->tab) {
			case LayerTab::Style:
				for (const auto& property : layer->getTypeProperties()) {
					elements.push_back(property->createUI(UI::elementSize));
				}
				break;
			case LayerTab::Offset:				
				for (const auto& property : layer->getBaseProperties()) {
					elements.push_back(property->createUI(UI::elementSize));
				}
				break;
			case LayerTab::Group:
				break;
		}

		grid->addElements(elements);
	}

	void LayerPropertiesPanel::onNewGroupButton(CCObject* sender) {

	}

	void LayerPropertiesPanel::onDeleteGroupButton(CCObject* sender) {

	}

	#pragma endregion

	#pragma region ModulatorsPanel

	void ModulatorsPanel::setup() {
		setContentSize(size);

		base = createBase(size, { -1, 0 });
		addChildAtPosition(base, Anchor::Center);

		grid = Grid::create(size, Direction::Vertical, 2, 0, "Modulators");
		addChild(grid);

		UI::get()->columns->addChild(this);
	}

	void ModulatorsPanel::updateUI() {
		grid->clear();
		if (!Manager::get()->selectedModulator) return;

		for (const auto& [i, modulator] : std::views::enumerate(Manager::get()->selectedForm->modulators)) {
			grid->addElement(UI::createComponentButton(
				CCSprite::create(modulator->spritePath.c_str()),
				(modulator == Manager::get()->selectedModulator),
				this,
				menu_selector(ModulatorsPanel::onSelectModulatorButton),
				menu_selector(ModulatorsPanel::onDeleteModulatorButton),
				i
			));
		}

		auto menu = CCMenu::create();
		auto createSprite = CCSprite::create("create.png"_spr);
		menu->setContentSize(createSprite->getContentSize());
		createSprite->setScale(0.66);
		menu->addChildAtPosition(CCMenuItemSpriteExtra::create(createSprite, this, menu_selector(ModulatorsPanel::onNewModulatorButton)), Anchor::Center);
		grid->addElement(menu);
	}

	void ModulatorsPanel::onNewModulatorButton(CCObject* sender) {
		std::vector<CCSprite*> sprites;
		for (const auto& type : Modulator::prototypes()) {
			sprites.push_back(CCSprite::create(type->spritePath.c_str()));
		}
		selectPopup = SelectionPopup::create(CCDirector::sharedDirector()->getWinSize() / 2, { 100, 150 }, sprites, [&](int i) {
			auto modulator = static_cast<Modulator*>(Modulator::prototypes()[i]->clone());
			Manager::get()->selectedForm->registerModulator(modulator);
			Manager::get()->selectModulator(modulator);
			});
		selectPopup->show();
	}

	void ModulatorsPanel::onDeleteModulatorButton(CCObject* sender) {
		auto form = Manager::get()->selectedForm;
		auto modulator = Manager::get()->selectedModulator;
		//if (form->modulators.size() <= 1) return;
		form->removeModulator(modulator);
	
		if (!form->modulators.empty()) Manager::get()->selectModulator(form->modulators[0]);
	}

	void ModulatorsPanel::onSelectModulatorButton(CCObject* sender) {
		Manager::get()->selectModulator(Manager::get()->selectedForm->modulators[sender->getTag()]);
	}

	#pragma endregion

	#pragma region ModulatorPropertiesPanel

	void ModulatorPropertiesPanel::setup() {
		setContentSize(size);

		base = createBase(size, { -1, 0 });
		addChildAtPosition(base, Anchor::Center);

		grid = Grid::create(size, Direction::Vertical, 2, 0);
		addChild(grid);

		UI::get()->columns->addChild(this);
	}

	void ModulatorPropertiesPanel::updateUI() {
		grid->clear();
		if (!Manager::get()->selectedModulator) return;

		auto modulator = Manager::get()->selectedModulator;
		std::vector<CCNode*> elements;
		
		for (const auto& property : modulator->getProperties()) {
			elements.push_back(property->createUI(UI::elementSize));
		}		

		grid->addElements(elements);
	}

	#pragma endregion

	#pragma region FormSettingsPanel

	void FormSettingsPanel::setup() {
		setContentSize(size);

		base = createBase(size, { -1, 0 });
		addChildAtPosition(base, Anchor::Center);

		grid = Grid::create(size, Direction::Vertical, 1, 0);
		addChild(grid);

		UI::get()->columns->addChild(this);
	}

	void FormSettingsPanel::updateUI() {
		grid->clear();
		if (!Manager::get()->selectedForm) return;

		std::vector<CCNode*> elements;
		elements.push_back(Manager::get()->selectedForm->quality.createUI(UI::elementSize));
		elements.push_back(CCMenuItemToggler::createWithStandardSprites(this, menu_selector(FormSettingsPanel::onPathVisibilityToggle), 1));
		elements.push_back(CCMenuItemToggler::createWithStandardSprites(this, menu_selector(FormSettingsPanel::onControlPathVisibilityToggle), 1));
		elements.push_back(CCMenuItemToggler::createWithStandardSprites(this, menu_selector(FormSettingsPanel::onHitboxVisibilityToggle), 1));
		grid->addElements(elements);
	}

	void FormSettingsPanel::onPathVisibilityToggle(CCObject* sender) {

	}

	void FormSettingsPanel::onControlPathVisibilityToggle(CCObject* sender) {

	}

	void FormSettingsPanel::onHitboxVisibilityToggle(CCObject* sender) {

	}

	#pragma endregion
}