#pragma once

#include <Geode/Geode.hpp>
#include "lib/Geometry.hpp"

using namespace geode::prelude;
using namespace Sculptor;

namespace Sculptor {
    class Layer;
    class Modulator;
    class Form;
    class Property;
    class Manager;
    class VectorEditor;
}

template <>
struct matjson::Serialize<CCPoint> {
    static matjson::Value toJson(const CCPoint& point);
    static geode::Result<CCPoint> fromJson(const matjson::Value& value); 
};

template <>
struct matjson::Serialize<BezierCurve> {
    static matjson::Value toJson(const BezierCurve& curve);
    static geode::Result<BezierCurve> fromJson(const matjson::Value& value);   
};

template <>
struct matjson::Serialize<Property*> {
    static matjson::Value toJson(Property* const& property);
    static geode::Result<Property*> fromJson(const matjson::Value& value);
};


template <>
struct matjson::Serialize<Layer*> {
    static matjson::Value toJson(Layer* const& layer);
    static geode::Result<Layer*> fromJson(const matjson::Value& value);
};

template <>
struct matjson::Serialize<Modulator*> {
    static matjson::Value toJson(Modulator* const& modulator);
    static geode::Result<Modulator*> fromJson(const matjson::Value& value);
};

template <>
struct matjson::Serialize<VectorEditor*> {
    static matjson::Value toJson(const VectorEditor* editor);
    static geode::Result<VectorEditor*> fromJson(const matjson::Value& value);
};


template <>
struct matjson::Serialize<Form*> {
    static matjson::Value toJson(const Form* form);
    static geode::Result<Form*> fromJson(const matjson::Value& value);
};

template <>
struct matjson::Serialize<Manager*> {
    static matjson::Value toJson(const Manager* manager);
    static geode::Result<Manager*> fromJson(const matjson::Value& value);
};



