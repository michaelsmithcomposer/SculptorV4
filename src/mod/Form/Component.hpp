#pragma once
#include <Geode/Geode.hpp>
#include "mod/Property.hpp"

using namespace geode::prelude;

namespace Sculptor {

	class Form;

	struct Component {	

		virtual ~Component() = default;

		virtual Component* clone() = 0;		

		virtual std::vector<Property*> getProperties() = 0;	
		virtual std::vector<Property*> getBaseProperties() = 0;
		virtual std::vector<Property*> getTypeProperties() { return {}; };

		Form* form;

	};

	template <typename Derived>
	class ComponentBase : public Component {
	public:		

		std::vector<Property*> getProperties() override{
			auto base = getBaseProperties();
			auto type = getTypeProperties();
			base.append_range(type);
			return base;
		}

		std::optional<Property*> getPropertyByLabel(std::string label) {
			for (const auto& property : getProperties()) {
				if (property->info.label == label) {
					return property;
				}
			}
			return std::nullopt;
		}

		std::string label;
		std::string spritePath;

	};

}