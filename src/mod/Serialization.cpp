#include <Geode/Geode.hpp>
#include "mod/Serialization.hpp"
#include "mod/Property.hpp"
#include "mod/Form/Layer.hpp"
#include "mod/Form/Modulator.hpp"
#include "mod/Form/Form.hpp"
#include "mod/Manager.hpp"
#include "mod/Form/VectorEditor.hpp"

using namespace geode::prelude;
using namespace Sculptor;

matjson::Value matjson::Serialize<CCPoint>::toJson(const CCPoint& point) {
	return matjson::makeObject({ {"x", point.x}, {"y", point.y} });
}

geode::Result<CCPoint> matjson::Serialize<CCPoint>::fromJson(const matjson::Value& value) {
	GEODE_UNWRAP_INTO(double x, value["x"].asDouble());
	GEODE_UNWRAP_INTO(double y, value["y"].asDouble());
	return geode::Ok(ccp(x, y));
}

//

matjson::Value matjson::Serialize<BezierCurve>::toJson(const BezierCurve& curve) {
	return matjson::makeObject({ {"points", curve.points() } });
}

geode::Result<BezierCurve> matjson::Serialize<BezierCurve>::fromJson(const matjson::Value& value) {
	GEODE_UNWRAP_INTO(Points points, value["points"].as<Points>());
	return geode::Ok(BezierCurve(points));
}



//

matjson::Value matjson::Serialize<Property*>::toJson(Property* const& property) {

	auto info = matjson::makeObject({
		{"label", property->info.label},
		{"isModulatable", property->info.isModulatable},
		{"defaultValue", property->info.defaultValue},
		{"filter", static_cast<int>(property->info.filter)},
		{"leadingDigits", property->info.leadingDigits},
		{"trailingDigits", property->info.trailingDigits},
		{"min", property->info.min},
		{"max", property->info.max}, 
		{"valuePool", property->info.valuePool},
		{"poolType", static_cast<int>(property->info.poolType)}
	});

	std::vector<matjson::Value> modValues;
	for (const auto& [modulator, amount] : property->getModValues()) {
		modValues.push_back(matjson::makeObject({
			{"formID", Manager::get()->getFormID(modulator->form)},
			{"modulatorID", modulator->form->getModulatorID(modulator)},
			{"amount", amount}
		}));
	}

	return matjson::makeObject({ {"info", info}, {"baseValue", property->getBaseValue()}, {"modValues", modValues} });
}

geode::Result<Property*> matjson::Serialize<Property*>::fromJson(const matjson::Value& value) {	

	GEODE_UNWRAP_INTO(std::string label,  value["info"]["label"].as<std::string>());
	GEODE_UNWRAP_INTO(bool isModulatable, value["info"]["isModulatable"].as<bool>());
	GEODE_UNWRAP_INTO(float defaultValue, value["info"]["defaultValue"].as<float>());
	GEODE_UNWRAP_INTO(int filter,		  value["info"]["filter"].as<int>());
	GEODE_UNWRAP_INTO(int leadingDigits,  value["info"]["leadingDigits"].as<int>());
	GEODE_UNWRAP_INTO(int trailingDigits, value["info"]["trailingDigits"].as<int>());
	GEODE_UNWRAP_INTO(float min,		  value["info"]["min"].as<float>());
	GEODE_UNWRAP_INTO(float max,		  value["info"]["max"].as<float>());
	GEODE_UNWRAP_INTO(std::vector<float> valuePool, value["info"]["valuePool"].as<std::vector<float>>());
	GEODE_UNWRAP_INTO(int poolType, value["info"]["poolType"].as<int>());

	GEODE_UNWRAP_INTO(float baseValue, value["baseValue"].as<float>());

	std::unordered_map<Modulator*, float> modValues;

	for (const auto& modulator : value["modValues"]) {
		GEODE_UNWRAP_INTO(int formID, modulator["formID"].as<int>());
		GEODE_UNWRAP_INTO(int modulatorID, modulator["modulatorID"].as<int>());		
		GEODE_UNWRAP_INTO(float amount, modulator["amount"].as<float>());
		auto form = Manager::get()->forms[formID];
		if (form) {
			modValues[form->modulators[modulatorID]] = amount;
		}
		
	}

	Property* prop = Property::create( {
		.label = label,
		.isModulatable = isModulatable,
		.defaultValue = defaultValue,
		.filter = static_cast<CommonFilter>(filter),
		.leadingDigits = leadingDigits,
		.trailingDigits = trailingDigits,
		.min = min,
		.max = max,
		.valuePool = valuePool,
		.poolType = static_cast<PoolType>(poolType)
	}, baseValue, modValues);

	return geode::Ok(prop);
}

//


matjson::Value matjson::Serialize<Layer*>::toJson(Layer* const& layer) {

	std::vector<matjson::Value> properties;
	for (const auto& property : layer->getNamedProperties()) {
		properties.push_back(matjson::Serialize<Property*>::toJson(property));
	}

	std::vector<matjson::Value> groupProperties;
	for (const auto& property : layer->groups) {
		groupProperties.push_back(matjson::Serialize<Property*>::toJson(property));
	}


	return matjson::makeObject({ { "label", layer->label }, { "properties", properties }, {"groupProperties", groupProperties}});
}

geode::Result<Layer*> matjson::Serialize<Layer*>::fromJson(const matjson::Value& value) {

	GEODE_UNWRAP_INTO(std::string label, value["label"].as<std::string>());
	Layer* layer;

	for (const auto& prototype : Layer::prototypes()) {
		if (label == prototype->label) {
			layer = static_cast<Layer*>(prototype->clone());

			for (const auto& propValue : value["properties"]) {
				GEODE_UNWRAP_INTO(std::string propLabel, propValue["info"]["label"].as<std::string>());
				layer->getPropertyByLabel(propLabel).value()->setFromJson(propValue);
			}

			for (const auto& groupValue : value["groupProperties"]) {
				auto property = matjson::Serialize<Property*>::fromJson(groupValue).unwrap();
				layer->groups.push_back(property);
			}

			break;
		}
	}

	return geode::Ok(layer);
}

//

matjson::Value matjson::Serialize<Modulator*>::toJson(Modulator* const& modulator) {
	std::vector<matjson::Value> properties;

	for (const auto& property : modulator->getProperties()) {
		properties.push_back(matjson::Serialize<Property*>::toJson(property));
	}

	return matjson::makeObject({ { "label", modulator->label }, { "properties", properties } });
}

geode::Result<Modulator*> matjson::Serialize<Modulator*>::fromJson(const matjson::Value& value) {
	GEODE_UNWRAP_INTO(std::string label, value["label"].as<std::string>());
	Modulator* modulator;

	for (const auto& prototype : Modulator::prototypes()) {
		if (label == prototype->label) {
			modulator = static_cast<Modulator*>(prototype->clone());

			for (const auto& propValue : value["properties"]) {
				GEODE_UNWRAP_INTO(std::string propLabel, propValue["info"]["label"].as<std::string>());
				modulator->getPropertyByLabel(propLabel).value()->setFromJson(propValue);
			}

			break;
		}
	}


	return geode::Ok(modulator);
}


//

matjson::Value matjson::Serialize<VectorEditor*>::toJson(const VectorEditor* editor) {
	std::vector<matjson::Value> result;
	for (const auto& treeNode : editor->nodes) {
		Points points;
		for (const auto& controlNode : treeNode->controlNodes) {
			points.push_back(controlNode->getPosition());
		}
		result.push_back(matjson::makeObject({
			{"position", treeNode->node->getPosition()},
			{"parentID", editor->getTreeNodeID(treeNode->parent)},
			{"isRoot", (editor->root == treeNode)},
			{"points", points}
		}));
	}

	return matjson::makeObject({ { "nodes", result } });
}

geode::Result<VectorEditor*> matjson::Serialize<VectorEditor*>::fromJson(const matjson::Value& value) {
	VectorEditor* editor = VectorEditor::create();

	std::vector<TreeNode*> nodes;

	for (const auto& node : value["nodes"]) {
		GEODE_UNWRAP_INTO(CCPoint position, node["position"].as<CCPoint>());
		auto newNode = TreeNode::create(position, editor);
		nodes.push_back(newNode);
	}

	for (const auto& [newNode, jsonNode] : std::views::zip(nodes, value["nodes"])) {
		GEODE_UNWRAP_INTO(int ID, jsonNode["parentID"].as<int>());
		GEODE_UNWRAP_INTO(bool isRoot, jsonNode["isRoot"].as<bool>());
		if (ID >= 0) {
			newNode->setNewParent(nodes[ID]);
		}
		if (isRoot) {
			editor->root = newNode;
		}
		GEODE_UNWRAP_INTO(Points points, jsonNode["points"].as<std::vector<CCPoint>>());
		for (const auto& point : points) {
			newNode->addControlNode(point, UINode::Type::Bezier);
		}
	}

	editor->setAllVisible(false);
	return geode::Ok(editor);
}

//

matjson::Value matjson::Serialize<Form*>::toJson(const Form* form) {

	std::vector<matjson::Value> modulators;
	for (const auto& modulator : form->modulators) {
		modulators.push_back(matjson::Serialize<Modulator*>::toJson(modulator));
	}

	std::vector<matjson::Value> layers;
	for (const auto& layer : form->layers) {
		layers.push_back(matjson::Serialize<Layer*>::toJson(layer));
	}

	return matjson::makeObject({
		{"mode", static_cast<int>(form->mode)},
		{"position", const_cast<Form*>(form)->getPosition()},
		{"editor", matjson::Serialize<VectorEditor*>::toJson(form->vectorEditor)},
		{"modulators", modulators},
		{"layers", layers}
	});
}

geode::Result<Form*> matjson::Serialize<Form*>::fromJson(const matjson::Value& value) {
	GEODE_UNWRAP_INTO(int mode, value["mode"].as<int>());
	GEODE_UNWRAP_INTO(CCPoint position, value["position"].as<CCPoint>());

	auto form = Form::create(static_cast<FormMode>(mode), position);	
	Manager::get()->registerForm(form);

	auto editor = matjson::Serialize<VectorEditor*>::fromJson(value["editor"]).unwrap();
	form->registerVectorEditor(editor);

	for (const auto& modulatorValue : value["modulators"]) {
		auto modulator = matjson::Serialize<Modulator*>::fromJson(modulatorValue);
		form->registerModulator(modulator.unwrap());
	}

	for (const auto& layerValue : value["layers"]) {
		auto layer = matjson::Serialize<Layer*>::fromJson(layerValue);
		form->registerLayer(layer.unwrap());
	}

	
	
	return geode::Ok(form);
}



//

matjson::Value matjson::Serialize<Manager*>::toJson(const Manager* manager) {

	int ID = LevelEditorLayer::get()->getNextFreeGroupID(gd::unordered_set<int>{});
	std::vector<matjson::Value> forms;
	for (const auto& form : manager->forms) {
		forms.push_back(matjson::Serialize<Form*>::toJson(form));				

		for (const auto& layer : form->layers) {
			for (auto object : layer->objects) {
				object->addToGroup(ID);
			}
		}
	}
	return matjson::makeObject({ {"forms", forms}, {"ID", ID} });

}

geode::Result<Manager*> matjson::Serialize<Manager*>::fromJson(const matjson::Value& value) {

	log::info("READING: ----------------------------------");
	log::info("{}", value.dump());

	GEODE_UNWRAP_INTO(int ID, value["ID"].as<int>());
	std::vector<GameObject*> toRemove;
	for (const auto& object : CCArrayExt<GameObject*>(LevelEditorLayer::get()->getAllObjects())) {
		if (object->belongsToGroup(ID)) {
			toRemove.push_back(object);
		}
	}
	for (auto& object : toRemove) {
		LevelEditorLayer::get()->removeObject(object, false);
	}

	for (const auto& formValue : value["forms"]) {
		auto form = matjson::Serialize<Form*>::fromJson(formValue).unwrap();			
	}

	return geode::Ok(Manager::get());
}
