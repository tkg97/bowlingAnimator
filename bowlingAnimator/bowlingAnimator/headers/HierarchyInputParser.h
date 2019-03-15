#pragma once
#include "json/json.h"
#include "Hierarchy.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

glm::mat4 parseJsonArrayIntoMatrix(const Json::Value& matrix);

glm::mat4 parseJsonArrayIntoScalingMatrix(const Json::Value & parameters);

void parseJsonArrayIntoHashMap(const Json::Value &allParts, std::unordered_map<std::string, int>& data);

void parseJsonArrayIntoAdjList(const std::unordered_map<std::string, int>& bodyParts, const Json::Value &edges, std::vector<std::vector<int>>& adjList);

void parseJsonValueIntoMatrices(const std::unordered_map<std::string, int>& bodyParts, std::vector<glm::mat4>& modelMatrices, std::vector<glm::mat4>& scaleMatrices, const Json::Value& parameters);

Hierarchy parseJsonHierarchy(std::string path);