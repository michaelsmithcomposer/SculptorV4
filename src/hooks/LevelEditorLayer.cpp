#include <Geode/Geode.hpp>
#include <alphalaneous.level-storage-api/include/LevelStorageAPI.hpp>
#include "mod/Serialization.hpp"
#include "mod/Manager.hpp"


using namespace geode::prelude;
using namespace Sculptor;

#include <Geode/modify/LevelEditorLayer.hpp>
struct SculptorLevelEditorLayer : Modify<SculptorLevelEditorLayer, LevelEditorLayer> {
	bool init(GJGameLevel* level, bool noUI) {
		if (!LevelEditorLayer::init(level, noUI)) return false;	
		
		Manager* manager = Manager::create();
		manager->setZOrder(10000);
		this->m_objectLayer->addChild(manager);

		

		return true;
	}	
};
