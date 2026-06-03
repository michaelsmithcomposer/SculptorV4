
#include <Geode/Geode.hpp>
#include "lib/UINode.hpp"

using namespace geode::prelude;

namespace Sculptor {

	void UINode::setup(CCPoint position, Type type, bool canDrag) {

		scheduleUpdate();
		CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, 0, true);
		mouseListener = MouseInputEvent().listen([this](MouseInputData data) { return this->handleMouseData(data); });

		setContentSize({ 11, 11 });
		setPosition(position);
		setType(type);

		this->canDrag = canDrag;

	}

	void UINode::onExit() {
		CCDirector::sharedDirector()->getTouchDispatcher()->removeDelegate(this);
		CCLayer::onExit();
	}

	void UINode::setType(Type type) {
		this->type = type;
		if (sprite) {
			sprite->removeFromParent();
		}
		sprite = CCSprite::createWithSpriteFrameName(textures[type].c_str());
		sprite->setScale(getContentHeight() / sprite->getContentHeight());
		addChildAtPosition(sprite, Anchor::Center);
	}

	void UINode::startDrag() {
		inDrag = true;
		auto position = getPosition();
		offset = getParent()->convertToNodeSpace(getMousePos()) - position;
	}

	void UINode::stopDrag() {
		inDrag = false;
	}

	void UINode::setActive(bool active) {
		setVisible(active);
		this->active = active;
	}

	void UINode::update(float dt) {
		if (!canDrag) return;

		auto position = getPosition();
		if (inDrag) {
			CCPoint mouse = getParent()->convertToNodeSpace(getMousePos());
			setPosition(mouse - offset);
			if (onMove && (position != lastPosition)) onMove(this, position);
		}
		lastPosition = position;
	}

	ListenerResult UINode::handleMouseData(MouseInputData data) {
		if (!active) return ListenerResult::Propagate;

		if (data.action == MouseInputData::Action::Press) {
			if (sprite->boundingBox().containsPoint(convertToNodeSpace(getMousePos()))) {
				if (data.button == MouseInputData::Button::Right && onRightClick) {					
					onRightClick(this, getPosition());									
				}
				else if (data.button == MouseInputData::Button::Middle && onMiddleClick) {
					onMiddleClick(this, getPosition());
				}
			}
		}
		else {
			inDrag = false;
		}
		return ListenerResult::Propagate;
	}

	bool UINode::ccTouchBegan(CCTouch* touch, CCEvent* event) {
		if (!active) return false;
		if (!sprite->boundingBox().containsPoint(convertToNodeSpace(touch->getLocation()))) return false;

		if (canDrag) {
			startDrag();
		}
		if (onClick) {
			onClick(this, getPosition());
		}

		return true;
	}












}
