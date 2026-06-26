#include <Geode/Geode.hpp>
#include "mod/Form/Component.hpp"
#include "lib/Utilities.hpp"
#include "lib/Geometry2.hpp"
#include "mod/Property.hpp"

using namespace geode::prelude;

namespace Sculptor {

	struct ObjectState;
	struct ModulationContext;
	class Form;

	class Layer : public ComponentBase<Layer> {
	public:	

		static std::vector<Ref<Layer>> prototypes();

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
				x, rotation, color,
				y, rotationX, secondaryColor,
				hue, rotationY, zLayer,
				saturation, scale, zOrder,
				value, scaleX, editorLayer,
				ID, scaleY
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

		std::vector<Ref<Property>> groups = {};

	protected:

		Ref<Property> ID = Property::create( {.label = "ID", .filter = CommonFilter::Int, .leadingDigits = 4 } );
		Ref<Property> x = Property::create( { .label = "X" } );
		Ref<Property> y = Property::create( { .label = "Y" } );
		Ref<Property> rotation = Property::create( {.label = "Rotation" } );
		Ref<Property> rotationX = Property::create( {.label = "Rotation X" } );
		Ref<Property> rotationY = Property::create( {.label = "Rotation Y" } );
		Ref<Property> scale = Property::create( {.label = "Scale", .defaultValue = 1 } );
		Ref<Property> scaleX = Property::create( {.label = "Scale X", .defaultValue = 1 } );
		Ref<Property> scaleY = Property::create( {.label = "Scale Y", .defaultValue = 1 } );
		Ref<Property> zLayer = Property::create( {.label = "Z Layer", .filter = CommonFilter::Int } );
		Ref<Property> zOrder = Property::create( {.label = "Z Order", .filter = CommonFilter::Int } );
		Ref<Property> editorLayer = Property::create( {.label = "Editor Layer", .filter = CommonFilter::Int } );
		Ref<Property> color = Property::create( {.label = "Color", .filter = CommonFilter::Uint } );
		Ref<Property> secondaryColor = Property::create( {.label = "Secondary Color", .filter = CommonFilter::Uint } );
		Ref<Property> hue = Property::create( {.label = "Hue" } );
		Ref<Property> saturation = Property::create( {.label = "Saturation", .leadingDigits = 1, .trailingDigits = 2 } );
		Ref<Property> value = Property::create( {.label = "Value", .leadingDigits = 1, .trailingDigits = 2 } );		

		Ref<Property> pathOffset = Property::create( {.label = "Path Offset", .isModulatable = false } );
		
		Ref<Property> colorPool = Property::create( {.label = "Color Pool", .filter = CommonFilter::Int, .min = 0, .valuePool = {1}, .poolType = PoolType::Color } );			

		bool needsContext;
		void updateBounds();
		void updateNeedsContext();

		ObjectState evaluateAsObjectState(const ModulationContext& context);

	};

	class SolidLayer : public Layer {
	public:

		static SolidLayer* create() {
			auto ret = new SolidLayer();
			ret->autorelease();
			return ret;
		}

		SolidLayer() {
			label = "Solid";
			spritePath = "layer_solid.png"_spr;					
		};

		SolidLayer* clone() override { return SolidLayer::create(); }
		void evaluate() override;
		std::vector<Property*> getTypeProperties() override {
			return { IDPool, colorPool, pathOffset };
		}

		Ref<Property> IDPool = Property::create( {.label = "ID Pool", .filter = CommonFilter::Int, .min = 0, .valuePool = {(float)objectIDs[GDObject::UnitTriangle]}, .poolType = PoolType::ID});
		
	};

	class OutlineLayer : public Layer {
	public:

		static OutlineLayer* create() {
			auto ret = new OutlineLayer();
			ret->autorelease();
			return ret;
		}

		OutlineLayer() {
			label = "Outline";
			spritePath = "layer_outline.png"_spr;			
		};

		OutlineLayer* clone() override { return OutlineLayer::create(); }
		void evaluate() override;
		std::vector<Property*> getTypeProperties() override {
			return { colorPool, pathOffset, lineWidth };
		}

		Ref<Property> lineWidth = Property::create( { .label = "Line Width", .defaultValue = 1 } );		

		//Property IDPool{ {.label = "ID Pool", .filter = CommonFilter::Int, .min = 0, .valuePool = {(float)objectIDs[GDObject::Line]}, .poolType = PoolType::ID}};
	
	};

	class GlowLayer : public Layer {
	public:

		static GlowLayer* create() {
			auto ret = new GlowLayer();
			ret->autorelease();
			return ret;
		}

		GlowLayer() {
			label = "Glow";
			spritePath = "layer_glow.png"_spr;
		};

		GlowLayer* clone() override { return GlowLayer::create(); }
		void evaluate() override;
		std::vector<Property*> getTypeProperties() override {
			return { colorPool, pathOffset, glowWidth };
		}

		Ref<Property> glowWidth = Property::create( {.label = "Glow Width", .defaultValue = 1 } );
	

	};


	class UniformObjectLayer : public Layer {
	public:

		static UniformObjectLayer* create() {
			auto ret = new UniformObjectLayer();
			ret->autorelease();
			return ret;
		}

		UniformObjectLayer() {
			label = "Uniform Object";
			spritePath = "layer_uniform.png"_spr;
		};

		UniformObjectLayer* clone() override { return UniformObjectLayer::create(); }
		void evaluate() override;
		std::vector<Property*> getTypeProperties() override {
			return { IDPool, colorPool, pathOffset, spacing };
		}	
		

		Ref<Property> spacing = Property::create( {.label = "Spacing", .defaultValue = gdUnit, .min = 1.f } );

		Ref<Property> IDPool = Property::create( {.label = "ID Pool", .filter = CommonFilter::Int, .min = 0, .valuePool = {1}, .poolType = PoolType::ID} );

	};

	class ChainLayer : public Layer {
	public:

		static ChainLayer* create() {
			auto ret = new ChainLayer();
			ret->autorelease();
			return ret;
		}

		ChainLayer() {
			label = "Chain";
			spritePath = "layer_chain.png"_spr;
		};

		ChainLayer* clone() override { return ChainLayer::create(); }
		void evaluate() override;
		std::vector<Property*> getTypeProperties() override {
			return { IDPool, colorPool, pathOffset };
		}			

		Ref<Property> IDPool = Property::create( {.label = "ID Pool", .filter = CommonFilter::Int, .min = 0, .valuePool = {1}, .poolType = PoolType::ID} );

	};

	class StripLayer : public Layer {
	public:

		static StripLayer* create() {
			auto ret = new StripLayer();
			ret->autorelease();
			return ret;
		}

		StripLayer() {
			label = "Strip";
			spritePath = "layer_strip.png"_spr;
		};

		StripLayer* clone() override { return StripLayer::create(); }
		void evaluate() override;
		std::vector<Property*> getTypeProperties() override {
			return { IDPool, colorPool, stripAngle, stripScale, pathOffset };
		}
		
		Ref<Property> stripAngle = Property::create( {.label = "Strip Angle", .isModulatable = false} );
		Ref<Property> stripScale = Property::create( {.label = "Strip Scale", .defaultValue = 1, .min = 0.05} );

		Ref<Property> IDPool = Property::create( {.label = "ID Pool", .filter = CommonFilter::Int, .min = 0, .valuePool = {1}, .poolType = PoolType::ID} );

	};                                                     




}