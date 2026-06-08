#include <Geode/Geode.hpp>
#include <alphalaneous.editortab_api/include/EditorTabAPI.hpp>
#include "mod/UI/UI.hpp"
#include "mod/Manager.hpp"
#include "mod/Form/Modulator.hpp"
#include "mod/Form/Layer.hpp"
#include "mod/Form/VectorEditor.hpp"
#include "mod/Form/Form.hpp"
#include "mod/Serialization.hpp"

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

		formSettingsPanel = FormSettingsPanel::create();
		layersPanel = LayersPanel::create();
		layerTabsPanel = LayerTabsPanel::create();
		layerPropertiesPanel = LayerPropertiesPanel::create();
		modulatorsPanel = ModulatorsPanel::create();
		modulatorPropertiesPanel = ModulatorPropertiesPanel::create();
					

		columns->updateLayout();
	}

	CCMenu* UI::createComponentButton(CCSprite* sprite, bool selected, CCObject* target, SEL_MenuHandler selectCallback, SEL_MenuHandler deleteCallback, SEL_MenuHandler copyCallback, int tag) {
		auto menu = CCMenu::create();

		auto button = CCMenuItemSpriteExtra::create(sprite, target, selectCallback);
		button->setTag(tag);

		menu->setContentSize(button->getContentSize());
		menu->setTouchPriority(0);

		if (selected) {
			auto extraMenu = CCMenu::create();
			extraMenu->setContentSize(button->getContentSize());

			auto deleteSprite = CCSprite::create("delete.png"_spr);
			deleteSprite->setScale(0.3);

			auto copySprite = CCSprite::create("copy.png"_spr);
			copySprite->setScale(0.3);
			
			extraMenu->setTouchPriority(-1);
			extraMenu->setZOrder(1);

			extraMenu->addChildAtPosition(CCMenuItemSpriteExtra::create(deleteSprite, target, deleteCallback), Anchor::TopRight, { -5, -5 });
			extraMenu->addChildAtPosition(CCMenuItemSpriteExtra::create(copySprite, target, copyCallback), Anchor::BottomRight, { -5, 5 });

			menu->addChildAtPosition(extraMenu, Anchor::Center);
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
			formSettingsPanel->updateUI();
			layersPanel->updateUI();
			layerTabsPanel->updateUI();
			layerPropertiesPanel->updateUI();
			modulatorsPanel->updateUI();
			modulatorPropertiesPanel->updateUI();			
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

		std::vector<CCNode*> elements;
		elements.push_back(CCMenuItemSpriteExtra::create(CCSprite::create("create.png"_spr), this, menu_selector(FormSettingsPanel::onNewFormButton)));				

		if (Manager::get()->selectedForm) {
			elements.push_back(CCMenuItemSpriteExtra::create(CCSprite::create("delete.png"_spr), this, menu_selector(FormSettingsPanel::onDeleteFormButton)));
			elements.push_back(CCMenuItemSpriteExtra::create(CCSprite::create("copy.png"_spr), this, menu_selector(FormSettingsPanel::onCopyButton)));
			elements.push_back(CCMenuItemSpriteExtra::create(CCSprite::create("paste.png"_spr), this, menu_selector(FormSettingsPanel::onPasteButton)));
			elements.push_back(CCMenuItemSpriteExtra::create(CCSprite::create("flatten.png"_spr), this, menu_selector(FormSettingsPanel::onFlattenButton)));
		}
		
		grid->addElements(elements);
	}

	void FormSettingsPanel::onNewFormButton(CCObject* sender) {
		selectPopup = SelectionPopup::create(CCDirector::sharedDirector()->getWinSize() / 2, { 100, 50 }, 1, 2, 2,
			[](int i) {
				return (i == 0) ? ButtonSprite::create("Closed") : ButtonSprite::create("Open");
			},
			[](int i) {
				auto form = Form::createDefault(static_cast<FormMode>(i), toEditorSpace(UI::midpoint()));
				Manager::get()->registerForm(form);
				Manager::get()->selectForm(form);
			});
		selectPopup->show();
	}

	void FormSettingsPanel::onDeleteFormButton(CCObject* sender) {
		Manager::get()->removeForm(Manager::get()->selectedForm);
	}

	void FormSettingsPanel::onCopyButton(CCObject* sender) {
		
	}
	void FormSettingsPanel::onPasteButton(CCObject* sender) {
		selectPopup = SelectionPopup::create(CCDirector::sharedDirector()->getWinSize() / 2, { 100, 50 }, 1, 3, 3,
			[](int i) {
				switch (i) {
				case 0:
					return ButtonSprite::create("Paste Style");
				case 1:
					return ButtonSprite::create("Paste Shape");
				case 2:
					return ButtonSprite::create("Paste Both");
				}
			},
			[](int i) {

			});
		selectPopup->show();
	}

	void FormSettingsPanel::onFlattenButton(CCObject* sender) {
		createQuickPopup("Flatten", "This will delete the form and flatten it into normal editor objects. Continue?", "Flatten", "No", [](auto, bool confirm) {
			if (confirm) {
				Manager::get()->removeForm(Manager::get()->selectedForm, false);
			}
		});		
	}




#pragma endregion

	#pragma region LayersPanel

	void LayersPanel::setup() {		
		setContentSize(size);		

		base = createBase(size, {-1, 0});
		addChildAtPosition(base, Anchor::Center);

		auto node = CCNode::create();
		node->setContentSize(size);
		node->setLayout(ColumnLayout::create()->setGap(0)->setAxisReverse(true)->setAxisAlignment(AxisAlignment::End)->setAutoScale(false));

		topGrid = Grid::create(topSize, Direction::Horizontal, 1, 0);
		mainGrid = Grid::create(mainSize, Direction::Vertical, 2, 0);

		node->addChild(topGrid);
		node->addChild(mainGrid);
		node->updateLayout();

		addChild(node);

		UI::get()->columns->addChild(this);
	}

	void LayersPanel::updateUI() {
		topGrid->clear();
		mainGrid->clear();
		if (!Manager::get()->selectedForm) return;

		if (Manager::get()->selectedLayer) {
			{
				std::vector<CCNode*> elements;
				for (const auto& [i, layer] : std::views::enumerate(Manager::get()->selectedForm->layers)) {
					elements.push_back(UI::createComponentButton(
						CCSprite::create(layer->spritePath.c_str()),
						(layer == Manager::get()->selectedLayer),
						this,
						menu_selector(LayersPanel::onSelectLayerButton),
						menu_selector(LayersPanel::onDeleteLayerButton),
						menu_selector(LayersPanel::onCopyLayerButton),
						i
					));
				}
				mainGrid->addElements(elements);
			}
			{
				std::vector<CCNode*> elements;
				elements.push_back(CCLabelBMFont::create("Layers", "chatFont.fnt"));				
				elements.push_back(CCMenuItemSpriteExtra::create(CCSprite::create("create.png"_spr), this, menu_selector(LayersPanel::onNewLayerButton)));		
				auto sprite = CCSprite::create("create_paste.png"_spr);
				if (Manager::get()->clipboard.layer) {
					sprite->setOpacity(50);
					elements.push_back(sprite);
				}
				else {
					elements.push_back(CCMenuItemSpriteExtra::create(sprite, this, menu_selector(LayersPanel::onPasteLayerButton)));
				}
				
				topGrid->addElements(elements);
			}
		}

		
	}

	void LayersPanel::onNewLayerButton(CCObject* sender) {		
		selectPopup = SelectionPopup::create(CCDirector::sharedDirector()->getWinSize() / 2, { 100, 150 }, 3, 6, Layer::prototypes().size(),
			[](int i) {
				return CCSprite::create(Layer::prototypes()[i]->spritePath.c_str());
			}, 
			[](int i) {
				auto layer = static_cast<Layer*>(Layer::prototypes()[i]->clone());
				Manager::get()->selectedForm->registerLayer(layer);
				Manager::get()->selectLayer(layer);
			});
		selectPopup->show();
	}

	void LayersPanel::onDeleteLayerButton(CCObject* sender) {
		auto form = Manager::get()->selectedForm;
		auto layer = Manager::get()->selectedLayer;
		
		form->removeLayer(layer);
		
		if (!form->layers.empty()) Manager::get()->selectLayer(form->layers[0]);
	}

	void LayersPanel::onSelectLayerButton(CCObject* sender) {
		Manager::get()->selectLayer(Manager::get()->selectedForm->layers[sender->getTag()]);
	}

	void LayersPanel::onCopyLayerButton(CCObject* sender) {
		Manager::get()->clipboard.layer = matjson::Serialize<Layer*>::toJson(Manager::get()->selectedLayer);		
	}

	void LayersPanel::onPasteLayerButton(CCObject* sender) {
		Manager::get()->selectedForm->registerLayer(matjson::Serialize<Layer*>::fromJson(Manager::get()->clipboard.layer).unwrap());
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
			auto button = CCMenuItemSpriteExtra::create(CCSprite::create(spritePaths[i].c_str()), this, menu_selector(LayerTabsPanel::onSelectTabButton));
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

		grid = Grid::create(size, Direction::Horizontal, 3, 0);
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
					auto size = property->info.valuePool.empty() ? UI::elementSize : UI::poolSize;
					elements.push_back(property->createUI(size));
				}
				break;
			case LayerTab::Offset:				
				for (const auto& property : layer->getBaseProperties()) {
					elements.push_back(property->createUI(UI::elementSize));
				}
				break;
			case LayerTab::Group:
				for (const auto& property : layer->groups) {
					elements.push_back(property->createUI(UI::elementSize));
				}
				elements.push_back(CCMenuItemSpriteExtra::create(CCSprite::create("create.png"_spr), this, menu_selector(LayerPropertiesPanel::onNewGroupButton)));
				break;
		}

		grid->addElements(elements);
	}

	void LayerPropertiesPanel::onNewGroupButton(CCObject* sender) {
		Manager::get()->selectedLayer->groups.push_back(new Property{ {.label = "Group", .filter = CommonFilter::Int, .min = 0} });
		UI::get()->updateUI();
	}

	void LayerPropertiesPanel::onDeleteGroupButton(CCObject* sender) {

	}

	#pragma endregion

	#pragma region ModulatorsPanel

	void ModulatorsPanel::setup() {
		setContentSize(size);

		base = createBase(size, { -1, 0 });
		addChildAtPosition(base, Anchor::Center);

		auto node = CCNode::create();
		node->setContentSize(size);
		node->setLayout(ColumnLayout::create()->setGap(0)->setAxisReverse(true)->setAxisAlignment(AxisAlignment::End)->setAutoScale(false));

		topGrid = Grid::create(topSize, Direction::Horizontal, 1);
		mainGrid = Grid::create(mainSize, Direction::Vertical, 2);

		node->addChild(topGrid);
		node->addChild(mainGrid);
		node->updateLayout();

		addChild(node);

		UI::get()->columns->addChild(this);
	}

	void ModulatorsPanel::updateUI() {
		topGrid->clear();
		mainGrid->clear();
		if (!Manager::get()->selectedModulator) return;

		{
			std::vector<CCNode*> elements;
			for (const auto& [i, modulator] : std::views::enumerate(Manager::get()->selectedForm->modulators)) {
				elements.push_back(UI::createComponentButton(
					CCSprite::create(modulator->spritePath.c_str()),
					(modulator == Manager::get()->selectedModulator),
					this,
					menu_selector(ModulatorsPanel::onSelectModulatorButton),
					menu_selector(ModulatorsPanel::onDeleteModulatorButton),
					menu_selector(ModulatorsPanel::onCopyModulatorButton),
					i
				));
			}
			mainGrid->addElements(elements);
		}
		{
			std::vector<CCNode*> elements;
			elements.push_back(CCLabelBMFont::create("Mods", "chatFont.fnt"));
			elements.push_back(CCMenuItemSpriteExtra::create(CCSprite::create("create.png"_spr), this, menu_selector(ModulatorsPanel::onNewModulatorButton)));			
			elements.push_back(CCMenuItemSpriteExtra::create(CCSprite::create("create_paste.png"_spr), this, menu_selector(ModulatorsPanel::onPasteModulatorButton)));
			topGrid->addElements(elements);
		}

		
	}

	void ModulatorsPanel::onNewModulatorButton(CCObject* sender) {		
		selectPopup = SelectionPopup::create(CCDirector::sharedDirector()->getWinSize() / 2, { 100, 150 }, 3, 6, Modulator::prototypes().size(), 
			[](int i) {
				return CCSprite::create(Modulator::prototypes()[i]->spritePath.c_str());
			},
			[](int i) {
				auto modulator = static_cast<Modulator*>(Modulator::prototypes()[i]->clone());
				Manager::get()->selectedForm->registerModulator(modulator);
				Manager::get()->selectModulator(modulator);
			});
		selectPopup->show();
	}

	void ModulatorsPanel::onDeleteModulatorButton(CCObject* sender) {
		auto form = Manager::get()->selectedForm;
		auto modulator = Manager::get()->selectedModulator;
		if (form->modulators.size() <= 1) return;

		form->removeModulator(modulator);
	
		if (!form->modulators.empty()) Manager::get()->selectModulator(form->modulators[0]);
	}

	void ModulatorsPanel::onSelectModulatorButton(CCObject* sender) {
		Manager::get()->selectModulator(Manager::get()->selectedForm->modulators[sender->getTag()]);
	}

	void ModulatorsPanel::onCopyModulatorButton(CCObject* sender) {
		Manager::get()->clipboard.modulator = matjson::Serialize<Modulator*>::toJson(Manager::get()->selectedModulator);
	}

	void ModulatorsPanel::onPasteModulatorButton(CCObject* sender) {
		Manager::get()->selectedForm->registerModulator(matjson::Serialize<Modulator*>::fromJson(Manager::get()->clipboard.modulator).unwrap());
		UI::get()->updateUI();
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


}