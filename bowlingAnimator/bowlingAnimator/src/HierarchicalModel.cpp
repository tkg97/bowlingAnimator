#include "HierarchicalModel.h"

void dfsTraversal(std::stack<glm::mat4> &helperStack, int currentNode, const std::vector< std::vector<int> >& adjList, 
	const Hierarchy& hierarchy, std::vector< glm::mat4 > &outputModelMatrices) {
	
	glm::mat4 currentMatrix;
	if (!helperStack.empty()) {
		currentMatrix = helperStack.top() * hierarchy.initialTranslationMatrices[currentNode] * hierarchy.rotationMatrices[currentNode] * 
			hierarchy.finalTranslationMatrices[currentNode];
	}
	else {
		currentMatrix = hierarchy.initialTranslationMatrices[currentNode] * hierarchy.rotationMatrices[currentNode] * hierarchy.finalTranslationMatrices[currentNode];
	}

	helperStack.push(currentMatrix);

	for (int i = 0;i < adjList[currentNode].size();i++) {
		int node = adjList[currentNode][i];
		dfsTraversal(helperStack, node, adjList, hierarchy, outputModelMatrices);
	}

	helperStack.pop();
	outputModelMatrices[currentNode] = currentMatrix;
}

std::vector<glm::mat4> computeModelMatrices(const Hierarchy& hierarchy) {
	std::vector<glm::mat4> outputModelMatrices(hierarchy.rotationMatrices.size());
	std::stack<glm::mat4> helperStack;
	dfsTraversal(helperStack, hierarchy.rootNode, hierarchy.adjList, hierarchy, outputModelMatrices);
	return outputModelMatrices;
}