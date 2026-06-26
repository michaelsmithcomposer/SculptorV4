#pragma once

#include <Geode/Geode.hpp>
#include "lib/Utilities.hpp"


using namespace geode::prelude;

namespace Sculptor {

    class Property;
    class PropertyInput;
   
    class ModulatablePropertyInput : public SculptorNode<ModulatablePropertyInput> {
    public:

        void setup(Property* property, CCSize size);

        void updateTextColor();
        void setBaseColor(ccColor3B color) { base->setColor(color); }

    private:

        Property* property;
        Property* baseValue;
        Property* modValue;

        NineSlice* base;
        CCLabelBMFont* label = CCLabelBMFont::create("", "chatFont.fnt");
        PropertyInput* baseEditor;
        PropertyInput* modEditor;  

        void onExit();

    };

}