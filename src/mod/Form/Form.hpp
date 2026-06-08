#pragma once

#include <Geode/Geode.hpp>
#include "lib/Utilities.hpp"
#include "lib/Geometry2.hpp"
#include "mod/Property.hpp"


using namespace geode::prelude;

namespace Sculptor {

	class Layer;
	class Modulator;
	class VectorEditor;

	class Form : public SculptorNode<Form> {
	public:	

		void setup(FormMode mode, CCPoint position);
		void onExit();
		static Form* createDefault(FormMode mode, CCPoint position);

		FormMode mode;

		VectorEditor* vectorEditor;
		std::vector<Layer*> layers;
		std::vector<Modulator*> modulators;

		Property quality{ { .label = "Quality", .isModulatable = false, .defaultValue = 1, .min = 0.1, .max = 3  } };

		bool dirty = true;

		VectorEditor* registerVectorEditor(VectorEditor* editor);
		Layer* registerLayer(Layer* layer);
		Modulator* registerModulator(Modulator* modulator);

		void removeLayer(Layer* layer, bool deleteObjects = true);
		void removeAllLayers(bool deleteObjects = true);
		void removeModulator(Modulator* modulator);
		void removeAllModulators();

		Sequences getPaths(float inflate);
		Polys getDecomposition(float inflate);
		Triangles getTriangulation(float inflate);		

		bool hitboxContains(CCPoint point);

		void onSelect();
		void onDeselect();


		int getModulatorID(Modulator* modulator) {
			return std::ranges::find(modulators, modulator) - modulators.begin();
		}
		int getLayerID(Layer* layer) {
			return std::ranges::find(layers, layer) - layers.begin();
		}

	private:

		void update(float dt);
	};

}