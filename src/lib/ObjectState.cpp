#pragma once

#include <Geode/Geode.hpp>
#include "ObjectState.hpp"

using namespace geode::prelude;

namespace Sculptor {

	GameObject* ObjectState::createObject() {
		return applyTo(EditorUI::get()->createObject(validateID(ID), { x, y }));
	}

	GameObject* ObjectState::applyTo(GameObject* object) {

		object->setPositionX(x);
		object->setPositionY(y);
		object->updateCustomScaleX(scale * scaleX);
		object->updateCustomScaleY(scale * scaleY);
		object->setRotationX(-(rotation + rotationX));
		object->setRotationY(-(rotation + rotationY));

		auto arr = CCArray::create();
		arr->addObject(object);
		if (flipX) EditorUI::get()->flipObjectsX(arr);
		if (flipY) EditorUI::get()->flipObjectsY(arr);
	
		object->setCustomZLayer(zLayer);
		object->m_zOrder = zOrder;
		object->m_editorLayer = editorLayer;

		object->setMainColorMode(std::max(color, 1));
		object->setSecondaryColorMode(std::max(secondaryColor, 1));
		object->m_baseColor->m_hsv.h = hue;
		object->m_baseColor->m_hsv.s = saturation;
		object->m_baseColor->m_hsv.v = value;		
		object->m_baseColor->m_hsv.absoluteBrightness = absoluteBrightness;
		object->m_baseColor->m_hsv.absoluteSaturation = absoluteSaturation;

		object->resetGroups();
		object->resetColorGroups();
		for (const auto& groupID : groups) {
			int added = object->addToGroup(groupID);
			if (added) {
				LevelEditorLayer::get()->addToGroup(object, groupID, false);
			}			
		}		

		object->m_updateParents = true;
		LevelEditorLayer::get()->updateObjectSection(object);
		LevelEditorLayer::get()->reorderObjectSection(object);
		LevelEditorLayer::get()->resetToggledGroupsAndObjects();			
		LevelEditorLayer::get()->m_unk3b22 = true;

		return object;
	}

	ObjectState ObjectState::compose(const ObjectState& other) {
		auto g = groups;
		g.append_range(other.groups);
		return ObjectState{
			.ID = ID + other.ID,

			.x = x + other.x,
			.y = y + other.y,
			.rotation = rotation + other.rotation,
			.rotationX = rotationX + other.rotationX,
			.rotationY = rotationY + other.rotationY,
			.scale = scale * other.scale,
			.scaleX = scaleX * other.scaleX,
			.scaleY = scaleY * other.scaleY,
			.flipX = flipX != other.flipX,
			.flipY = flipY != other.flipY,

			.zLayer = zLayer + other.zLayer,
			.zOrder = zOrder + other.zOrder,
			.editorLayer = editorLayer + other.editorLayer,

			.color = color + other.color,
			.secondaryColor = secondaryColor + other.secondaryColor,
			.hue = hue + other.hue,
			.saturation = saturation + other.saturation,
			.value = value + other.value,			
			.absoluteBrightness = absoluteBrightness || other.absoluteBrightness,
			.absoluteSaturation = absoluteSaturation || other.absoluteSaturation,

			.groups = g,

		};
	}

	ObjectState ObjectState::fromTriangle(const Triangle& triangle) {

		CCPoint position = triangle.a().lerp(triangle.c(), 0.5);

		CCPoint basisX = (triangle.b() - triangle.a());
		CCPoint basisY = (triangle.c() - triangle.b());

		float angleX = atan2(basisX.y, basisX.x);
		float angleY = atan2(basisY.y, basisY.x);
		return ObjectState{
			.ID = objectIDs[GDObject::UnitTriangle],
			.x = position.x,
			.y = position.y,
			.rotationX = -CC_RADIANS_TO_DEGREES(angleY + HALF_PI),
			.rotationY = -CC_RADIANS_TO_DEGREES(angleX),
			.scaleX = basisX.getLength() / gdUnit + 0.01f,
			.scaleY = basisY.getLength() / gdUnit + 0.01f,			
			.flipY = true,
		};
	}

	ObjectState ObjectState::fromLine(const Line& line, float width) {
		CCPoint position = line.lerp(0.5);
		return ObjectState{
			.ID = objectIDs[GDObject::Line],
			.x = position.x,
			.y = position.y,
			.rotation = CC_RADIANS_TO_DEGREES(line.angle()),
			.scaleX = line.length() / gdUnit,
			.scaleY = width

		};
	}

	ObjectState ObjectState::fromCircle(const Circle& circle) {
		return ObjectState{
			.ID = objectIDs[GDObject::UnitCircle],
			.x = circle.origin.x,
			.y = circle.origin.y,
			.scaleX = (circle.radius * 2) / gdUnit,
			.scaleY = (circle.radius * 2) / gdUnit
		};
	}

}