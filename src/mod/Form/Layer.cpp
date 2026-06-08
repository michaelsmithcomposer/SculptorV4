#include <Geode/Geode.hpp>
#include "mod/Form/Layer.hpp"
#include "mod/Form/Modulator.hpp"
#include "mod/Form/Form.hpp"
#include "lib/ObjectState.hpp"
#include "mod/Manager.hpp"
#include "mod/Chain.hpp"

using namespace geode::prelude;

namespace Sculptor {

	std::vector<Layer*> Layer::prototypes() {
		static std::vector<Layer*> instance = {	
			new SolidLayer,
			new OutlineLayer,
			new UniformObjectLayer,
			new ChainLayer
		};
		return instance;
	}



	void Layer::updateObjects() {	
		objectIndex = 0;
		objectCount = objects.size();
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

	void Layer::buildObject(ModulationContext context, std::function<ObjectState(const ModulationContext&)> func) {
		ObjectState styleObject;
		if (needsContext) {
			auto base = func({});					
			styleObject = func({ &base, this, context.edge, context.sequence });
		}
		else {
			styleObject = func({});
		}
		ObjectState propsObject = evaluateAsObjectState({ &styleObject, this });
		objects.push_back(styleObject.compose(propsObject).createObject());
		objectIndex++;
	}

	ObjectState Layer::evaluateAsObjectState(const ModulationContext& context) {

		std::vector<int> groupIDs;
		for (const auto& property : groups) {
			int ID = round(property->evaluate(context));
			if (ID > 0) {
				groupIDs.push_back(ID);
			}
		}

		log::info("Layer::evaluateAsObjectState , groups = {}", groupIDs);

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
			.absoluteSaturation = true,
			.groups = groupIDs
		};
	}


	//

	void SolidLayer::evaluate() {			
		for (const auto& triangle : triangulation) {
			buildObject({}, [&](auto context) { 
				auto object = ObjectState::fromTriangle(triangle); 
				object.ID = (int)IDPool.evaluate(context);
				object.color = (int)colorPool.evaluate(context);
				return object;
			});
		}
	}

	void OutlineLayer::evaluate() {		
		for (const auto& sequence : paths) {
			for (const auto& edge : sequence.edges()) {
				buildObject({ .edge = &edge, .sequence = &sequence }, [&](auto context) {									
					auto object = ObjectState::fromLine(edge, lineWidth.evaluate(context));
					object.ID = (int)IDPool.evaluate(context);
					object.color = (int)colorPool.evaluate(context);
					return object;
				});
			}
		}		
		for (const auto& sequence : paths) {
			for (const auto& point : sequence.points()) {
				buildObject({ .sequence = &sequence }, [&](auto context) {				
					float r = lineWidth.evaluate(context) * 0.5;
					auto object = ObjectState::fromCircle(Circle({ point, r}));
					object.ID = (int)IDPool.evaluate(context);
					object.color = (int)colorPool.evaluate(context);
					return object;
				});
			}
		}
	}

	void UniformObjectLayer::evaluate() {	
		
		for (const auto& sequence : paths) {	

			float l = 0;
			float length = sequence.length();
			while (l < length) {
				float t = l / length;
				CCPoint position = roundTo(sequence.lerp(t), 1);

				float s = 0.0f;
				buildObject({ .sequence = &sequence }, [&](auto context) {					
					s = spacing.evaluate(context);
					return ObjectState{.ID = (int)IDPool.evaluate(context), .x = position.x, .y = position.y, .color = (int)colorPool.evaluate(context) };
				});

				l += s;				
			}

		}
	}


	void ChainLayer::evaluate() {		

		for (const auto& sequence : paths) {
			
			if (isClose(sequence.length(), 0)) continue;
			
			float t = 0;

			while (t < 1) {						

				auto point = sequence.lerp(t);
				ObjectState dummyObject = { .x = point.x, .y = point.y };
				ModulationContext dummyContext = {
					.objectState = &dummyObject,
					.layer = this,
					.sequence = &sequence,
				};
				int ID = (int)IDPool.evaluate(dummyContext);

				auto [a, b] = getSpriteConnections(ID);
				float width = a.getDistance(b);
				float endT = getChainEndT(sequence, t, width);				

				CCPoint start = sequence.lerp(t);
				CCPoint end = sequence.lerp(endT);

				Line line = { start , end };				

				buildObject({ .sequence = &sequence }, [&](auto context) {					
					Line localLine = Line(a, b);

					CCPoint position = line.lerp(0.5);
					float rotation = line.angle() - localLine.angle();
					float scale = line.length() / width;
					CCPoint mid = localLine.lerp(0.5);
					CCPoint scaledMid = { mid.x * scale, mid.y };
					CCPoint rotatedMid = scaledMid.rotate(CCPoint::forAngle(rotation)) * scale;					

					log::info("x = {}, y = {}, r = {}, s = {}", position.x - rotatedMid.x, position.y - rotatedMid.y, CC_RADIANS_TO_DEGREES(rotation),  scale);

					return ObjectState{
						.ID = ID,
						.x = position.x - rotatedMid.x,
						.y = position.y - rotatedMid.y,
						.rotation = CC_RADIANS_TO_DEGREES(rotation),
						.scaleX = scale,
						.scaleY = scale,
						.color = (int)colorPool.evaluate(context)
					};
				});

				t = endT;				

			}

		}

	}
	

}