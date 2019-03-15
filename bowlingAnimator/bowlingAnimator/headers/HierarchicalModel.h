#pragma once
#include "Hierarchy.h"
#include <Vector>
#include <stack>
#include <glm/glm.hpp>

std::vector<glm::mat4> computeModelMatrices(const Hierarchy& hierarchy);

// since the correct input ensures that the graph will be DAG, any possibility of cycle is not considered
void dfsTraversal(std::stack<glm::mat4> &helperStack, int currentNode, const std::vector< std::vector<int> >& adjList,
	const std::vector< glm::mat4 >& modelMatrices, std::vector< glm::mat4 > &outputModelMatrices);