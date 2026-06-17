#include <Geode/Geode.hpp>
#include "mod/Form/Component.hpp"
#include "lib/Utilities.hpp"
#include "lib/Geometry2.hpp"

using namespace geode::prelude;

namespace Sculptor {

	struct ObjectState;
	struct ModulationContext;
	class Form;

	class Layer : public ComponentBase<Layer> {
	public:	

		static std::vector<Layer*> prototypes();

		~Layer() {
			for (auto property : groups) {
				delete property;
			}
		}

		Form* form;	
		std::vector<GameObject*> objects;

		CCRect bounds = { 0, 0, 100, 100 };

		Sequences paths;
		Polys decomposition;
		Triangles triangulation;

		int objectIndex = 0;
		int objectCount = 0;

		virtual void evaluate() = 0;

		std::vector<Property*> getBaseProperties() override {
			return { 
				&x, &rotation, &color,
				&y, &rotationX, &secondaryColor,
				&hue, &rotationY, &zLayer,
				&saturation, &scale, &zOrder,
				&value, &scaleX, &editorLayer,
				&ID, &scaleY
			};
		}			
		
		std::vector<Property*> getProperties() override {
			auto props = ComponentBase<Layer>::getProperties();
			for (const auto& group : groups) {
				props.push_back(group);
			}
			return props;
		}

		std::vector<Property*> getNamedProperties() {
			return ComponentBase<Layer>::getProperties();
		}

		void updateObjects();
		void deleteObjects() {
			for (const auto& obj : objects) {
				EditorUI::get()->deleteObject(obj, false);
			}
			objects.clear();
		}
		void buildObject(ModulationContext context, std::function<ObjectState(const ModulationContext&)> func);

		std::vector<Modulator*> getUsedModulators();
		void copyPropertiesTo(Layer* other);
		Property* addGroupProperty();

		std::vector<Property*> groups = {};

	protected:

		Property ID{ {.label = "ID", .filter = CommonFilter::Int, .leadingDigits = 4 } };
		Property x{ { .label = "X" } };
		Property y{ { .label = "Y" } };
		Property rotation{ {.label = "Rotation" } };
		Property rotationX{ {.label = "Rotation X" } };
		Property rotationY{ {.label = "Rotation Y" } };
		Property scale{ {.label = "Scale", .defaultValue = 1 } };
		Property scaleX{ {.label = "Scale X", .defaultValue = 1 } };
		Property scaleY{ {.label = "Scale Y", .defaultValue = 1 } };
		Property zLayer{ {.label = "Z Layer", .filter = CommonFilter::Int } };
		Property zOrder{ {.label = "Z Order", .filter = CommonFilter::Int } };
		Property editorLayer{ {.label = "Editor Layer", .filter = CommonFilter::Int } };
		Property color{ {.label = "Color", .filter = CommonFilter::Uint } };
		Property secondaryColor{ {.label = "Secondary Color", .filter = CommonFilter::Uint } };
		Property hue{ {.label = "Hue" } };
		Property saturation{ {.label = "Saturation", .leadingDigits = 1, .trailingDigits = 2 } };
		Property value{ {.label = "Value", .leadingDigits = 1, .trailingDigits = 2 } };		

		Property pathOffset{ {.label = "Path Offset", .isModulatable = false } };
		
		Property colorPool{ {.label = "Color Pool", .filter = CommonFilter::Int, .min = 0, .valuePool = {1}, .poolType = PoolType::Color } };			

		bool needsContext;
		void updateBounds();
		void updateNeedsContext();

		ObjectState evaluateAsObjectState(const ModulationContext& context);

	};

	class SolidLayer : public Layer {
	public:

		SolidLayer() {
			label = "Solid";
			spritePath = "layer_solid.png"_spr;					
		};

		SolidLayer* clone() override { return new SolidLayer; }
		void evaluate();
		std::vector<Property*> getTypeProperties() override {
			return { &IDPool, &colorPool, &pathOffset };
		}

		Property IDPool{ {.label = "ID Pool", .filter = CommonFilter::Int, .min = 0, .valuePool = {(float)objectIDs[GDObject::UnitTriangle]}, .poolType = PoolType::ID}};
		
	};

	class OutlineLayer : public Layer {
	public:

		OutlineLayer() {
			label = "Outline";
			spritePath = "layer_outline.png"_spr;			
		};

		OutlineLayer* clone() override { return new OutlineLayer; }
		void evaluate();
		std::vector<Property*> getTypeProperties() override {
			return { &IDPool, &colorPool, &pathOffset, &lineWidth };
		}

		Property lineWidth{ { .label = "Line Width", .defaultValue = 1 } };		

		Property IDPool{ {.label = "ID Pool", .filter = CommonFilter::Int, .min = 0, .valuePool = {(float)objectIDs[GDObject::Line]}, .poolType = PoolType::ID}};
	
	};

	class GlowLayer : public Layer {
	public:

		GlowLayer() {
			label = "Glow";
			spritePath = "layer_glow.png"_spr;
		};

		GlowLayer* clone() override { return new GlowLayer; }
		void evaluate();
		std::vector<Property*> getTypeProperties() override {
			return { &colorPool, &pathOffset, &glowWidth };
		}

		Property glowWidth{ {.label = "Glow Width", .defaultValue = 1 } };
	

	};


	class UniformObjectLayer : public Layer {
	public:

		UniformObjectLayer() {
			label = "Uniform Object";
			spritePath = "layer_uniform.png"_spr;
		};

		UniformObjectLayer* clone() override { return new UniformObjectLayer; }
		void evaluate();
		std::vector<Property*> getTypeProperties() override {
			return { &IDPool, &colorPool, &pathOffset, &spacing };
		}	
		

		Property spacing{ {.label = "Spacing", .defaultValue = gdUnit, .min = 1.f } };

		Property IDPool{ {.label = "ID Pool", .filter = CommonFilter::Int, .min = 0, .valuePool = {1}, .poolType = PoolType::ID} };

	};

	class ChainLayer : public Layer {
	public:

		ChainLayer() {
			label = "Chain";
			spritePath = "layer_chain.png"_spr;
		};

		ChainLayer* clone() override { return new ChainLayer; }
		void evaluate();
		std::vector<Property*> getTypeProperties() override {
			return { &IDPool, &colorPool, &pathOffset };
		}			

		Property IDPool{ {.label = "ID Pool", .filter = CommonFilter::Int, .min = 0, .valuePool = {1}, .poolType = PoolType::ID} };

	};

	class StripLayer : public Layer {
	public:

		StripLayer() {
			label = "Strip";
			spritePath = "layer_strip.png"_spr;
		};

		StripLayer* clone() override { return new StripLayer; }
		void evaluate();
		std::vector<Property*> getTypeProperties() override {
			return { &IDPool, &colorPool, &stripAngle, &stripScale, &pathOffset };
		}
		
		Property stripAngle{ {.label = "Strip Angle", .isModulatable = false} };
		Property stripScale{ {.label = "Strip Scale", .defaultValue = 1, .min = 0} };

		Property IDPool{ {.label = "ID Pool", .filter = CommonFilter::Int, .min = 0, .valuePool = {1}, .poolType = PoolType::ID} };

	};                                                     




}