#pragma once

#include <Geode/Geode.hpp>
#include "mod/UI/PropertyInput.hpp"
#include "mod/Property.hpp"

using namespace geode::prelude;

namespace Sculptor {

    void PropertyInput::setup(Property* property, CCSize size) {

        mouseListener = MouseInputEvent().listen([this](MouseInputData data) { this->handleMouseData(data); });
        scrollListener = ScrollWheelEvent().listen([this](double x, double y) -> ListenerResult { return this->handleScroll(x, y); });
        scheduleUpdate();

        this->setContentSize(size);
        base = createBase(size);
        this->addChildAtPosition(base, Anchor::Center);       
       
        this->label->setScale(0.6);
        this->addChildAtPosition(this->label, Anchor::Top, { 0, -7 });      

        valueInput = TextInput::create(size.width, "", "bigFont.fnt");   
        this->addChildAtPosition(valueInput, Anchor::Center, { 0, -5 });

        setProperty(property);
    }

    void PropertyInput::setProperty(Property* property) {
        this->property = property;
        label->setString(property->info.label.c_str());
        valueInput->setCommonFilter(property->info.filter);
        reportValue(property->getBaseValue());

    }



    void PropertyInput::reportValue(float value) {             
        float clamped = clamp(value, property->info.min, property->info.max);        
        property->setBaseValue(clamped);
        setStringFromValue();
        
            
    }

    void PropertyInput::update(float dt) {
        for (int i = 0; i < valueInput->getString().size(); i++) {
            spriteAtIndex(i)->setColor(normalColor);
        }

        if (!typing) {
            updateFocusIndex();
        }
    }

    void PropertyInput::setStringFromValue() {
        auto value = property->getBaseValue();
        int count = property->info.leadingDigits + property->info.trailingDigits;
        std::string padding = " ";
        if (value < 0) {
            count++;
            padding = "";
        }
        if (property->info.filter == CommonFilter::Float) {
            count++;
        }
        auto str = padding + std::format("{:0{}.{}f}", value, count, property->info.trailingDigits);
        valueInput->setString(str);
        auto text = valueInput->getChildByType<CCTextInputNode>(0)->getChildByType<CCLabelBMFont>(0);
        if (value == 0) {
            text->setOpacity(50);
        }
        else {
            text->setOpacity(255);
        }
    }

    void PropertyInput::setValueFromString() {
        try {
            reportValue(std::stof(valueInput->getString()));
        }
        catch (...) {
            reportValue(0);
        }       
    }

    void PropertyInput::updateFocusIndex() {
        focusIndex = std::nullopt;

        for (int i = 0; i < property->info.leadingDigits + property->info.trailingDigits; i++) {
            int index = digitToStringIndex(i);
            auto sprite = spriteAtIndex(index);
            auto box = sprite->boundingBox();
            auto origin = convertToNodeSpace(sprite->getParent()->convertToWorldSpace(box.origin));
            auto corner = convertToNodeSpace(sprite->getParent()->convertToWorldSpace(box.origin + box.size));
            CCRect rect = { origin.x, origin.y, corner.x - origin.x,  corner.y - origin.y };
            rect.inflateRect(-1);
            if (rect.containsPoint(convertToNodeSpace(getMousePos()))) {
                sprite->setColor(highlightColor);
                focusIndex = i;
                return;
            }
        }
    }

    ListenerResult PropertyInput::handleMouseData(MouseInputData data) {
        //if (!hasFocus()) return ListenerResult::Propagate;

        if (data.action == MouseInputData::Action::Press) {
            CCRect rect = { 0, 0, boundingBox().size.width, boundingBox().size.height };
            if (rect.containsPoint(convertToNodeSpace(getMousePos()))) {
                valueInput->setString("");
                typing = true;
                return ListenerResult::Stop;
            }
            else if (typing) {
                setValueFromString();
                typing = false;
            }
        }
        return ListenerResult::Propagate;
    }

    ListenerResult PropertyInput::handleScroll(double x, double y) {
        //if (!hasFocus()) return ListenerResult::Propagate;

        if (focusIndex.has_value() && !typing) {
            int i = focusIndex.value();
            float m = magnitudeAtIndex(i);
            float v = property->getBaseValue() + y * m;
            reportValue(v);
            return ListenerResult::Stop;
        }
        return ListenerResult::Propagate;
    }

    int PropertyInput::digitToStringIndex(int i) {
        return (i < property->info.leadingDigits) ? i + 1 : i + 2;
    }

    float PropertyInput::magnitudeAtIndex(int i) {
        if (i < property->info.leadingDigits) {
            return std::pow(10, (property->info.leadingDigits - i - 1));
        }
        else {
            return std::pow(10, -(i - property->info.leadingDigits + 1));
        }
    }

    CCFontSprite* PropertyInput::spriteAtIndex(int i) {
        return static_cast<CCFontSprite*>(valueInput->getChildByType<CCTextInputNode>(0)->getChildByType<CCLabelBMFont>(0)->getChildByIndex(i));
    }

}