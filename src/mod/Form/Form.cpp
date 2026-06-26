#include <Geode/Geode.hpp>
#include "mod/Form/Form.hpp"
#include "mod/Manager.hpp"
#include "mod/Form/Layer.hpp"
#include "mod/Form/Modulator.hpp"
#include "mod/Form/VectorEditor.hpp"
#include "mod/UI/UI.hpp"

using namespace geode::prelude;

namespace Sculptor {

	void Form::setup(FormMode mode, CCPoint position) {
		Manager::get()->formLayer->addChild(this);
		debug = CCDrawNode::create();
		setZOrder(1000);
		addChild(debug);
		scheduleUpdate();
		setPosition(position);

		this->mode = mode;		
	}

	void Form::onExit() {	
		for (auto& modulator : modulators) {
			delete modulator;
		}
		for (auto& layer : layers) {
			delete layer;
		}		
		CCNode::onExit();
	}

	void Form::update(float dt) {	

		if (dirty && Manager::get()->isActive && Manager::get()->selectedForm && Manager::get()->selectedForm == this) {
			dirty = false;
			for (auto layer : layers) {
				layer->updateObjects();
			}
		}
	}

	Form* Form::createDefault(FormMode mode, CCPoint position) {
		auto form = Form::create(mode, position);	
		
		form->registerVectorEditor(VectorEditor::createDefault({0, 0}));
	
		auto a = new TreeNode(ccp(0, 60), form->vectorEditor, form->vectorEditor->root);
		auto b = new TreeNode(ccp(60, 60), form->vectorEditor, a);
		auto c = new TreeNode(ccp(60, 0), form->vectorEditor, b);
		if (form->mode == FormMode::Closed) {
			form->vectorEditor->root->setNewParent(c);
		}				

		auto layer = new SolidLayer;
		form->registerLayer(layer);		

		auto modulator = new NoiseModulator;
		form->registerModulator(modulator);

		Manager::get()->selectForm(form);

		return form;
	}

	VectorEditor* Form::registerVectorEditor(VectorEditor* editor) {		
		this->vectorEditor = editor;
		editor->form = this;
		addChild(editor);		
		return editor;
	}

	Layer* Form::registerLayer(Layer* layer) {
		layer->form = this;
		this->layers.push_back(layer);
		layer->updateObjects();
		for (auto& property : layer->getProperties()) {
			property->setCallback([layer](Property* p) {
				layer->form->dirty = true;
			});
		}
		return layer;
	}

	Modulator* Form::registerModulator(Modulator* modulator) {
		modulator->form = this;	
		this->modulators.push_back(modulator);
		for (auto& property : modulator->getProperties()) {
			property->setCallback([this](Property* p) {
				dirty = true;
			});
		}
		return modulator;
	}

	void Form::removeLayer(Layer* layer, bool deleteObjects) {
		if (deleteObjects) layer->deleteObjects();
		std::erase(layers, layer);	
		if (Manager::get()->selectedLayer == layer) Manager::get()->selectedLayer = nullptr;
		delete layer;
		UI::get()->updateUI();		
	}

	void Form::removeAllLayers(bool deleteObjects) {
		for (auto& layer : layers) {
			if (deleteObjects) layer->deleteObjects();			
			if (Manager::get()->selectedLayer == layer) Manager::get()->selectedLayer = nullptr;
			delete layer;
		}		
		layers.clear();
		UI::get()->updateUI();
	}

	void Form::removeModulator(Modulator* modulator) {
		for (auto& layer : layers) {
			for (auto& property : layer->getProperties()) {
				property->removeModulator(modulator);
			}
		}
		std::erase(modulators, modulator);
		if (Manager::get()->selectedModulator == modulator) Manager::get()->selectedModulator = nullptr;
		delete modulator;
		UI::get()->updateUI();
	}	

	void Form::removeAllModulators() {
		for (auto& modulator : modulators) {
			for (auto& layer : layers) {
				for (auto& property : layer->getProperties()) {
					property->removeModulator(modulator);
				}
			}			
			if (Manager::get()->selectedModulator == modulator) Manager::get()->selectedModulator = nullptr;
			delete modulator;
		}
		modulators.clear();
		UI::get()->updateUI();
	}

	std::optional<Modulator*> Form::getEquivalentModulator(Modulator* modulator) {
		for (auto& m : modulators) {
			if (m->isEquivalent(modulator)) return m;
		}
		return std::nullopt;
	}

	void Form::copyStyleTo(Form* other) {
		other->removeAllModulators();
		other->removeAllLayers();

		for (const auto& modulator : modulators) {
			Manager::get()->copyModulatorTo(modulator, other);
		}
		for (const auto& layer : layers) {
			Manager::get()->copyLayerTo(layer, other);
		}
	}

	Sequences Form::getPaths(float inflate) {
		if (inflate == 0 && mode == FormMode::Open) return vectorEditor->getPaths();

		Sequences result;
		auto decomposition = vectorEditor->getDecomposition(inflate);
		for (const auto& poly : decomposition) {
			result.push_back(poly.asSequence());
		}
		return result;
	}

	Polys Form::getDecomposition(float inflate) {
		return vectorEditor->getDecomposition(inflate);
	}

	Triangles Form::getTriangulation(float inflate) {
		return vectorEditor->getTriangulation(inflate);
	}

	bool Form::hitboxContains(CCPoint point) {		
		int hits = 0;
		for (const auto& poly : getDecomposition(5.5)) {
			if (poly.contains(point)) {
				hits++;
			}
		}
		return hits % 2 == 1;
	}


	void Form::onSelect() {
		vectorEditor->showControlPath = true;
		vectorEditor->showPath = true;
		vectorEditor->showHitbox = false;
		vectorEditor->setAllVisible(true);
		vectorEditor->scheduleUpdate();		
	}

	void Form::onDeselect() {
		vectorEditor->mouseNode->setActive(false);
		vectorEditor->showControlPath = false;
		vectorEditor->showPath = false;
		vectorEditor->showHitbox = true;
		vectorEditor->setAllVisible(false);
		vectorEditor->unscheduleUpdate();
		vectorEditor->redraw();
	}


}