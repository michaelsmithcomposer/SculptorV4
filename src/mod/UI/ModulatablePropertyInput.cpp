#pragma once

#include <Geode/Geode.hpp>"
#include "mod/UI/ModulatablePropertyInput.hpp"
#include "mod/Property.hpp"
#include "mod/UI/PropertyInput.hpp"
#include "mod/Manager.hpp"
#include "mod/Form/Modulator.hpp"

using namespace geode::prelude;

namespace Sculptor {

    void ModulatablePropertyInput::setup(Property* property, CCSize size) {

        

        this->property = property;

        this->setContentSize(size);
        base = createBase(size);
        this->addChildAtPosition(base, Anchor::Center);
        label->setString(property->info.label.c_str());
        label->setScale(0.6);
        this->addChildAtPosition(label, Anchor::Top, { 0, -7 });

        auto node = CCNode::create();
        node->setContentSize(size);
        node->setLayout(RowLayout::create()->setGap(0)->setAxisAlignment(AxisAlignment::Center));

        baseValue = new Property{ {
            .label = "",
            .isModulatable = false,
            .defaultValue = property->getBaseValue(),
            .filter = property->info.filter,
            .leadingDigits = property->info.leadingDigits,
            .trailingDigits = property->info.trailingDigits,
            .min = property->info.min,
            .max = property->info.max
            } };

        baseValue->setCallback([this](Property* prop) { this->property->setBaseValue(prop->getBaseValue()); });
        baseEditor = static_cast<PropertyInput*>(baseValue->createUI({ size.width / 2, size.height }));
        baseEditor->setBaseVisible(false);
        node->addChild(baseEditor);

        modValue = new Property{ {
            .label = "",
            .isModulatable = false,
            .defaultValue = property->getModValue(Manager::get()->selectedModulator),
            .filter = property->info.filter,
            .leadingDigits = property->info.leadingDigits,
            .trailingDigits = property->info.trailingDigits,            
            } };

        modValue->setCallback([this](Property* prop) {
            this->property->setModValue(Manager::get()->selectedModulator, prop->getBaseValue());
            updateTextColor();
        });
        modEditor = static_cast<PropertyInput*>(modValue->createUI({ size.width / 2, size.height }));
        modEditor->setBaseVisible(false);
        if (Manager::get()->selectedModulator) {
            modEditor->setColor(Manager::get()->selectedModulator->color);
            modEditor->setHighlightColor({ 255, 255, 255 });
        }
        node->addChild(modEditor);

        node->updateLayout();
        addChildAtPosition(node, Anchor::BottomLeft);
    }

    void ModulatablePropertyInput::updateTextColor() {
        auto str = std::string(label->getString());
        str = str.substr(0, str.find("*"));
        std::vector <std::pair<int, ccColor3B>> colors;
        for (const auto& [modulator, amount] : property->getModValues()) {
            if (amount != 0) {
                colors.push_back(std::make_pair(str.size(), modulator->color));
                str += "*";
            }
        }
        label->setString(str.c_str());
        for (const auto& [i, color] : colors) {
            static_cast<CCFontSprite*>(label->getChildByIndex(i))->setColor(color);
        }
    }

}