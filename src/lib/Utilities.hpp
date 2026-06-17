#pragma once

#include <Geode/Geode.hpp>
#include "external/clipper2/clipper.h"

using namespace geode::prelude;

namespace Sculptor {

	static constexpr float PI = M_PI;
	static constexpr float TAU = M_PI * 2;
	static constexpr float HALF_PI = M_PI / 2;

	template <typename Derived, typename Base = CCNode>
	class SculptorNode : public Base {
	public:
		template <typename... Args>
		static Derived* create(Args&&... args) {
			auto node = new Derived();
			if (node->init(std::forward<Args>(args)...)) {
				node->autorelease();
				return node;
			}
			delete node;
			return nullptr;
		}

		template <typename... Args>
		bool init(Args&&... args) {
			if (!Base::init()) return false;

			static_cast<Derived*>(this)->setup(std::forward<Args>(args)...);

			return true;
		}
	};

	CCSprite* objectSpriteFromID(int ID);
	CCSprite* objectButtonSpriteFromID(int ID);
	CCSprite* colorSpriteFromID(int ID);

	void exportSprites();

	std::pair<CCPoint, CCPoint> getSpriteConnections(int ID);

	float sign(float a);

	float sinTime(float frequency, float phase, float amplitude);

	float lerp(float a, float b, float mix);

	float inverseLerp(float a, float b, float t);

	float clamp(float value, float a, float b);

	float wrap(float value, float min, float max);

	bool isClose(float a, float b, float precision = 0.01f);

	bool isClose(CCPoint a, CCPoint b, float precision = 0.01f);

	float roundTo(float value, int places);

	CCPoint roundTo(CCPoint value, int places);

	bool rangeContains(float start, float end, float value);

	int safeModulo(int a, int b);

	NineSlice* createBase(CCSize size, CCSize padding = { 0, 0 });

	int validateID(int ID);

	float noiseXY(CCPoint point);

	float noiseIndex(int i);

	CCPoint toEditorSpace(CCPoint point);

	CCPoint fromEditorSpace(CCPoint point);

	bool ctrl();
	bool alt();
	bool shift();

	static constexpr float gdUnit = 30.f;

	static constexpr int bezierMinEdges = 5;
	static constexpr int bezierMaxEdges = 12;
	static constexpr int bezierLengthFactor = 5;
	static constexpr int bezierCurveFactor = 1;

	enum class FormMode {
		Closed,
		Open
	};

	enum class ClipboardState {
		Form,
		Layer,
		Modulator
	};

	enum class Direction {
		Horizontal,
		Vertical
	};

	enum class LayerTab {
		Style,
		Offset,
		Group,
	};

	enum class PoolType {
		None,
		ID,
		Color
	};

	enum class GDObject {
		Block,
		UnitSquare,
		HalfSquare,
		QuarterSquare,
		UnitCircle,
		BigCircle,
		QuarterCircle,
		Line,
		ThickLine,
		UnitGlow,
		HalfGlow,
		QuarterGlow,
		QuarterGlowLine,
		QuarterGlowCorner,
		UnitTriangle		
	};

	inline std::unordered_map<GDObject, int> objectIDs = {
		{GDObject::Block, 1},
		{GDObject::UnitSquare, 211},
		{GDObject::HalfSquare, 916},
		{GDObject::QuarterSquare, 917},
		{GDObject::UnitCircle, 3621},
		{GDObject::BigCircle, 3637},
		{GDObject::QuarterCircle, 1764},
		{GDObject::Line, 1753},
		{GDObject::ThickLine, 1756},
		{GDObject::UnitGlow, 1888},
		{GDObject::HalfGlow, 1886},
		{GDObject::QuarterGlow, 1887},
		{GDObject::QuarterGlowLine, 1292},
		{GDObject::QuarterGlowCorner, 1009},
		{GDObject::UnitTriangle, 693},
	};


}