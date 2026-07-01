#include <Geode/Geode.hpp>
#include <alphalaneous.editortab_api/include/EditorTabAPI.hpp>
#include "mod/UI/UI.hpp"
#include "mod/Manager.hpp"
#include "mod/Form/Form.hpp"
#include "mod/Form/VectorEditor.hpp"

using namespace geode::prelude;
using namespace Sculptor;

#include <Geode/modify/EditorUI.hpp>
class $modify(SculptorEditorUI, EditorUI) {
	bool init(LevelEditorLayer * layer) {
		if (!EditorUI::init(layer)) return false;

		alpha::editor_tabs::addTab("sculptor"_spr, "sculptor"_spr, 
			[] {
				auto node = CCNode::create();
				node->setUserFlag("alphalaneous.tinker/disable-editor-scroll");
				return node;
			}, 
			[] { 
				return CCNode::create(); 
			});
		
			return true;
	}

	void selectObject(GameObject* object, bool ignoreFilter) {		
		if (!Manager::get()->shouldSelectObject(object) && !ignoreFilter) return;
		EditorUI::selectObject(object, ignoreFilter);
	}

	void selectObjects(CCArray* objects, bool ignoreFilter) {
		auto toSelect = CCArray::create();
		for (const auto& object : CCArrayExt<GameObject*>(objects)) {
			if (Manager::get()->shouldSelectObject(object) || ignoreFilter) {
				toSelect->addObject(object);
			}
		}
		EditorUI::selectObjects(toSelect, ignoreFilter);
	}

    void showUI(bool show) {
		EditorUI::showUI(show);

		auto tabBarRes = alpha::editor_tabs::nodeForTab("sculptor"_spr);
		if (!tabBarRes) return;

		auto tabBar = tabBarRes.unwrap();

		if (tabBar->isVisible()) {
			auto ui = UI::get();
			if (!ui) return;

			ui->setGameUIVisible(false);
		}
	}
};