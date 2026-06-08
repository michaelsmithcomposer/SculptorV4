#pragma once
#include <Geode/Geode.hpp>
#include "lib/Utilities.hpp"

using namespace geode::prelude;

namespace Sculptor {

	class Modulator;
	class ModulationContext;
	

	class Property {
	public:
	
		struct Info {
			std::string label = "";
			bool isModulatable = true;			
			float defaultValue = 0;
			CommonFilter filter = CommonFilter::Float;
			int leadingDigits = 3;
			int trailingDigits = -1;
			float min = NAN;
			float max = NAN;

			std::vector<float> valuePool;
			PoolType poolType = PoolType::None;
		};

		Property(Info info, std::optional<float> value = std::nullopt, std::optional<std::unordered_map<Modulator*, float>> modValues = std::nullopt) : 
			info(resolveInfo(info)), 
			baseValue(value.value_or(info.defaultValue)), 
			modValues(modValues.value_or({})) {};

		void setFromJson(const matjson::Value& value);

		float getBaseValue() const { return baseValue; };
		void setBaseValue(float value) {
			baseValue = value;
			if (onChanged) onChanged(this);
		};

		std::unordered_map<Modulator*, float> getModValues() const { return modValues; };
		float getModValue(Modulator* modulator) { return modValues[modulator]; };
		void setModValue(Modulator* modulator, float value) {
			modValues[modulator] = value;
			if (onChanged) onChanged(this);
		};
		void removeModulator(Modulator* modulator) {
			modValues.erase(modulator);
		}

		void setCallback(std::function<void(Property*)> onChanged) { this->onChanged = onChanged; };

		float evaluate(const ModulationContext& context);

		CCNode* createUI(CCSize size);

		Info info;


	private:

		float baseValue;
		std::unordered_map<Modulator*, float> modValues;
		std::function<void(Property*)> onChanged;

		Info resolveInfo(Info& info);


	};

}