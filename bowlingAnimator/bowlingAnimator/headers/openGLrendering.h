#pragma once
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

extern GLFWwindow* window;
extern GLuint VertexArrayID;
extern GLuint programID;
extern GLuint MatrixID, ViewMatrixID, ModelMatrixID;
extern GLuint LightID1, LightID2;
extern GLuint TextureID;
extern glm::mat4 ProjectionMatrix, ViewMatrix, ModelMatrix, MVP;
extern glm::vec3 lightPos1, lightPos2;

int initializeGLFW();

int initializeGLEW();

void initializeOpenGL();

void setupBuffer(GLuint &buffer, GLsizeiptr size, const GLvoid * address);

void render(GLuint Texture, GLuint vertexBuffer, GLuint uvBuffer, GLuint normalBuffer, GLuint vertexCount);