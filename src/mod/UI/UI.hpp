#pragma once

#include <Geode/Geode.hpp>
#include "lib/Utilities.hpp"
#include <alphalaneous.editortab_api/include/EditorTabAPI.hpp>
#include "lib/Grid.hpp"
#include "mod/UI/SelectionPopup.hpp"

using namespace geode::prelude;

namespace Sculptor {

	class SculptorPanel;
	class LayersPanel;
	class LayerTabsPanel;
	class LayerPropertiesPanel;
	class ModulatorsPanel;
	class ModulatorPropertiesPanel;
	class FormSettingsPanel;

	struct DecoratorButton {
		CCMenuItemSpriteExtra* button;
		Anchor anchor;
		CCPoint offset = { 0, 0 };
		
	};


	class UI : public SculptorNode<UI> {
	public:

		inline static UI* instance = nullptr;
		static UI* get() { return instance; }

		void setup();

		void createTab();
		static CCMenu* createNodeBadges(CCNode* node, std::vector<DecoratorButton>);
		void updateUI();

		void onModeEnter();
		void onModeExit();		

		void setGameUIVisible(bool visible) {
			auto categoriesMenu = EditorUI::get()->getChildByIDRecursive("toolbar-categories-menu");
			if (categoriesMenu) {
				categoriesMenu->setVisible(visible);
			}

			auto togglesMenu = EditorUI::get()->getChildByIDRecursive("toolbar-toggles-menu");
			if (togglesMenu) {
				togglesMenu->setVisible(visible);
			}

			auto spacerLineLeft = EditorUI::get()->getChildByIDRecursive("spacer-line-left");
			if (spacerLineLeft) {
				spacerLineLeft->setVisible(visible);
			}

			auto spacerLineRight = EditorUI::get()->getChildByIDRecursive("spacer-line-right");
			if (spacerLineRight) {
				spacerLineRight->setVisible(visible);
			}
		}
		
		bool inTab() {
			return alpha::editor_tabs::getCurrentTab().unwrapOrDefault() == "sculptor"_spr;
		}

		BoomScrollLayer* tab;
		CCNode* page;
		CCNode* columns;

		SculptorPanel* sculptorPanel;
		LayersPanel* layersPanel;
		LayerTabsPanel* layerTabsPanel;
		LayerPropertiesPanel* layerPropertiesPanel;
		ModulatorsPanel* modulatorsPanel;
		ModulatorPropertiesPanel* modulatorPropertiesPanel;
		FormSettingsPanel* formSettingsPanel;

		static constexpr CCSize pageSize = { 570, 90 };
		static constexpr CCSize poolSize = { 370, 50 };
		static constexpr CCSize elementSize = { 87, 50 };
		static CCPoint midpoint() {
			return CCDirector::sharedDirector()->getWinSize() / 2;
		}

	private:

		ListenerHandle mouseListener;
		ListenerResult handleMouseData(MouseInputData data);

	};	

	class SculptorPanel : public SculptorNode<SculptorPanel> {
	public: 

		void setup();
		void updateUI();

	private:

		NineSlice* base;
		Grid* grid;

		void onModeButton(CCObject* sender);
		void onReturnButton(CCObject* sender);	

		static constexpr CCSize size = { 250, 50 };

	};

	//

	class FormSettingsPanel : public SculptorNode<FormSettingsPanel> {
	public:

		void setup();
		void updateUI();

	private:

		NineSlice* base;
		Grid* grid;

		SelectionPopup* selectPopup;

		void onNewFormButton(CCObject* sender);		
		void onDeleteFormButton(CCObject* sender);
		void onCopyButton(CCObject* sender);
		void onPasteButton(CCObject* sender);	
		void onFlattenButton(CCObject* sender);

		static constexpr CCSize size = { 0.04 * UI::pageSize.width, UI::pageSize.height };

	};

	//

	class LayersPanel : public SculptorNode<LayersPanel> {
	public: 

		void setup();
		void updateUI();

	private:

		NineSlice* base;
		Grid* topGrid;
		Grid* mainGrid;

		SelectionPopup* selectPopup;

		void onNewLayerButton(CCObject* sender);
		void onDeleteLayerButton(CCObject* sender);
		void onSelectLayerButton(CCObject* sender);

		void onCopyLayerButton(CCObject* sender);
		void onPasteLayerButton(CCObject* sender);

		static constexpr CCSize size = { 0.125 * UI::pageSize.width, UI::pageSize.height };
		static constexpr CCSize topSize = { size.width, 0.2 * UI::pageSize.height };
		static constexpr CCSize mainSize = { size.width, 0.8 * UI::pageSize.height };

	};

	//

	class LayerTabsPanel : public SculptorNode<LayerTabsPanel> {
	public:

		void setup();
		void updateUI();

		LayerTab tab = LayerTab::Offset;

	private:

		NineSlice* base;
		Grid* grid;

		void onSelectTabButton(CCObject* sender);		

		static constexpr CCSize size = { 0.03 * UI::pageSize.width, UI::pageSize.height };
		inline static std::vector<std::string> spritePaths = { "tab_style.png"_spr, "tab_offset.png"_spr, "tab_group.png"_spr };

	};

	//

	class LayerPropertiesPanel : public SculptorNode<LayerPropertiesPanel> {
	public:

		void setup();
		void updateUI();

	private:

		NineSlice* base;
		Grid* grid;

		void onNewGroupButton(CCObject* sender);
		void onDeleteGroupButton(CCObject* sender);

		static constexpr CCSize size = { 0.5 * UI::pageSize.width, UI::pageSize.height };

	};

	//


	class ModulatorsPanel : public SculptorNode<ModulatorsPanel> {
	public:

		void setup();
		void updateUI();

	private:

		NineSlice* base;
		Grid* topGrid;
		Grid* mainGrid;

		SelectionPopup* selectPopup;

		void onNewModulatorButton(CCObject* sender);
		void onDeleteModulatorButton(CCObject* sender);
		void onSelectModulatorButton(CCObject* sender);

		void onCopyModulatorButton(CCObject* sender);
		void onPasteModulatorButton(CCObject* sender);

		static constexpr CCSize size = { 0.125 * UI::pageSize.width, UI::pageSize.height };
		static constexpr CCSize topSize = { size.width, 0.2 * UI::pageSize.height };
		static constexpr CCSize mainSize = { size.width, 0.8 * UI::pageSize.height };

	};

	//

	class ModulatorPropertiesPanel : public SculptorNode<ModulatorPropertiesPanel> {
	public:

		void setup();
		void updateUI();

	private:

		NineSlice* base;
		Grid* grid;	

		static constexpr CCSize size = { 0.15 * UI::pageSize.width, UI::pageSize.height };

	};

	//


	
	
}