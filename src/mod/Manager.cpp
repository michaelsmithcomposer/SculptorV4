#include <Geode/Geode.hpp>
#include <alphalaneous.level-storage-api/include/LevelStorageAPI.hpp>
#include <ranges>
#include "mod/Manager.hpp"
#include "mod/Layer.hpp"
#include "mod/Modulator.hpp"
#include "mod/UI.hpp"
#include "mod/Form.hpp"
#include "mod/VectorEditor.hpp"
#include "mod/Serialization.hpp"

using namespace geode::prelude;

namespace Sculptor {

	void Manager::setup() {
		scheduleUpdate();
		instance = this;
		addChild(UI::create());
	}

	void Manager::update(float dt) {
		time += dt;

		if (!reconstructed) {
			reconstructed = true;
			auto data = alpha::level_storage::getSaveContainer(LevelEditorLayer::get(), Mod::get());			
			matjson::Serialize<Manager*>::fromJson(data["manager"]);
			deselect();
		}
	}

	void Manager::onSave() {
		isActive = false;
		auto json = matjson::Serialize<Manager*>::toJson(this);		
		log::info("WRITING: ----------------------------------");
		log::info("{}", json.dump());
		alpha::level_storage::setSavedValue(LevelEditorLayer::get(), "manager", json);	
		
	}

	void Manager::selectForm(Form* form) {
		if (selectedForm) selectedForm->onDeselect();
		selectedForm = form;
		if (!form->layers.empty()) selectedLayer = form->layers.front();
		if (!form->modulators.empty()) selectedModulator = form->modulators.front();
		form->onSelect();
		UI::get()->updateUI();
	}

	void Manager::selectLayer(Layer* layer) { 
		selectedLayer = layer; 
		UI::get()->updateUI();
	}
	void Manager::selectModulator(Modulator* modulator) { 
		selectedModulator = modulator; 
		UI::get()->updateUI();
	}
	void Manager::deselect() {
		if (selectedForm) selectedForm->onDeselect();
		selectedForm = nullptr;
		selectedLayer = nullptr;
		selectedModulator = nullptr;
		UI::get()->updateUI();
	}

	bool Manager::shouldSelectObject(GameObject* object) {
		for (const auto& form : forms) {
			if (std::ranges::contains(form->vectorEditor->proxyObjects, object)) {
				return true;
			}
			for (const auto& layer : form->layers) {
				if (std::ranges::contains(layer->objects, object)) {
					return false;
				}
			}
		}
		return !UI::get()->inTab();
	}

}