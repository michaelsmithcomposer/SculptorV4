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

	/*void ConnectionTagger::setup() {
		CCSize size = { 250, 250 };
		setContentSize(size);
		setPosition(165, 65);
		addChildAtPosition(createBase(size), Anchor::Center);

		drawNode = CCDrawNode::create();
		drawNode->setContentSize(size);
		drawNode->setBlendFunc({ GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR });
		drawNode->setZOrder(10);
		addChildAtPosition(drawNode, Anchor::Center);

		addChildAtPosition(label, Anchor::Bottom, { 0, -10 });

		mouseListener = MouseInputEvent().listen([this](MouseInputData data) { return this->handleMouseData(data); });
		keyboardListener = KeyboardInputEvent().listen([this](KeyboardInputData data) { this->handleKeyboardData(data); });

		map = readObjectConnectionsJson().unwrap();

		updateUI();
	}

	void ConnectionTagger::updateUI() {
		drawNode->clear();
		if (sprite) {
			sprite->removeFromParent();
		}
		sprite = CCSprite::createWithSpriteFrameName(ObjectToolbox::sharedState()->intKeyToFrame(ID));
		sprite->setScale(scale);
		addChildAtPosition(sprite, Anchor::Center);
	}

	ListenerResult ConnectionTagger::handleMouseData(MouseInputData data) {
		if (data.action == MouseInputData::Action::Press) {
			if (data.button == MouseInputData::Button::Left) {
				auto point = drawNode->convertToNodeSpace(getMousePos());

				drawNode->drawCircle(point, 2.5, { 1, 1, 1, 1 }, 0.01, { 1, 1, 1, 1 }, 5);
				map[ID].push_back(point / scale);
			}
			else if (data.button == MouseInputData::Button::Right) {
				map[ID].clear();
				drawNode->clear();
			}			
		}
		return ListenerResult::Propagate;
	}

	ListenerResult ConnectionTagger::handleKeyboardData(KeyboardInputData data) {
		if (data.action == KeyboardInputData::Action::Press) {
			if (data.key == enumKeyCodes::KEY_Z) {
				ID++;
				map[ID].clear();
				label->setString(fmt::format("{}", ID).c_str());
				writeObjectConnectionsJson(map);
				log::info("{}", readObjectConnectionsJson());
				updateUI();
			}
			else if (data.key == enumKeyCodes::KEY_X) {
				std::vector<CCPoint> points = { CCPoint{-14, -14}, CCPoint{-14, 14}, CCPoint{14, 14}, CCPoint{14, -14} };
				for (auto point : points) {
					map[ID].push_back(point);
					drawNode->drawCircle(point * scale, 2.5, { 1, 1, 1, 1 }, 0.01, { 1, 1, 1, 1 }, 5);
				}				
			}
			else if (data.key == enumKeyCodes::KEY_C) {
				std::vector<CCPoint> points = { CCPoint{-14, 0}, CCPoint{14, 0}};
				for (auto point : points) {
					map[ID].push_back(point);
					drawNode->drawCircle(point * scale, 2.5, { 1, 1, 1, 1 }, 0.01, { 1, 1, 1, 1 }, 5);
				}
			}
		}
		return ListenerResult::Propagate;
	}*/
}