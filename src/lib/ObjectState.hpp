#pragma once

#include <Geode/Geode.hpp>
#include "Geometry.hpp"
#include "Utilities.hpp"

using namespace geode::prelude;

namespace Sculptor {

	struct ObjectState {

		int ID = 0;

		float x = 0;
		float y = 0;
		float rotation = 0;
		float rotationX = 0;
		float rotationY = 0;
		float scale = 1;
		float scaleX = 1;
		float scaleY = 1;		
		bool flipX = false;
		bool flipY = false;

		int zLayer = 0;
		int zOrder = 0;
		int editorLayer = 0;

		int color = 0;
		int secondaryColor = 0;
		float hue = 0;
		float saturation = 0;
		float value = 0;
		bool absoluteBrightness = true;
		bool absoluteSaturation = true;

		std::vector<int> groups;

		GameObject* createObject();

		GameObject* applyTo(GameObject* object);

		static ObjectState fromTriangle(const Triangle& triangle);
		static ObjectState fromLine(const Line& line, float width);
		static ObjectState fromCircle(const Circle& circle);

		ObjectState compose(const ObjectState& other);
	};

}

template <>
struct fmt::formatter<Sculptor::ObjectState> {
	constexpr auto parse(fmt::format_parse_context& ctx) {
		return ctx.begin();
	}

	auto format(const Sculptor::ObjectState& obj, fmt::format_context& ctx) const {
		fmt::format_to(ctx.out(),
			"ID: {}\n"
			"x: {}\n"
			"y: {}\n"
			"rotation: {}\n"
			"rotationX: {}\n"
			"rotationY: {}\n"
			"scale: {}\n"
			"scaleX: {}\n"
			"scaleY: {}\n"			
			"flipX: {}\n"
			"flipY: {}\n"
			"zLayer: {}\n"
			"zOrder: {}\n"
			"editorLayer: {}\n"
			"color: {}\n"
			"secondaryColor: {}\n"
			"hue: {}\n"
			"saturation: {}\n"
			"value: {}\n"
			"absoluteBrightness: {}\n"
			"absoluteSaturation: {}\n"
			,
			obj.ID,
			obj.x,
			obj.y,
			obj.rotation,
			obj.rotationX,
			obj.rotationY,
			obj.scale,
			obj.scaleX,
			obj.scaleY,		
			obj.flipX,
			obj.flipY,
			obj.zLayer,
			obj.zOrder,
			obj.editorLayer,
			obj.color,
			obj.secondaryColor,
			obj.hue,
			obj.saturation,
			obj.value,
			obj.absoluteBrightness,
			obj.absoluteSaturation
		);
		return ctx.out();
	}
};