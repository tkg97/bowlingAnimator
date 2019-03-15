#pragma once
#include "HierarchyInputParser.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <iostream>
using namespace std;

glm::mat4 parseJsonArrayIntoMatrix(const Json::Value& matrix) noexcept(false) {
	if (matrix.isArray() == false || matrix.size() != 4) {
		throw exception("Please make sure that input parameter matrices are valid");
	}
	vector<double> data;
	for (int i = 0;i < 4;i++) {
		if (matrix[i].isArray() == false || matrix[i].size() != 4) {
			throw exception("Please make sure that input parameter matrices are valid");
		}
		for (int j = 0;j < 4;j++) {
			data.push_back(matrix[i][j].asDouble());
		}
	}
	glm::mat4 modelMatrix = glm::make_mat4(&data[0]);
	return modelMatrix;
}

glm::vec3 parseJsonArrayIntoVector(const Json::Value& data) noexcept(false) {
	if (data.isArray() == false || data.size() != 3) {
		throw exception("Please ensure that transformation data is provided in valid format");
	}
	vector<double> dataVector;
	for (int i = 0;i < 3;i++) {
		dataVector.push_back(data[i].asDouble());
	}
	glm::vec3 dataVec = glm::make_vec3(&dataVector[0]);
	return dataVec;
}

glm::mat4 parseJsonArrayIntoTranslationMatrix(const Json::Value& parameters) noexcept(false) {
	glm::vec3 translationData = parseJsonArrayIntoVector(parameters);
	glm::mat4 translationMatrix;
	translationMatrix = glm::translate(translationMatrix, translationData);
	return translationMatrix;
}

glm::mat4 parseJsonArrayIntoScalingMatrix(const Json::Value & parameters) noexcept(false) {
	glm::vec3 scaleData = parseJsonArrayIntoVector(parameters);
	glm::mat4 scaleMatrix;
	scaleMatrix = glm::scale(scaleMatrix, scaleData);
	return scaleMatrix;
}

glm::mat4 parseJsonArrayIntoRotationMatrix(const Json::Value& parameters) noexcept(false) {
	glm::vec3 rotationData = parseJsonArrayIntoVector(parameters);
	glm::mat4 rotationMatrix;
	rotationMatrix = glm::rotate(rotationMatrix, rotationData[0], { 1,0,0 });
	rotationMatrix = glm::rotate(rotationMatrix, rotationData[1], { 0,1,0 });
	rotationMatrix = glm::rotate(rotationMatrix, rotationData[2], { 0,0,1 });
	return rotationMatrix;
}

void parseJsonArrayIntoHashMap(const Json::Value &allParts, unordered_map<string, int>& data) noexcept(false) {
	if (allParts.isArray() == false || allParts.size() == 0) {
		throw exception("Parts data Should be of array type having non zero values");
	}
	auto iter = allParts.begin();
	int count = 0;
	while (iter != allParts.end()) {
		string s = iter->asString();
		data.insert({ s, count });
		++iter;
		++count;
	}
}

void parseJsonArrayIntoAdjList(const unordered_map<string, int>& bodyParts, const Json::Value &edges, vector<vector<int>>& adjList) noexcept(false) {
	Json::Value::const_iterator iter = edges.begin();
	while (iter != edges.end()) {
		string parent = iter->get("Parent", "").asString();
		string child = iter->get("Child", "").asString();
		if (bodyParts.find(parent) == bodyParts.end() || bodyParts.find(child) == bodyParts.end()) {
			throw exception("Please make sure that input edges are valid");
		}
		int parentNode = bodyParts.at(parent);
		int childNode = bodyParts.at(child);
		adjList[parentNode].push_back(childNode);
		++iter;
	}
}

void parseJsonValueIntoMatrices(const unordered_map<string, int>& bodyParts, vector<glm::mat4>& initialTranslationMatrices, vector<glm::mat4>& rotationMatrices,
	vector<glm::mat4>& finalTranslationMatrices, vector<glm::mat4>& scaleMatrices, const Json::Value& parameters) noexcept(false) {
	unordered_map<string, int>::const_iterator iter = bodyParts.begin();
	while (iter != bodyParts.end()) {
		Json::Value matrixParameters = parameters.get(iter->first, {});
		if (matrixParameters.empty()) throw exception("Please make sure parameters are in valid format");
		Json::Value initialTranslationParameters = matrixParameters.get("InitialTranslation", {});
		Json::Value finalTranslationParameters = matrixParameters.get("FinalTranslation", {});
		Json::Value rotationParameters = matrixParameters.get("Rotation", {});
		Json::Value scaleParameters = matrixParameters.get("GlobalScaling", {});
		initialTranslationMatrices[iter->second] = parseJsonArrayIntoTranslationMatrix(initialTranslationParameters);
		rotationMatrices[iter->second] = parseJsonArrayIntoRotationMatrix(rotationParameters);
		finalTranslationMatrices[iter->second] = parseJsonArrayIntoTranslationMatrix(finalTranslationParameters);
		scaleMatrices[iter->second] = parseJsonArrayIntoScalingMatrix(scaleParameters);
		++iter;
	}
}

Hierarchy parseJsonHierarchy(string path) {
	ifstream f(path);

	int headNode;
	unordered_map<string, int> bodyParts;
	vector< vector<int> > adjList;
	vector< glm::mat4 > initialTranslationMatrices;
	vector< glm::mat4 > finalTranslationMatrices;
	vector< glm::mat4 > rotationMatrices;
	vector<glm::mat4> scaleMatrices;

	Json::Reader reader;
	Json::Value rootValue;

	if (reader.parse(f, rootValue)) {
		try {
			// Parse Parts
			Json::Value allParts = rootValue.get("Parts", {});
			parseJsonArrayIntoHashMap(allParts, bodyParts);
			adjList.resize(allParts.size());
			initialTranslationMatrices.resize(allParts.size());
			finalTranslationMatrices.resize(allParts.size());
			rotationMatrices.resize(allParts.size());
			scaleMatrices.resize(allParts.size());

			// Parse Root
			string headPart = rootValue.get("Root", "").asString();
			if (bodyParts.find(headPart) == bodyParts.end()) {
				throw exception("Root value not valid");
			}
			headNode = bodyParts[headPart];

			// Parse Edges
			Json::Value allEdges = rootValue.get("Edges", {});
			parseJsonArrayIntoAdjList(bodyParts, allEdges, adjList);

			// Parse Parameters
			Json::Value parameters = rootValue.get("Parameters", {});
			parseJsonValueIntoMatrices(bodyParts, initialTranslationMatrices, rotationMatrices, finalTranslationMatrices, scaleMatrices, parameters);
		}
		catch (exception e) {
			cerr << "Exception in parsing input file - " << e.what() << endl;
			cin.get();
			exit(-1);
		}
	}
	else {
		cerr << "Hierarchy file couldn't be parsed" << endl;
		cin.get();
		exit(-1);
	}
	f.close();
	return { headNode, bodyParts, adjList, initialTranslationMatrices, rotationMatrices, finalTranslationMatrices, scaleMatrices };
}