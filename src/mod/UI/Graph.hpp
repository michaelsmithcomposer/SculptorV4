#pragma once

#include <Geode/Geode.hpp>
#include "lib/Utilities.hpp"

using namespace geode::prelude;

namespace Sculptor {

	class Graph : public SculptorNode<Graph, CCDrawNode> {
	public:

		void setup(CCSize size);
		void setFunction(std::function<float(float)> func) { this->func = func; }

	private:

		int steps = 30;

		std::function<float(float)> func;
		void redraw();

	};

}