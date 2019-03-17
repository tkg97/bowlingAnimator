#include "openGLrendering.h"
#include "common/shader.hpp"
#include "common/texture.hpp"
#include "common/objloader.hpp"
#include "common/controls.hpp"
#include "inputParser.h"
#include "HierarchicalModel.h"
#include "bezier.h"
#include <iostream>
#include <algorithm>
#include <math.h>

// Define global variables which can be used for rendering across transactions
GLuint VertexArrayID;
GLuint programID;
GLuint MatrixID, ViewMatrixID, ModelMatrixID;
GLuint LightID1, LightID2;
GLuint TextureID;
glm::mat4 ProjectionMatrix, ViewMatrix, ModelMatrix, MVP;
glm::vec3 lightPos1, lightPos2;

static bool checkIfInGutter(glm::vec3 location) {
	if ((location[0] >= -0.9 && location[0] < -0.1) || (location[0] > 2.1 && location[0] <= 2.9)) {
		return true;
	}
	return false;
}

int main(void)
{
	Hierarchy inputHierarchy = parseJsonHierarchy("inputFiles/opengl/HierarchyInput.json");
	std::vector< std::vector< float > > bezierPoints = parseBezierPoints("inputFiles/opengl/pointsInput.json");
	std::vector< std::vector< float > > airBezierPoints;

	initializeOpenGL();
	resetControls();


	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("shaders/VertexShading.shader", "shaders/FragmentShading.shader");

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ModelMatrixID = glGetUniformLocation(programID, "M");

	// Get a handle for our "LightPosition" uniform
	LightID1 = glGetUniformLocation(programID, "LightPosition_worldspace");
	LightID2 = glGetUniformLocation(programID, "LightPosition_worldspace1");

	// Load the texture
	GLuint TextureCube = loadBMP_custom("inputFiles/Opengl/texture_skin.bmp");
	GLuint TextureBall = loadBMP_custom("inputFiles/Opengl/texture_ball.bmp");
	GLuint TextureFloor = loadBMP_custom("inputFiles/Opengl/texture_floor.bmp");

	// Get a handle for our "myTextureSampler" uniform
	TextureID = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	std::vector<glm::vec3> verticesCube, verticesBall, verticesFloor;
	std::vector<glm::vec2> uvsCube, uvsBall, uvsFloor;
	std::vector<glm::vec3> normalsCube, normalsBall, normalsFloor;

	bool res = loadOBJ("inputFiles/Opengl/cube.obj", verticesCube, uvsCube, normalsCube);
	if (!res) exit(-1);

	res = loadOBJ("inputFiles/Opengl/bowlingball.obj", verticesBall, uvsBall, normalsBall);
	if (!res) exit(-1);

	res = loadOBJ("inputFiles/Opengl/bowlingring.obj", verticesFloor, uvsFloor, normalsFloor);
	if (!res) exit(-1);

	GLuint vertexbufferCube, uvbufferCube, normalbufferCube;
	setupBuffer(vertexbufferCube, (verticesCube.size() * sizeof(glm::vec3)), (&verticesCube[0]));
	setupBuffer(uvbufferCube, (uvsCube.size() * sizeof(glm::vec2)), (&uvsCube[0]));
	setupBuffer(normalbufferCube, (normalsCube.size() * sizeof(glm::vec3)), (&normalsCube[0]));

	GLuint vertexbufferBall, uvbufferBall, normalbufferBall;
	setupBuffer(vertexbufferBall, (verticesBall.size() * sizeof(glm::vec3)), (&verticesBall[0]));
	setupBuffer(uvbufferBall, (uvsBall.size() * sizeof(glm::vec2)), (&uvsBall[0]));
	setupBuffer(normalbufferBall, (normalsBall.size() * sizeof(glm::vec3)), (&normalsBall[0]));

	GLuint vertexbufferFloor, uvbufferFloor, normalbufferFloor;
	setupBuffer(vertexbufferFloor, (verticesFloor.size() * sizeof(glm::vec3)), (&verticesFloor[0]));
	setupBuffer(uvbufferFloor, (uvsFloor.size() * sizeof(glm::vec2)), (&uvsFloor[0]));
	setupBuffer(normalbufferFloor, (normalsFloor.size() * sizeof(glm::vec3)), (&normalsFloor[0]));

	lightPos1 = glm::vec3(4, 3, 1);
	lightPos2 = glm::vec3(2.5, 3, -10);

	int time = 0;
	Hierarchy currentHierarchy = inputHierarchy;
	float rightUpperArmThetaX = -1.57f;
	float torsoThetaX = 0;
	float hipTranslateY = 0;
	float upperLegThetaX = 0;
	float lowerLegThetaY = 0;

	glm::mat4 modelMatrixFloor;
	modelMatrixFloor = glm::translate(modelMatrixFloor, { 1,-2.1,-3 });
	modelMatrixFloor = glm::rotate(modelMatrixFloor, 1.57f, { 1,0,0 });

	glm::mat4 modelMatrixBallGutter;
	int gutterTime;
	float gutterZ;
	bool inGutter = false;

	float ballRate = 2;
	float bodyRate = 2;

	int t1 = 470 / bodyRate;
	int t2 = t1 + 130 / ballRate;
	int t3;

	do {

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		computeMatricesFromInputs();

		ProjectionMatrix = getProjectionMatrix();
		ViewMatrix = getViewMatrix();

		glUseProgram(programID);

		int nodeRightUpperArm = currentHierarchy.bodyParts["rightUpperArm"];
		int nodeTorso = currentHierarchy.bodyParts["body"];
		int nodeHip = currentHierarchy.bodyParts["hip"];
		int nodeRightLowerLeg = currentHierarchy.bodyParts["rightLowerLeg"];
		int nodeRightUpperLeg = currentHierarchy.bodyParts["rightUpperLeg"];
		int nodeLeftLowerLeg = currentHierarchy.bodyParts["leftLowerLeg"];
		int nodeLeftUpperLeg = currentHierarchy.bodyParts["leftUpperLeg"];

		glm::mat4 tempRotationRightUpperArm, tempRotationTorso, tempRotationUpperLeg, tempRotationLowerLeg, tempTranslationHip;

		//tempRotationRightUpperArm = glm::rotate(tempRotationRightUpperArm, rightUpperArmThetaY, { 0,1,0 });
		tempRotationRightUpperArm = glm::rotate(tempRotationRightUpperArm, rightUpperArmThetaX, { 1,0,0 });
		tempRotationTorso = glm::rotate(tempRotationTorso, torsoThetaX, { 1,0,0 });
		tempRotationLowerLeg = glm::rotate(tempRotationLowerLeg, lowerLegThetaY, { 0, 1, 0 });
		tempRotationUpperLeg = glm::rotate(tempRotationUpperLeg, upperLegThetaX, { 1,0,0 });
		tempTranslationHip = glm::translate(tempTranslationHip, { 0,hipTranslateY,0 });

		currentHierarchy.rotationMatrices[nodeRightUpperArm] = tempRotationRightUpperArm * inputHierarchy.rotationMatrices[nodeRightUpperArm];
		currentHierarchy.rotationMatrices[nodeTorso] = tempRotationTorso * inputHierarchy.rotationMatrices[nodeTorso];
		currentHierarchy.rotationMatrices[nodeLeftUpperLeg] = tempRotationUpperLeg * inputHierarchy.rotationMatrices[nodeLeftUpperLeg];
		currentHierarchy.rotationMatrices[nodeRightUpperLeg] = tempRotationUpperLeg * inputHierarchy.rotationMatrices[nodeRightUpperLeg];
		currentHierarchy.rotationMatrices[nodeLeftLowerLeg] = tempRotationLowerLeg * inputHierarchy.rotationMatrices[nodeLeftLowerLeg];
		currentHierarchy.rotationMatrices[nodeRightLowerLeg] = tempRotationLowerLeg * inputHierarchy.rotationMatrices[nodeRightLowerLeg];
		currentHierarchy.finalTranslationMatrices[nodeHip] = tempTranslationHip * inputHierarchy.finalTranslationMatrices[nodeHip];

		std::vector<glm::mat4> modelMatrices = computeModelMatrices(currentHierarchy);

		std::unordered_map<std::string, int>::iterator iter = currentHierarchy.bodyParts.begin();

		while (iter != currentHierarchy.bodyParts.end()) {
			ModelMatrix = modelMatrices[iter->second] * currentHierarchy.scaleMatrices[iter->second];
			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
			if (iter->first == "ball") {
				if (time == t1) {
					glm::vec4 origin(0, 0, 0, 1);
					glm::vec4 currentBallPosition = ModelMatrix * origin;
					std::vector<float> point;
					for (int i = 0;i < 3;i++) {
						point.push_back(currentBallPosition[i]);
					}
					airBezierPoints.push_back(point);
					airBezierPoints.push_back(bezierPoints[0]);
				}
				if (time > t1 && time <= t2) {
					glm::vec3 currentLocation = bezierLocation(airBezierPoints, std::min((time - t1) / 130.0f * ballRate, 1.0f));
					ModelMatrix = glm::translate(glm::mat4(1.0), currentLocation);
					ModelMatrix = glm::rotate(ModelMatrix, -std::min((time - t1) / 130.0f * ballRate, 1.0f) * 10, { 1,0,0 }) * currentHierarchy.scaleMatrices[iter->second];
					MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
				}
				if (time > t2) {
					if (inGutter) {
						if (-(time - gutterTime)*0.005 * ballRate + gutterZ <= -13) {
							ModelMatrix = glm::translate(modelMatrixBallGutter, { 0,0, (-13-gutterZ) }) * currentHierarchy.scaleMatrices[iter->second];
							MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
						}
						else {
							ModelMatrix = glm::translate(modelMatrixBallGutter, { 0,0, -(time - gutterTime)*0.005 * ballRate });
							ModelMatrix = glm::rotate(ModelMatrix, -(time - gutterTime)*0.005f * ballRate * 10, { 1,0,0 }) * currentHierarchy.scaleMatrices[iter->second];
							MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
						}
					}
					else {
						glm::vec3 currentLocation = bezierLocation(bezierPoints, std::min((time - t2) / 300.0f * ballRate, 1.0f));
						ModelMatrix = glm::translate(glm::mat4(1.0), currentLocation);
						ModelMatrix = glm::rotate(ModelMatrix, -std::min((time - t2) / 300.0f, 1.0f) * ballRate * 10, { 1,0,0 }) * currentHierarchy.scaleMatrices[iter->second];
						MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
						inGutter = checkIfInGutter(currentLocation);
						if (inGutter) {
							float x = (currentLocation[0] < 0) ? -0.5 : 2.5;
							glm::vec3 translation(x, -2.1, currentLocation[2]);
							modelMatrixBallGutter = glm::translate(glm::mat4(1.0), translation);
							gutterTime = time;
							gutterZ = currentLocation[2];
							t3 = t2 + abs(-13 - gutterZ) / (ballRate * 0.005f);
						}
					}
				}
				render(TextureBall, vertexbufferBall, uvbufferBall, normalbufferBall, verticesBall.size());
			}
			else render(TextureCube, vertexbufferCube, uvbufferCube, normalbufferCube, verticesCube.size());
			++iter++;
		}

		ModelMatrix = modelMatrixFloor;
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		render(TextureFloor, vertexbufferFloor, uvbufferFloor, normalbufferFloor, verticesFloor.size());

		glfwSwapBuffers(window);
		glfwPollEvents();
		if (time <= t1) {
			rightUpperArmThetaX += 0.004*bodyRate;
			torsoThetaX -= 0.0007 * bodyRate;
			hipTranslateY -= 0.0018 * bodyRate;
			upperLegThetaX = acos(1 - fabs(hipTranslateY) / 2.0);
			lowerLegThetaY = -(2 * upperLegThetaX);
		}
		else if (time <= t2) {
			rightUpperArmThetaX += 0.004 * bodyRate;
		}
		if (time <= 1000000) time++;
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbufferCube);
	glDeleteBuffers(1, &uvbufferCube);
	glDeleteBuffers(1, &normalbufferCube);
	glDeleteBuffers(1, &vertexbufferBall);
	glDeleteBuffers(1, &uvbufferBall);
	glDeleteBuffers(1, &normalbufferBall);
	glDeleteBuffers(1, &vertexbufferFloor);
	glDeleteBuffers(1, &uvbufferFloor);
	glDeleteBuffers(1, &normalbufferFloor);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}