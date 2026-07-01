#include <Geode/Geode.hpp>
#include "mod/Form/Component.hpp"
#include "lib/Utilities.hpp"
#include "lib/Geometry2.hpp"
#include "mod/Property.hpp"

using namespace geode::prelude;

namespace Sculptor {

	struct ObjectState;
	class Layer;

	struct ModulationContext {
		ObjectState* objectState = nullptr;
		Layer* layer = nullptr;
		const Line* edge = nullptr;
		const Sequence* sequence = nullptr;		

		Line nearestEdge() const;
	};	

	class Modulator : public ComponentBase<Modulator> {
	public:

		static std::vector<Ref<Modulator>> prototypes();	

		~Modulator();

		Form* form;

		virtual float sample(const ModulationContext& context) = 0;

		std::vector<Property*> getBaseProperties() override {
			//return { depth, offset, ramp, rangeMin, rangeMax };
			return { depth, offset, ramp };
		}

		bool isEquivalent(Modulator* other);
		void copyPropertiesTo(Modulator* other);

		float evaluate(const ModulationContext& context);

		Ref<Property> depth = Property::create( {.label = "depth", .isModulatable = false, .defaultValue = 1} );
		Ref<Property> offset = Property::create( {.label = "offset", .isModulatable = false} );
		Ref<Property> ramp = Property::create( {.label = "ramp", .isModulatable = false} );		
		//Ref<Property> rangeMin = Property::create( {.label = "min", .isModulatable = false, .defaultValue = -1} );
		//Ref<Property> rangeMax = Property::create( {.label = "max", .isModulatable = false, .defaultValue = 1} );
	
		bool contextual = true;
		ccColor3B color;

	};

	class NoiseModulator : public Modulator {
	public:

		static NoiseModulator* create() {
			auto ret = new NoiseModulator();
			ret->autorelease();
			return ret;
		}

		NoiseModulator() {
			label = "Noise";
			spritePath = "mod_noise.png"_spr;			
			contextual = false;
			color = { 100, 100, 255 };
		}

		NoiseModulator* clone() override { return NoiseModulator::create(); };

		float sample(const ModulationContext& context) override;

	};

	class XModulator : public Modulator {
	public:

		static XModulator* create() {
			auto ret = new XModulator();
			ret->autorelease();
			return ret;
		}

		XModulator() {
			label = "X";
			spritePath = "mod_x.png"_spr;			
			color = { 255, 100, 100 };
		}

		XModulator* clone() override { return XModulator::create(); };

		float sample(const ModulationContext& context) override;

	};

	class YModulator : public Modulator {
	public:

		static YModulator* create() {
			auto ret = new YModulator();
			ret->autorelease();
			return ret;
		}

		YModulator() {
			label = "Y";
			spritePath = "mod_y.png"_spr;			
			color = { 100, 255, 100 };
		}

		YModulator* clone() override { return YModulator::create(); };

		float sample(const ModulationContext& context) override;

	};

	class SineModulator : public Modulator {
	public:

		static SineModulator* create() {
			auto ret = new SineModulator();
			ret->autorelease();
			return ret;
		}

		SineModulator() {
			label = "Sine";
			spritePath = "mod_sine.png"_spr;			
			color = { 255, 255, 100 };
		}

		SineModulator* clone() override { return SineModulator::create(); };

		float sample(const ModulationContext& context) override;

		std::vector<Property*> getTypeProperties() override {
			return { frequency, phase, angle };
		}

		Ref<Property> frequency = Property::create( {.label = "frequency", .isModulatable = false, .defaultValue = 1, .min = 0} );
		Ref<Property> phase = Property::create( {.label = "phase", .isModulatable = false} );
		Ref<Property> angle = Property::create( {.label = "angle", .isModulatable = false} );

	};

	class NormalModulator : public Modulator {
	public:

		static NormalModulator* create() {
			auto ret = new NormalModulator();
			ret->autorelease();
			return ret;
		}

		NormalModulator() {
			label = "Normal";
			spritePath = "mod_normal.png"_spr;			
			color = { 255, 100, 255 };
		}

		NormalModulator* clone() override { return NormalModulator::create(); };

		float sample(const ModulationContext& context) override;			

	};

	class IndexModulator : public Modulator {
	public:

		static IndexModulator* create() {
			auto ret = new IndexModulator();
			ret->autorelease();
			return ret;
		}

		IndexModulator() {
			label = "Index";
			spritePath = "mod_index.png"_spr;			
			color = { 0, 216, 255 };
		}

		IndexModulator* clone() override { return IndexModulator::create(); };

		float sample(const ModulationContext& context) override;

	};
	

}