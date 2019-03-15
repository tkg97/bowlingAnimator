#pragma once
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

struct Hierarchy {
	int rootNode;
	std::unordered_map<std::string, int> bodyParts;
	std::vector< std::vector<int> > adjList;
	std::vector< glm::mat4 > modelMatrices;
};
