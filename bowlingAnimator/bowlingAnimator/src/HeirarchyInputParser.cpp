#pragma once
#include "HeirarchyInputParser.h"
#include <glm/gtc/type_ptr.hpp>
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

void parseJsonValueIntoMatrices(const unordered_map<string, int>& bodyParts, vector<glm::mat4>& modelMatrices, const Json::Value& parameters) noexcept(false) {
	unordered_map<string, int>::const_iterator iter = bodyParts.begin();
	while (iter != bodyParts.end()) {
		Json::Value matrix = parameters.get(iter->first, {});
		modelMatrices[iter->second] = parseJsonArrayIntoMatrix(matrix);
		++iter;
	}
}

Heirarchy parseJsonHeirarchy(string path) {
	ifstream f(path);

	int headNode;
	unordered_map<string, int> bodyParts;
	vector< vector<int> > adjList;
	vector< glm::mat4 > modelMatrices;

	Json::Reader reader;
	Json::Value rootValue;

	if (reader.parse(f, rootValue)) {
		try {
			// Parse Parts
			Json::Value allParts = rootValue.get("Parts", {});
			parseJsonArrayIntoHashMap(allParts, bodyParts);
			adjList.resize(allParts.size());
			modelMatrices.resize(allParts.size());

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
			parseJsonValueIntoMatrices(bodyParts, modelMatrices, parameters);
		}
		catch (exception e) {
			cerr << "Exception in parsing input file - " << e.what() << endl;
			cin.get();
			exit(-1);
		}
	}
	else {
		cerr << "Heirarchy file couldn't be parsed" << endl;
		cin.get();
		exit(-1);
	}
	f.close();
	return { headNode, bodyParts, adjList, modelMatrices };
}