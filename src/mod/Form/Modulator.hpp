#include <Geode/Geode.hpp>
#include "mod/Form/Component.hpp"
#include "lib/Utilities.hpp"
#include "lib/Geometry2.hpp"

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

		static std::vector<Modulator*> prototypes();	

		~Modulator();

		Form* form;

		virtual float sample(const ModulationContext& context) = 0;

		std::vector<Property*> getBaseProperties() override {
			return { &depth, &offset, &ramp };
		}

		bool isEquivalent(Modulator* other);
		void copyPropertiesTo(Modulator* other);

		float evaluate(const ModulationContext& context) {
			float value = sample(context);

			float r = ramp.evaluate({});
			float exp = r > 0 ? 1 + r : 1 / (1 - r);

			value = pow(abs(value), exp) * sign(value);
			value *= depth.evaluate({});
			value += offset.evaluate({});

			return value;
		}

		Property depth{ {.label = "depth", .isModulatable = false, .defaultValue = 1} };
		Property offset{ {.label = "offset", .isModulatable = false} };
		Property ramp{ {.label = "ramp", .isModulatable = false} };

	
		bool contextual = true;
		ccColor3B color;

	};

	class NoiseModulator : public Modulator {
	public:

		NoiseModulator() {
			label = "Noise";
			spritePath = "mod_noise.png"_spr;			
			contextual = false;
			color = { 100, 100, 255 };
		}

		NoiseModulator* clone() override { return new NoiseModulator; };

		float sample(const ModulationContext& context) override;

	};

	class XModulator : public Modulator {
	public:

		XModulator() {
			label = "X";
			spritePath = "mod_x.png"_spr;			
			color = { 255, 100, 100 };
		}

		XModulator* clone() override { return new XModulator; };

		float sample(const ModulationContext& context) override;

	};

	class YModulator : public Modulator {
	public:

		YModulator() {
			label = "Y";
			spritePath = "mod_y.png"_spr;			
			color = { 100, 255, 100 };
		}

		YModulator* clone() override { return new YModulator; };

		float sample(const ModulationContext& context) override;

	};

	class SineModulator : public Modulator {
	public:

		SineModulator() {
			label = "Sine";
			spritePath = "mod_sine.png"_spr;			
			color = { 255, 255, 100 };
		}

		SineModulator* clone() override { return new SineModulator; };

		float sample(const ModulationContext& context) override;

		std::vector<Property*> getTypeProperties() override {
			return { &frequency, &phase, &angle };
		}

		Property frequency{ {.label = "frequency", .isModulatable = false, .defaultValue = 1, .min = 0} };
		Property phase{ {.label = "phase", .isModulatable = false} };
		Property angle{ {.label = "angle", .isModulatable = false} };

	};

	class NormalModulator : public Modulator {
	public:

		NormalModulator() {
			label = "Normal";
			spritePath = "mod_normal.png"_spr;			
			color = { 255, 100, 255 };
		}

		NormalModulator* clone() override { return new NormalModulator; };

		float sample(const ModulationContext& context) override;			

	};

	class IndexModulator : public Modulator {
	public:

		IndexModulator() {
			label = "Index";
			spritePath = "mod_index.png"_spr;			
			color = { 0, 216, 255 };
		}

		IndexModulator* clone() override { return new IndexModulator; };

		float sample(const ModulationContext& context) override;

	};
	

}