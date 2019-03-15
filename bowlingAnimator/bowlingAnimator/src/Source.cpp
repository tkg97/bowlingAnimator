#include "openGLrendering.h"
#include "common/shader.hpp"
#include "common/texture.hpp"
#include "common/objloader.hpp"
#include "common/controls.hpp"
#include "HierarchyInputParser.h"
#include <iostream>
#include <stack>

int main(void)
{
	Hierarchy h = parseJsonHierarchy("inputFiles/opengl/heirarchyInput.json");

	if (initializeGLFW() < 0) exit(-1);

	if (initializeGLEW() < 0) exit(-1);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	resetControls();

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("shaders/VertexShading.shader", "shaders/FragmentShading.shader");
	GLuint programIDline = LoadShaders("shaders/LineVertexShader.shader", "shaders/LineFragmentShader.shader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint MatrixIDline = glGetUniformLocation(programIDline, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Get a handle for our "LightPosition" uniform
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	GLuint LightID1 = glGetUniformLocation(programID, "LightPosition_worldspace1");
	GLuint colorID = glGetUniformLocation(programIDline, "in_color");

	// Load the texture
	GLuint TextureCube = loadBMP_custom("inputFiles/Opengl/texture_red.bmp");

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	std::vector<glm::vec3> verticesCube;
	std::vector<glm::vec2> uvsCube;
	std::vector<glm::vec3> normalsCube;
	bool res = loadOBJ("inputFiles/Opengl/cube.obj", verticesCube, uvsCube, normalsCube);

	if (!res) exit(-1);

	GLuint vertexbufferCube;
	setupBuffer(vertexbufferCube, (verticesCube.size() * sizeof(glm::vec3)), (&verticesCube[0]));

	GLuint uvbufferCube;
	setupBuffer(uvbufferCube, (uvsCube.size() * sizeof(glm::vec2)), (&uvsCube[0]));

	GLuint normalbufferCube;
	setupBuffer(normalbufferCube, (normalsCube.size() * sizeof(glm::vec3)), (&normalsCube[0]));

	glm::vec3 lightPos = glm::vec3(0, 0, -4);
	glm::vec3 lightPos2 = glm::vec3(2.5, 3, -2.5);

	std::stack<glm::mat4> matrixStack;



	do {

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		computeMatricesFromInputs();

		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();

		glm::mat4 ModelMatrixCube = glm::mat4(1.0);
		ModelMatrixCube = glm::translate(ModelMatrixCube, glm::vec3(0.0f, 0.0f, 3.5f));
		ModelMatrixCube = glm::rotate(ModelMatrixCube, glm::radians(-90.0f), { 1,0,0 });
		//ModelMatrixCube = glm::scale(ModelMatrixCube, vec3(0.5f, 0.5f, 0.5f));
		glm::mat4 MVPCube = ProjectionMatrix * ViewMatrix * ModelMatrixCube;

		matrixStack.push(MVPCube);

		//render the first arm
		glUseProgram(programID);

		// render cube

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVPCube[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrixCube[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(LightID1, lightPos2.x, lightPos2.y, lightPos2.z);
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureCube);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glBindBuffer(GL_ARRAY_BUFFER, vertexbufferCube);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 2nd attribute buffer : UVs
		glBindBuffer(GL_ARRAY_BUFFER, uvbufferCube);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 3rd attribute buffer : normals
		glBindBuffer(GL_ARRAY_BUFFER, normalbufferCube);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Draw the triangles !
		glDrawArrays(GL_TRIANGLES, 0, verticesCube.size());

		ModelMatrixCube = glm::translate(ModelMatrixCube, glm::vec3(0.0f, 0.0f, 3.5f));
		ModelMatrixCube = glm::rotate(ModelMatrixCube, glm::radians(-90.0f), { 1,0,0 });

		MVPCube = ProjectionMatrix * ViewMatrix * ModelMatrixCube;

		matrixStack.push(MVPCube);

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVPCube[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrixCube[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(LightID1, lightPos2.x, lightPos2.y, lightPos2.z);
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureCube);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glBindBuffer(GL_ARRAY_BUFFER, vertexbufferCube);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 2nd attribute buffer : UVs
		glBindBuffer(GL_ARRAY_BUFFER, uvbufferCube);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 3rd attribute buffer : normals
		glBindBuffer(GL_ARRAY_BUFFER, normalbufferCube);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Draw the triangles !
		glDrawArrays(GL_TRIANGLES, 0, verticesCube.size());


		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Swap buffers
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