
#include <Geode/Geode.hpp>
#include "mod/Property.hpp"
#include "mod/Form/Modulator.hpp"
#include "mod/UI/PropertyInput.hpp"
#include "mod/UI/ModulatablePropertyInput.hpp"
#include "mod/UI/PoolInput.hpp"
#include "mod/Serialization.hpp"

using namespace geode::prelude;

namespace Sculptor {

	void Property::setFromJson(const matjson::Value& value) {
		auto result = matjson::Serialize<Property*>::fromJson(value).unwrap();
		info = result->info;
		baseValue = result->baseValue;
		modValues = result->modValues;
		delete result;
	}

	float Property::evaluate(const ModulationContext& context) {
		if (!info.isModulatable) return baseValue;

		float result = baseValue;
		for (const auto& [modulator, amount] : modValues) {
			if (amount != 0) {
				result += amount * modulator->evaluate(context);
			}			
		}
		
		if (info.valuePool.empty()) {
			return clamp(result, info.min, info.max);
		}
		else {
			int i = (int)std::round(result) % info.valuePool.size();			
			return info.valuePool[i];
		}
	}

	Property::Info Property::resolveInfo(Property::Info& info) {
		if (info.trailingDigits < 0) info.trailingDigits = (info.filter == CommonFilter::Float) ? 1 : 0;
		float magnitude = pow(10.f, info.leadingDigits) - pow(10.f, -info.trailingDigits);
		if (std::isnan(info.min)) info.min = -magnitude;
		if (std::isnan(info.max)) info.max = magnitude;
		return info;
	}

	CCNode* Property::createUI(CCSize size) {
		if (!info.valuePool.empty()) {
			return PoolInput::create(this, size);
		}
		if (info.isModulatable) {
			return ModulatablePropertyInput::create(this, size);
		}
		else {
			return PropertyInput::create(this, size);			
		}
	}
}
