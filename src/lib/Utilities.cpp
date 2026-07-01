#include <Geode/Geode.hpp>
#include "lib/Utilities.hpp"
#include "mod/Manager.hpp"
#include "external/validObjectIDs.hpp"
#include "external/spriteConnections.h"

using namespace geode::prelude;

namespace Sculptor {

	CCSprite* objectSpriteFromID(int ID) {		
		return CCSprite::createWithSpriteFrameName(ObjectToolbox::sharedState()->intKeyToFrame(ID));
	}

	CCSprite* objectButtonSpriteFromID(int ID) {
		auto objectSprite = CCSprite::createWithSpriteFrameName(ObjectToolbox::sharedState()->intKeyToFrame(ID));
		return BasedButtonSprite::create(objectSprite, BaseType::Editor, (int)EditorBaseSize::Normal, (int)EditorBaseColor::Gray);
	}

	CCSprite* colorSpriteFromID(int ID) {
		auto action = LevelEditorLayer::get()->m_levelSettings->m_effectManager->getColorAction(ID);
		auto sprite = ColorChannelSprite::create();
		sprite->updateValues(action);
		return sprite;
	}

	void exportSprites() {	

		std::string outDir = "C:/Users/pauld/OneDrive/Desktop/sprites/";
		std::filesystem::create_directories(outDir);

		for (int id : validObjectIDs) {		

			auto sprite = objectSpriteFromID(id);
			auto size = sprite->getContentSize();
			auto rt = CCRenderTexture::create(size.width, size.height);
			rt->beginWithClear(1, 1, 1, 1);
			sprite->setColor(ccBLACK);
			sprite->setPosition(size / 2);			
			sprite->visit();
			rt->end();

			auto image = rt->newCCImage(true);

			std::string filePath = outDir + std::to_string(id) + ".png";
			image->saveToFile(filePath.c_str());

			log::info("saved {} to {}", id, filePath);
			
		}
	}

	std::pair<CCPoint, CCPoint> getSpriteConnections(int ID) {
		auto sprite = objectSpriteFromID(ID);	
		auto [a, b] = spriteConnections.at(ID);

		CCPoint localA = { a.x - sprite->getContentWidth() / 2, -a.y + sprite->getContentHeight() / 2 };
		CCPoint localB = { b.x - sprite->getContentWidth() / 2, -b.y + sprite->getContentHeight() / 2 };

		sprite->release();
		return std::make_pair(localA, localB);
	}

	float sinTime(float frequency, float phase, float amplitude) {
		return sinf(((Manager::get()->time * frequency) - phase) * 2.0f * M_PI) * amplitude;
	}

	float sign(float a) {
		return a >= 0 ? 1 : -1;
	}

	float lerp(float a, float b, float mix) {
		return a + (b - a) * mix;
	}

	float inverseLerp(float a, float b, float t) {
		return (t - a) / (b - a);
	}

	float clamp(float value, float a, float b) {
		float min = std::min(a, b);
		float max = std::max(a, b);
		return std::clamp(value, min, max);
	}

	float wrap(float value, float min, float max) {
		float range = max - min;
		return min + std::fmod(std::fmod(value - min, range) + range, range);
	}

	bool isClose(float a, float b, float precision) {
		return std::fabs(a - b) < precision;
	}

	bool isClose(CCPoint a, CCPoint b, float precision) {
		return isClose(a.x, b.x, precision) && isClose(a.y, b.y, precision);
	}

	bool rangeContains(float start, float end, float value) {
		float min = std::min(start, end);
		float max = std::max(start, end);
		return ((value > min) || isClose(value, min)) && ((value < max) || (isClose(value, max)));
	}

	int safeModulo(int a, int b) {
		return (b + (a % b)) % b;
	}

	int validateID(int ID) {
		return std::ranges::contains(validObjectIDs, ID) ? ID : 1;
	}

	CCPoint toEditorSpace(CCPoint point) {
		return Manager::get()->getBatchLayer()->convertToNodeSpace(point);
	}

	CCPoint fromEditorSpace(CCPoint point) {
		return Manager::get()->getBatchLayer()->convertToWorldSpace(point);
	}

	float roundTo(float value, int places) {
		double factor = std::pow(10.0, places);
		return std::round(value * factor) / factor;
	}

	CCPoint roundTo(CCPoint value, int places) {
		double factor = std::pow(10.0, places);
		return ccp(roundTo(value.x, places), roundTo(value.y, places));
	}

	float noiseXY(CCPoint point) {
	
		int x = static_cast<int>(point.x * 10);
		int y = static_cast<int>(point.y * 10);
		
		int h = x * 374761393 + y * 668265263;
		h = (h ^ (h >> 13)) * 1274126177;
		h = h ^ (h >> 16);
	
		return static_cast<float>(h & 0x7FFFFFFF) / static_cast<float>(0x7FFFFFFF) * 2.0f - 1.0f;
		
	}

	float noiseIndex(int i) {		
		int h = i * 374761393 + i * 668265263;
		h = (h ^ (h >> 13)) * 1274126177;
		h = h ^ (h >> 16);

		return static_cast<float>(h & 0x7FFFFFFF) / static_cast<float>(0x7FFFFFFF) * 2.0f - 1.0f;
	}

	bool ctrl() {
		return CCKeyboardDispatcher::get()->m_bControlPressed;
	}

	bool alt() {
		return CCKeyboardDispatcher::get()->m_bAltPressed;
	}

	bool shift() {
		return CCKeyboardDispatcher::get()->m_bShiftPressed;
	}

	NineSlice* createBase(CCSize size, ccColor3B color, CCSize padding) {
		auto base = NineSlice::create("square02b_001.png", { 0, 0, 80, 80 });
		base->setScale(0.5);
		base->setColor(color);
		base->setOpacity(45);
		base->setContentSize({ (size.width + padding.width) * 2, (size.height + padding.height) * 2 });
		return base;
	}
}