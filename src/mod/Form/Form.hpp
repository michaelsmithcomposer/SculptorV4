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
		std::vector<Ref<Layer>> layers;
		std::vector<Ref<Modulator>> modulators;

		Property quality{ { .label = "Quality", .isModulatable = false, .defaultValue = 1, .min = 0.1, .max = 3  } };

		bool dirty = true;

		VectorEditor* registerVectorEditor(VectorEditor* editor);
		Layer* registerLayer(Layer* layer);
		Modulator* registerModulator(Modulator* modulator);

		void removeLayer(Layer* layer, bool deleteObjects = true);
		void removeAllLayers(bool deleteObjects = true);
		void removeModulator(Modulator* modulator);
		void removeAllModulators();

		std::optional<Modulator*> getEquivalentModulator(Modulator* modulator);
		void copyStyleTo(Form* other);


		Sequences getPaths(float inflate);
		Polys getDecomposition(float inflate);
		Triangles getTriangulation(float inflate);		

		bool hitboxContains(CCPoint point);

		void onSelect();
		void onDeselect();

		int getModulatorID(Modulator* modulator);
		int getLayerID(Layer* layer);

		CCDrawNode* debug;

	private:

		void update(float dt);
	};

}