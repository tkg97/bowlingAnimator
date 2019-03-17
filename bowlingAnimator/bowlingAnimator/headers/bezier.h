#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

int binomialCoeff(int n, int k)
{
	// Base Cases 
	if (k == 0 || k == n)
		return 1;

	// Recur 
	return  binomialCoeff(n - 1, k - 1) + binomialCoeff(n - 1, k);
}

glm::vec3 bezierLocation(std::vector<std::vector<float>> points, float u) {
	std::vector<float> result;
	int n = points.size() - 1;
	for (int i = 0; i < points[0].size(); i++) {
		float g = 0;
		for (int j = 0; j <= n; j++) {
			g += binomialCoeff(n, j)*pow(1 - u, n - j)*pow(u, j)*points[j][i];
		}
		result.push_back(g);
	}
	glm::vec3 resVec = glm::make_vec3(&result[0]);
	return resVec;
}