#pragma once

#include <Geode/Geode.hpp>
#include "lib/Geometry2.hpp"
#include "lib/Utilities.hpp"

using namespace geode::prelude;

namespace Sculptor {

	float getChainEndT(const Sequence& sequence, float t, float length);

	void writeObjectConnectionsJson(std::unordered_map<int, std::vector<CCPoint>> map);

	geode::Result<std::unordered_map<int, std::vector<CCPoint>>> readObjectConnectionsJson();

}