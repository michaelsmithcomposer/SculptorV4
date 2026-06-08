#include <Geode/Geode.hpp>
#include "mod/Form/Modulator.hpp"
#include "lib/ObjectState.hpp"
#include "mod/Manager.hpp"
#include "mod/Form/Form.hpp"
#include "mod/Form/Layer.hpp"
#include "lib/Geometry2.hpp"

using namespace geode::prelude;

namespace Sculptor {

	Line ModulationContext::nearestEdge() const {
		if (edge) {
			return *edge;
		}
		else if (sequence) {
			CCPoint point = { objectState->x, objectState->y };
			return nearestLineAndProjection(point, sequence->edges()).first;
		}
		else {
			CCPoint point = { objectState->x, objectState->y };
			return nearestLineAndProjection(point, sequencesToLines(layer->paths)).first;
		}
	}

	std::vector<Modulator*> Modulator::prototypes() {
		static std::vector<Modulator*> instance = {
			new NoiseModulator,
			new XModulator,
			new YModulator,
			new SineModulator,
			new NormalModulator,
			new IndexModulator,
		};
		return instance;
	}

	Modulator::~Modulator() {
	
	}

	float NoiseModulator::sample(const ModulationContext& context) {
		if (!context.objectState || !context.layer) return CCRANDOM_MINUS1_1();
		if (context.layer) return noiseIndex(context.layer->objectIndex);				
	}

	float XModulator::sample(const ModulationContext& context) {
		if (!context.objectState || !context.layer) return 0;
		float x = context.layer->bounds.origin.x;
		return clamp(inverseLerp(x, x + context.layer->bounds.size.width, context.objectState->x), -1, 1);
	}

	float YModulator::sample(const ModulationContext& context) {		
		if (!context.objectState || !context.layer) return 0;
		float y = context.layer->bounds.origin.y;
		return clamp(inverseLerp(y, y + context.layer->bounds.size.height, context.objectState->y), -1, 1);
	}

	float SineModulator::sample(const ModulationContext& context) {
		if (!context.objectState) return 0;
		
		float dx = cos(angle.evaluate({}));
		float dy = sin(angle.evaluate({}));

		float length = abs(context.layer->bounds.size.width * dx) + abs(context.layer->bounds.size.height * dy);

		float projection = (context.objectState->x - context.layer->bounds.origin.x) * dx + 
						   (context.objectState->y - context.layer->bounds.origin.y) * dy;

		return sin((projection / length) * (frequency.evaluate({}) * TAU) + phase.evaluate({}));
	}

	float NormalModulator::sample(const ModulationContext& context) {
		if (!context.objectState || !context.layer) return 0;
		CCPoint point = { context.objectState->x, context.objectState->y };
		CCPoint normal = context.nearestEdge().normal();
		return atan2(normal.y, normal.x) / PI;	
	}

	float IndexModulator::sample(const ModulationContext& context) {
		if (!context.objectState || !context.layer) return 0;
		return context.layer->objectIndex;
	}
}