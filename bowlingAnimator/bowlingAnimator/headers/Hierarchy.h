#pragma once
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

struct Hierarchy {
	int rootNode;
	std::unordered_map<std::string, int> bodyParts;
	std::vector< std::vector<int> > adjList;
	std::vector< glm::mat4 > initialTranslationMatrices;
	std::vector< glm::mat4 > rotationMatrices;
	std::vector< glm::mat4 > finalTranslationMatrices;
	std::vector< glm::mat4 > scaleMatrices;
};
