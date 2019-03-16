#include "openGLrendering.h"
#include "common/shader.hpp"
#include "common/texture.hpp"
#include "common/objloader.hpp"
#include "common/controls.hpp"
#include "HierarchyInputParser.h"
#include "HierarchicalModel.h"
#include <iostream>
#include <math.h>

// Define global variables which can be used for rendering across transactions
GLuint VertexArrayID;
GLuint programID;
GLuint MatrixID, ViewMatrixID, ModelMatrixID;
GLuint LightID1, LightID2;
GLuint TextureID;
glm::mat4 ProjectionMatrix, ViewMatrix, ModelMatrix, MVP;
glm::vec3 lightPos1, lightPos2;

int main(void)
{
	Hierarchy inputHierarchy = parseJsonHierarchy("inputFiles/opengl/HierarchyInput.json");

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
	GLuint TextureCube = loadBMP_custom("inputFiles/Opengl/texture_red.bmp");
	GLuint TextureOther = loadBMP_custom("inputFiles/Opengl/skin_texture.bmp");

	// Get a handle for our "myTextureSampler" uniform
	TextureID = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	std::vector<glm::vec3> verticesCube;
	std::vector<glm::vec2> uvsCube;
	std::vector<glm::vec3> normalsCube;

	bool res = loadOBJ("inputFiles/Opengl/cube.obj", verticesCube, uvsCube, normalsCube);

	if (!res) exit(-1);

	GLuint vertexbufferCube, uvbufferCube, normalbufferCube;
	setupBuffer(vertexbufferCube, (verticesCube.size() * sizeof(glm::vec3)), (&verticesCube[0]));
	setupBuffer(uvbufferCube, (uvsCube.size() * sizeof(glm::vec2)), (&uvsCube[0]));
	setupBuffer(normalbufferCube, (normalsCube.size() * sizeof(glm::vec3)), (&normalsCube[0]));

	lightPos1 = glm::vec3(0, 0, +4);
	lightPos2 = glm::vec3(2.5, 3, -2.5);

	int time = 0;
	Hierarchy currentHierarchy = inputHierarchy;
	float rightArmThetaX = -1.57f;
	float torsoThetaX = 0;
	float hipTranslateY = 0;
	float upperLegThetaX = 0;
	float lowerLegThetaY = 0;

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

		tempRotationRightUpperArm = glm::rotate(tempRotationRightUpperArm, rightArmThetaX, { 1,0,0 });
		tempRotationTorso = glm::rotate(tempRotationTorso, torsoThetaX, { 1,0,0 });
		tempRotationLowerLeg = glm::rotate(tempRotationLowerLeg, lowerLegThetaY, { 0, 1, 0});
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

		std::vector<glm::mat4>::iterator iterModel = modelMatrices.begin();
		std::vector<glm::mat4>::iterator iterScale = inputHierarchy.scaleMatrices.begin();

		while (iterModel != modelMatrices.end()) {
			ModelMatrix = (*iterModel) * (*iterScale);
			MVP = ProjectionMatrix * ViewMatrix * (ModelMatrix);
			render(TextureOther, vertexbufferCube, uvbufferCube, normalbufferCube, verticesCube.size());
			++iterModel;
			++iterScale;
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
		if (time <= 470) {
			rightArmThetaX += 0.005;
			torsoThetaX -= 0.0005;
			hipTranslateY -= 0.001;
			upperLegThetaX = acos(1 - fabs(hipTranslateY) / 2.0);
			lowerLegThetaY = -(2 * upperLegThetaX);
		}
		if (time <= 1000000) time++;
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbufferCube);
	glDeleteBuffers(1, &uvbufferCube);
	glDeleteBuffers(1, &normalbufferCube);
	glDeleteProgram(programID);
	glDeleteTextures(1, &TextureCube);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}