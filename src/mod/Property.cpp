
#include <Geode/Geode.hpp>
#include "mod/Property.hpp"
#include "mod/Modulator.hpp"
#include "mod/PropertyInput.hpp"
#include "mod/ModulatablePropertyInput.hpp"
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
			result += amount * modulator->evaluate(context);
		}
		return clamp(result, info.min, info.max);
	}

	Property::Info Property::resolveInfo(Property::Info& info) {
		if (info.trailingDigits < 0) info.trailingDigits = (info.filter == CommonFilter::Float) ? 1 : 0;
		float magnitude = pow(10.f, info.leadingDigits) - pow(10.f, -info.trailingDigits);
		if (std::isnan(info.min)) info.min = -magnitude;
		if (std::isnan(info.max)) info.max = magnitude;
		return info;
	}

	CCNode* Property::createUI(CCSize size) {
		if (info.isModulatable) {
			return ModulatablePropertyInput::create(this, size);
		}
		else {
			return PropertyInput::create(this, size);			
		}
	}
}
