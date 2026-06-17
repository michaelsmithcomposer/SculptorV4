#pragma once
#include <Geode/Geode.hpp>
#include "lib/Utilities.hpp"

using namespace geode::prelude;

namespace Sculptor {

	class Form;
	class Layer;
	class Modulator;

	class Manager : public SculptorNode<Manager> {
	public:

		inline static Manager* instance = nullptr;
		static Manager* get() { return instance; }		
		bool reconstructed = false;
		bool isActive = true;

		void setup();
		void reconstruct();
		void onSave();

		std::vector<Form*> forms;

		Form* selectedForm;
		Layer* selectedLayer;
		Modulator* selectedModulator;

		Form* clipboard;
		ClipboardState clipboardState;

		void selectForm(Form* form);
		void selectLayer(Layer* layer);
		void selectModulator(Modulator* modulator);
		void deselect();	

		Layer* copyLayerTo(Layer*, Form* target);
		Modulator* copyModulatorTo(Modulator* modulator, Form* target);


		Form* registerForm(Form* form) {
			forms.push_back(form);
			return form;
		}
		void removeForm(Form* form, bool deleteObjects = true);
		int getFormID(Form* form) const {
			return std::ranges::find(forms, form) - forms.begin();
		}

		bool shouldSelectObject(GameObject* object);

		CCNode* getBatchLayer() {
			return LevelEditorLayer::get()->getChildByID("main-node")->getChildByID("batch-layer");
		}

		float time = 0.f;

		bool showPaths = true;
		bool showControlPaths = true;
		bool showHitboxes = true;

	private:

		void update(float dt);

	};

}