#include "openGLrendering.h"
#include "common/shader.hpp"
#include "common/texture.hpp"
#include "common/objloader.hpp"
#include "common/controls.hpp"
#include "HierarchyInputParser.h"
#include "HierarchicalModel.h"
#include <iostream>

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

	lightPos1 = glm::vec3(0, 0, -4);
	lightPos2 = glm::vec3(2.5, 3, -2.5);

	do {

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		computeMatricesFromInputs();

		ProjectionMatrix = getProjectionMatrix();
		ViewMatrix = getViewMatrix();

		glUseProgram(programID);

		std::vector<glm::mat4> modelMatrices = computeModelMatrices(inputHierarchy);

		std::vector<glm::mat4>::iterator iter = modelMatrices.begin();

		while (iter != modelMatrices.end()) {
			ModelMatrix = (*iter);
			MVP = ProjectionMatrix * ViewMatrix * (ModelMatrix);
			render(TextureCube, vertexbufferCube, uvbufferCube, normalbufferCube, verticesCube.size());
			++iter;
		}

		glfwSwapBuffers(window);
		glfwPollEvents();

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