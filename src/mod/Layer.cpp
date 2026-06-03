#include <Geode/Geode.hpp>
#include "mod/Layer.hpp"
#include "mod/Modulator.hpp"
#include "mod/Form.hpp"
#include "lib/ObjectState.hpp"

using namespace geode::prelude;

namespace Sculptor {

	std::vector<Layer*> Layer::prototypes() {
		static std::vector<Layer*> instance = {	
			new SolidLayer,
			new OutlineLayer,
			new UniformObjectLayer
		};
		return instance;
	}

	Layer::Layer() {
		for (auto& property : getProperties()) {
			property->setCallback([this](Property* p) {
				this->form->dirty = true;
			});
		}
	}

	Layer::~Layer() {
				
	}

	void Layer::updateObjects() {
		paths = form->getPaths(pathOffset.evaluate({}));
		decomposition = form->getDecomposition(pathOffset.evaluate({}));
		triangulation = form->getTriangulation(pathOffset.evaluate({}));
		updateNeedsContext();
		updateBounds();
		deleteObjects();
		evaluate();
	}

	void Layer::updateBounds() {
		Sequence sequence;
		for (const auto& object : objects) {
			sequence.push_back(form->convertToNodeSpace(fromEditorSpace(object->getPosition())));
		}
		if (sequence.size() > 1) {
			bounds = sequence.boundingBox();
		}
		
	}

	void Layer::updateNeedsContext() {
		for (const auto& prop : getProperties()) {
			if (!prop->info.isModulatable) continue;
			for (const auto& [modulator, value] : prop->getModValues()) {
				if (value != 0 && modulator->contextual) {
					needsContext = true;
					return;
				}
			}
		}
		needsContext = false;
	}

	void Layer::buildObject(std::function<ObjectState(const ModulationContext&)> func) {
		ObjectState styleObject;
		if (needsContext) {
			auto base = func({});
			ModulationContext context = { &base, this };
			styleObject = func(context);
		}
		else {
			styleObject = func({});
		}
		ObjectState propsObject = evaluateAsObjectState({ &styleObject, this });
		objects.push_back(styleObject.compose(propsObject).createObject());
	}

	ObjectState Layer::evaluateAsObjectState(const ModulationContext& context) {
		return ObjectState{
			.ID = static_cast<int>(ID.evaluate(context)),
			.x = x.evaluate(context) + form->getPositionX(),
			.y = y.evaluate(context) + form->getPositionY(),
			.rotation = rotation.evaluate(context),
			.rotationX = rotationX.evaluate(context),
			.rotationY = rotationY.evaluate(context),
			.scale = scale.evaluate(context),
			.scaleX = scaleX.evaluate(context),
			.scaleY = scaleY.evaluate(context),
			.zLayer = static_cast<int>(zLayer.evaluate(context)),
			.zOrder = static_cast<int>(zOrder.evaluate(context)),
			.editorLayer = static_cast<int>(editorLayer.evaluate(context)),
			.color = static_cast<int>(color.evaluate(context)),
			.secondaryColor = static_cast<int>(secondaryColor.evaluate(context)),
			.hue = hue.evaluate(context),
			.saturation = saturation.evaluate(context),
			.value = value.evaluate(context),
			.absoluteBrightness = true,
			.absoluteSaturation = true
		};
	}


	//

	void SolidLayer::evaluate() {			
		for (const auto& triangle : triangulation) {
			buildObject([&](auto) { return ObjectState::fromTriangle(triangle); });
		}
	}

	void OutlineLayer::evaluate() {		
		for (const auto& sequence : paths) {
			for (const auto& edge : sequence.edges()) {
				buildObject([&](auto context) {
					float width = lineWidth.evaluate(context);					
					auto obj = ObjectState::fromLine(edge, width);					
					return obj;
				});
			}
		}
		for (const auto& sequence : paths) {
			for (const auto& point : sequence.getPoints()) {
				buildObject([&](auto context) {
					float radius = lineWidth.evaluate(context) * 0.5;
					auto obj = ObjectState::fromCircle(Circle({ point, radius }));					
					return obj;
				});
			}
		}
	}

	void UniformObjectLayer::evaluate() {	

		//auto paths = form->getPaths(pathOffset.evaluate({}));
		for (const auto& sequence : paths) {		


			float l = 0;
			int i = 0;
			float length = sequence.length();
			while (l < length) {
				float t = l / length;
				CCPoint position = roundTo(sequence.lerp(t), 1);

				float s = 0.0f;
				buildObject([&](auto context) {
					context.index = i;
					s = spacing.evaluate(context);
					return ObjectState{ .x = position.x, .y = position.y };
				});

				l += s;
				i++;
			}

		}
	}
	

}