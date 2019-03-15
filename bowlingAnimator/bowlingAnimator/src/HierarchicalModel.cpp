#include "HierarchicalModel.h"

void dfsTraversal(std::stack<glm::mat4> &helperStack, int currentNode, const std::vector< std::vector<int> >& adjList, 
	const std::vector< glm::mat4 >& modelMatrices, std::vector< glm::mat4 > &outputModelMatrices) {
	
	glm::mat4 currentMatrix = modelMatrices[currentNode];
	if (!helperStack.empty()) {
		currentMatrix *= helperStack.top();
	}

	helperStack.push(currentMatrix);

	for (int i = 0;i < adjList[currentNode].size();i++) {
		int node = adjList[currentNode][i];
		dfsTraversal(helperStack, node, adjList, modelMatrices, outputModelMatrices);
	}

	helperStack.pop();
	outputModelMatrices[currentNode] = currentMatrix;
}

std::vector<glm::mat4> computeModelMatrices(const Hierarchy& hierarchy) {
	std::vector<glm::mat4> outputModelMatrices(hierarchy.modelMatrices.size());
	std::stack<glm::mat4> helperStack;
	dfsTraversal(helperStack, hierarchy.rootNode, hierarchy.adjList, hierarchy.modelMatrices, outputModelMatrices);
	return outputModelMatrices;
}