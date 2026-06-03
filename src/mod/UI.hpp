#pragma once

#include <Geode/Geode.hpp>
#include "lib/Utilities.hpp"
#include <alphalaneous.editortab_api/include/EditorTabAPI.hpp>
#include "lib/Grid.hpp"
#include "mod/SelectionPopup.hpp"

using namespace geode::prelude;

namespace Sculptor {

	class SculptorPanel;
	class LayersPanel;
	class LayerTabsPanel;
	class LayerPropertiesPanel;
	class ModulatorsPanel;
	class ModulatorPropertiesPanel;
	class FormSettingsPanel;


	class UI : public SculptorNode<UI> {
	public:

		inline static UI* instance = nullptr;
		static UI* get() { return instance; }

		void setup();

		void createTab();
		static CCMenu* createComponentButton(CCSprite* sprite, bool selected, CCObject* target, SEL_MenuHandler selectCallback, SEL_MenuHandler deleteCallback, int tag);
		void updateUI();

		void onModeEnter();
		void onModeExit();		

		void setGameUIVisible(bool visible) {
			EditorUI::get()->getChildByIDRecursive("toolbar-categories-menu")->setVisible(visible);
			EditorUI::get()->getChildByIDRecursive("toolbar-toggles-menu")->setVisible(visible);
			EditorUI::get()->getChildByIDRecursive("spacer-line-left")->setVisible(visible);
			EditorUI::get()->getChildByIDRecursive("spacer-line-right")->setVisible(visible);
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
		void onNewClosedFormButton(CCObject* sender);
		void onNewOpenFormButton(CCObject* sender);
		void onDeleteFormButton(CCObject* sender);

		static constexpr CCSize size = { 250, 50 };

	};

	//

	class LayersPanel : public SculptorNode<LayersPanel> {
	public: 

		void setup();
		void updateUI();

	private:

		NineSlice* base;
		Grid* grid;

		SelectionPopup* selectPopup;

		void onNewLayerButton(CCObject* sender);
		void onDeleteLayerButton(CCObject* sender);
		void onSelectLayerButton(CCObject* sender);

		static constexpr CCSize size = { 0.1 * UI::pageSize.width, UI::pageSize.height };

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

		static constexpr CCSize size = { 0.57 * UI::pageSize.width, UI::pageSize.height };

	};

	//


	class ModulatorsPanel : public SculptorNode<ModulatorsPanel> {
	public:

		void setup();
		void updateUI();

	private:

		NineSlice* base;
		Grid* grid;

		SelectionPopup* selectPopup;

		void onNewModulatorButton(CCObject* sender);
		void onDeleteModulatorButton(CCObject* sender);
		void onSelectModulatorButton(CCObject* sender);

		static constexpr CCSize size = { 0.1 * UI::pageSize.width, UI::pageSize.height };

	};

	//

	class ModulatorPropertiesPanel : public SculptorNode<ModulatorPropertiesPanel> {
	public:

		void setup();
		void updateUI();

	private:

		NineSlice* base;
		Grid* grid;	

		static constexpr CCSize size = { 0.2 * UI::pageSize.width, UI::pageSize.height };

	};

	//

	class FormSettingsPanel : public SculptorNode<FormSettingsPanel> {
	public:

		void setup();
		void updateUI();

	private:

		NineSlice* base;
		Grid* grid;

		void onPathVisibilityToggle(CCObject* sender);
		void onControlPathVisibilityToggle(CCObject* sender);
		void onHitboxVisibilityToggle(CCObject* sender);

		static constexpr CCSize size = { 0.1 * UI::pageSize.width, UI::pageSize.height };

	};
	
	
}