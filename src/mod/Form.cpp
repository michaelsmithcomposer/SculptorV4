#include <Geode/Geode.hpp>
#include "mod/Form.hpp"
#include "mod/Manager.hpp"
#include "mod/Layer.hpp"
#include "mod/Modulator.hpp"
#include "mod/VectorEditor.hpp"

using namespace geode::prelude;

namespace Sculptor {

	void Form::setup(FormMode mode, CCPoint position) {
		Manager::get()->getBatchLayer()->addChild(this);
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
		return layer;
	}

	Modulator* Form::registerModulator(Modulator* modulator) {
		modulator->form = this;
		this->modulators.push_back(modulator);
		return modulator;
	}

	void Form::removeLayer(Layer* layer) {
		layer->deleteObjects();
		std::erase(layers, layer);	
		if (Manager::get()->selectedLayer == layer) Manager::get()->selectedLayer = nullptr;
		delete layer;
	}

	void Form::removeModulator(Modulator* modulator) {
		for (auto& layer : layers) {
			for (auto& property : layer->getProperties()) {
				property->getModValues().erase(modulator);
			}
		}
		std::erase(modulators, modulator);
		if (Manager::get()->selectedModulator == modulator) Manager::get()->selectedModulator = nullptr;
		delete modulator;
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