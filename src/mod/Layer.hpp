#include <Geode/Geode.hpp>
#include "mod/Component.hpp"
#include "lib/Utilities.hpp"
#include "lib/Geometry.hpp"

using namespace geode::prelude;

namespace Sculptor {

	struct ObjectState;
	struct ModulationContext;
	class Form;

	class Layer : public ComponentBase<Layer> {
	public:

		Layer();
		~Layer();

		static std::vector<Layer*> prototypes();

		Form* form;	
		std::vector<GameObject*> objects;

		CCRect bounds = { 0, 0, 100, 100 };

		Sequences paths;
		Polys decomposition;
		Triangles triangulation;

		virtual void evaluate() = 0;

		std::vector<Property*> getBaseProperties() override {
			return { 
				&x, &y, &hue, &saturation, &value, &ID,
				&rotation, &rotationX, &rotationY, &scale, &scaleX, &scaleY, 
				&color,& secondaryColor, &zLayer, &zOrder, &editorLayer
			};
		}			
		
		std::vector<Property*> getProperties() override {
			auto props = ComponentBase<Layer>::getProperties();
			for (const auto& group : groups) {
				props.push_back(group);
			}
			return props;
		}

		void updateObjects();
		void deleteObjects() {
			for (const auto& obj : objects) {
				EditorUI::get()->deleteObject(obj, false);
			}
			objects.clear();
		}
		void buildObject(std::function<ObjectState(const ModulationContext&)> func);		

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
		Property saturation{ {.label = "Saturation", .leadingDigits = 1 } };
		Property value{ {.label = "Value", .leadingDigits = 1 } };

		Property pathOffset{ {.label = "Path Offset", .isModulatable = false } };

		std::vector<Property*> groups;	

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
			return { &pathOffset };
		}

		
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
			return { &pathOffset, &lineWidth };
		}

		/*Property pathOffset{ {.label = "Path Offset", .isModulatable = false } };*/
		Property lineWidth{ { .label = "Line Width", .defaultValue = 1 } };
		
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
			return { &pathOffset, &spacing };
		}

		//Property pathOffset{ {.label = "Path Offset", .isModulatable = false } };
		Property spacing{ {.label = "Spacing", .defaultValue = gdUnit, .min = 1.f } };

	};



}