#pragma once

#include <Geode/Geode.hpp>
#include "mod/UI/Graph.hpp"

using namespace geode::prelude;

namespace Sculptor {

	void Graph::setup(CCSize size) {
		this->setContentSize(size);		
		this->addChildAtPosition(createBase(size), Anchor::Center);
	}

	void Graph::redraw() {

		for (int i = 0; i < steps; i++) {
			float t = ((float)i / steps - 0.5) * 2;
			float sample = func(t);
		}

	}

}