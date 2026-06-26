#include <Geode/Geode.hpp>
#include "mod/Chain.hpp"
#include "mod/Serialization.hpp"

using namespace geode::prelude;

namespace Sculptor {

	float getChainEndT(const Sequence& sequence, float t, float length) {			

		int steps = std::min(sequence.length(), 500.f);	
		
		float searchSizeT = (1 - t);
		float stepSizeT = searchSizeT / steps;		

		CCPoint start = sequence.lerp(t);

		for (int i = 1; i < steps; i++) {
			float stepT = t + i * stepSizeT;
			CCPoint point = sequence.lerp(stepT);
			if (start.getDistance(point) >= length) {
				return stepT;
			}
		}
		return 1;
	}

	void writeObjectConnectionsJson(std::unordered_map<int, std::vector<CCPoint>> map) {
		auto path = Mod::get()->getSaveDir() / "objectConnections.json";
		std::vector<matjson::Value> result;
		for (auto [ID, connections] : map) {
			result.push_back(matjson::makeObject({ { "ID", ID }, { "connections", connections } }));
		}
		std::ofstream(path) << matjson::makeObject({ { "objectConnections", result } }).dump();
	}

	geode::Result<std::unordered_map<int, std::vector<CCPoint>>> readObjectConnectionsJson() {
		auto path = Mod::get()->getSaveDir() / "objectConnections.json";
		std::ifstream file(path);	
		auto value = matjson::parse(file).unwrap();

		std::unordered_map<int, std::vector<CCPoint>> result;
		for (const auto& object : value["objectConnections"]) {
			GEODE_UNWRAP_INTO(int ID, object["ID"].as<int>());
			GEODE_UNWRAP_INTO(std::vector<CCPoint> connections, object["connections"].as<std::vector<CCPoint>>());
			result[ID] = connections;
		}
		return geode::Ok(result);
	}	
}