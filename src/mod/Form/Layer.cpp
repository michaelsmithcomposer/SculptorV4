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
			new GlowLayer,
			new UniformObjectLayer,
			new StripLayer,
			new ChainLayer,			
		};
		return instance;
	}

	std::vector<Modulator*> Layer::getUsedModulators() {
		std::vector<Modulator*> result;
		for (const auto& prop : getProperties()) {
			for (auto [modulator, amount] : prop->getModValues()) {
				if (amount != 0 && !std::ranges::contains(result, modulator)) {
					result.push_back(modulator);
				}
			}
		}
		return result;
	}

	void Layer::copyPropertiesTo(Layer* other) {
		std::unordered_map<Modulator*, Modulator*> map;
		for (const auto& modulator : getUsedModulators()) {
			auto eq = other->form->getEquivalentModulator(modulator);
			if (eq) {
				map[modulator] = *eq;
			}
			else {				
				map[modulator] = Manager::get()->copyModulatorTo(modulator, other->form);
			}
		}

		for (const auto& prop : ComponentBase<Layer>::getProperties()) {
			auto target = *other->getPropertyByLabel(prop->info.label);

			target->info = prop->info;

			target->setBaseValue(prop->getBaseValue());

			for (auto [sourceModulator, amount] : prop->getModValues()) {
				if (amount != 0) {
					target->setModValue(map.at(sourceModulator), amount);
				}				
			}
		}

		for (const auto& prop : groups) {
			auto target = other->addGroupProperty();	

			target->setBaseValue(prop->getBaseValue());

			for (auto [sourceModulator, amount] : prop->getModValues()) {
				if (amount != 0) {
					target->setModValue(map.at(sourceModulator), amount);
				}
			}
		}

		
	}

	Property* Layer::addGroupProperty() {
		auto property = new Property{ {.label = "Group", .filter = CommonFilter::Int, .min = 0} };
		groups.push_back(property);
		return property;
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

		LevelEditorLayer::get()->calculateColorGroups();
		LevelEditorLayer::get()->generateVisibilityGroups();
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
					object.ID = objectIDs[GDObject::Line];
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
					object.ID = objectIDs[GDObject::UnitCircle];
					object.color = (int)colorPool.evaluate(context);
					return object;
				});
			}
		}
	}

	void GlowLayer::evaluate() {
		for (const auto& sequence : paths) {
			for (const auto& edge : sequence.edges()) {		

				auto dummyObject = ObjectState{					
					.x = edge.a().x,
					.y = edge.a().y,					
				};

				float scale = glowWidth.evaluate({ &dummyObject });

				buildObject({ .edge = &edge, .sequence = &sequence }, [&](auto context) {
					return ObjectState{
						.ID = objectIDs[GDObject::QuarterGlow],
						.x = edge.a().x,
						.y = edge.a().y,
						.scale = glowWidth.evaluate({}),
					};
				});

				Line middle = edge.shrunk(scale * (gdUnit / 4.0));
				CCPoint normal = sequence.normalAt(middle.lerp(0.5));

				if (edge.length() > scale * (gdUnit / 2.0)) {
					buildObject({ .edge = &edge, .sequence = &sequence }, [&](auto context) {
						float width = glowWidth.evaluate(context);
						CCPoint position = middle.lerp(0.5) + normal * (width * (gdUnit / 6.0));
						return ObjectState{
							.ID = objectIDs[GDObject::QuarterGlowLine],
							.x = position.x,
							.y = position.y,
							.rotation = CC_RADIANS_TO_DEGREES(normal.getAngle() - (PI / 2)),
							.scaleX = middle.length() / (gdUnit / 2),
							.scaleY = width
						};
					});
					buildObject({ .edge = &edge, .sequence = &sequence }, [&](auto context) {
						float width = glowWidth.evaluate(context);
						CCPoint position = middle.b() + normal * (width * (gdUnit / 6.0)) + fromPolar(width * (gdUnit / 6.0), normal.getAngle() + (PI / 2));
						return ObjectState{
							.ID = objectIDs[GDObject::QuarterGlowCorner],
							.x = position.x,
							.y = position.y,
							.rotation = CC_RADIANS_TO_DEGREES(normal.getAngle() - (PI / 2)),
							.scale = width						
						};
					});
					buildObject({ .edge = &edge, .sequence = &sequence }, [&](auto context) {
						float width = glowWidth.evaluate(context);
						CCPoint position = middle.a() + normal * (width * (gdUnit / 6.0)) + fromPolar(width * (gdUnit / 6.0), normal.getAngle() - (PI / 2));
						return ObjectState{
							.ID = objectIDs[GDObject::QuarterGlowCorner],
							.x = position.x,
							.y = position.y,
							.rotation = CC_RADIANS_TO_DEGREES(normal.getAngle() + PI),
							.scale = width
						};
					});					
				}
				else if (edge.length() > scale * (gdUnit / 4.0)) {
					buildObject({ .edge = &edge, .sequence = &sequence }, [&](auto context) {
						float width = glowWidth.evaluate(context);						
						return ObjectState{
							.ID = objectIDs[GDObject::QuarterGlow],
							.x = edge.lerp(0.5).x,
							.y = edge.lerp(0.5).y,							
							.scale = width
						};
					});				
				}
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

				Line line = { end, start };				

				buildObject({ .sequence = &sequence }, [&](auto context) {					
					Line localLine = Line(a, b);

					CCPoint position = line.lerp(0.5);
					float rotation = line.angle() - localLine.angle();
					float scale = line.length() / width;
					CCPoint mid = localLine.lerp(0.5);
					CCPoint scaledMid = { mid.x * scale, mid.y };
					CCPoint rotatedMid = scaledMid.rotate(CCPoint::forAngle(rotation)) * scale;							

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

	void StripLayer::evaluate() {
		form->debug->clear();
		for (const auto& poly : decomposition) {

			auto box = poly.boundingBox();
			auto circle = Circle::fromBoundingBox(box);
			float angle = stripAngle.evaluate({});

			auto axis = Line(circle.origin + fromPolar(circle.radius, angle), circle.origin - fromPolar(circle.radius, angle));			

			float t = 0;

			while (t < 1) {

				auto point = axis.lerp(t);

				ObjectState dummyObject = { .x = point.x, .y = point.y };
				ModulationContext dummyContext = {
					.objectState = &dummyObject,
					.layer = this,					
				};
				int ID = (int)IDPool.evaluate(dummyContext);	

				auto sprite = objectSpriteFromID(ID);

				float width = sprite->getContentWidth() * stripScale.evaluate(dummyContext);
				float stripT = width / axis.length();
				float endT = t + stripT;

				auto midpoint = axis.lerp(lerp(t, endT, 0.5));				

				auto stripAxisOffset = fromPolar(circle.radius, angle + HALF_PI);
				auto axisOffset = fromPolar(width / 2, angle);	
				auto stripAxis = Line(midpoint + stripAxisOffset, midpoint - stripAxisOffset);
				auto stripNormalized = (stripAxis.a() - stripAxis.b()).normalize();				

				PathsD l = OpenIntersection({ Sequence{ { stripAxis.a() + axisOffset, stripAxis.b() + axisOffset } }.path() }, { poly.path() });
				PathsD r = OpenIntersection({ Sequence{ { stripAxis.a() - axisOffset, stripAxis.b() - axisOffset } }.path() }, { poly.path() });

				PathsD lRects;
				for (const auto path : l) {
					lRects.push_back({ 
						path.back(),
						path.front(),
						{ path.front().x - axisOffset.x * 2, path.front().y - axisOffset.y * 2},
						{ path.back().x - axisOffset.x * 2, path.back().y - axisOffset.y * 2},
						
					});
				}

				PathsD rRects;
				for (const auto path : r) {
					rRects.push_back({						
						{ path.front().x + axisOffset.x * 2, path.front().y + axisOffset.y * 2},
						{ path.back().x + axisOffset.x * 2, path.back().y + axisOffset.y * 2},
						path.back(),
						path.front(),						
					});
				}

				auto rectIntersect = Intersect(lRects , rRects, FillRule::NonZero);
				auto paths = Intersect(rectIntersect, { poly.path() }, FillRule::NonZero);
				
				for (const auto& path : paths) {	

					Poly pathPoly = { Sequence::fromPath(path).points() };
					
					CCPoint bestPos;
					CCPoint bestNeg;

					float bestPosDistance = FLT_MAX;
					float bestNegDistance = FLT_MAX;

					for (const auto& pd : path) {

						CCPoint point = ccp(pd.x, pd.y);
						CCPoint projection = stripAxis.projectionOf(point);		
						
						auto normal = pathPoly.normalAt(point).dot(stripNormalized);

						if (isClose(normal, 0)) continue;						

						if (normal > 0) {							
							float d = projection.getDistance(stripAxis.b());
							if (d < bestPosDistance) {
								bestPosDistance = d;
								bestPos = projection;
							}
						}
						else {							
							float d = projection.getDistance(stripAxis.a());
							if (d < bestNegDistance) {
								bestNegDistance = d;
								bestNeg = projection;
							}
						}
					}
					

					if (bestPosDistance < FLT_MAX && bestNegDistance < FLT_MAX && !isClose(bestPos, bestNeg)) {
						Sequence sequence = { { bestPos + axisOffset, bestPos - axisOffset, bestNeg - axisOffset, bestNeg + axisOffset } };

						float area = abs(Area(sequence.path()));
						auto i = Intersect({ sequence.path() }, { poly.path() }, FillRule::NonZero);
						if (i.size() != 1) continue;		
						float iarea = abs(Area(i.front()));					

						if (isClose((area / iarea), 1, 1)) {
							CCPoint position = bestPos.lerp(bestNeg, 0.5);
							buildObject({}, [&](auto context) {
								return ObjectState{
									.ID = ID,
									.x = position.x,
									.y = position.y,
									.rotation = CC_RADIANS_TO_DEGREES(angle),
									.scaleX = width / sprite->getContentWidth(),
									.scaleY = bestPos.getDistance(bestNeg) / sprite->getContentHeight(),
									.color = (int)colorPool.evaluate(context)
								};
							});
						}				
					}
					
			
				}		
				
				t += stripT;

			}

		}
	}
	

}