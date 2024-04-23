#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <iostream>

#include "Camera Class.h"
#include "vertices.h"
#include "Shader.h"
#include "Compute Shader.h"
#include "Model Loader.hpp"
#include "Render Engine.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void renderQuad();

const int SCRN_WIDTH = 1000, SCRN_HEIGHT = 1024;
bool firstMouse = true;
float lastX = 0, lastY = 0;
CameraClass camera(glm::vec3(0, 0, 3));

float deltaTime, lastFrame = 0, currentFrame;


int main()
{
	// Setup
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// Time before next frame
	glfwSwapInterval(0);

	// Create window object
	GLFWwindow* window = glfwCreateWindow(SCRN_WIDTH, SCRN_HEIGHT, "Ray Tracer", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Falid to create glfw object\n";
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	// Loads Glad.c
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to create glfw window";
		return -1;
	}

	int max_compute_work_group_count[3];
	int max_compute_work_group_size[3];
	int max_compute_work_group_invocations;

	for (int idx = 0; idx < 3; idx++) {
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, idx, &max_compute_work_group_count[idx]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, idx, &max_compute_work_group_size[idx]);
	}
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max_compute_work_group_invocations);

	std::cout << "OpenGL Limitations: " << std::endl;
	std::cout << "maximum number of work groups in X dimension " << max_compute_work_group_count[0] << std::endl;
	std::cout << "maximum number of work groups in Y dimension " << max_compute_work_group_count[1] << std::endl;
	std::cout << "maximum number of work groups in Z dimension " << max_compute_work_group_count[2] << std::endl;

	std::cout << "maximum size of a work group in X dimension " << max_compute_work_group_size[0] << std::endl;
	std::cout << "maximum size of a work group in Y dimension " << max_compute_work_group_size[1] << std::endl;
	std::cout << "maximum size of a work group in Z dimension " << max_compute_work_group_size[2] << std::endl;

	std::cout << "Number of invocations in a single local work group that may be dispatched to a compute shader " << max_compute_work_group_invocations << std::endl;


	ComputeShader computeShader("compute.GLSL");
	Shader shaders("vertex.GLSL", "fragment.GLSL");


	Engine engine;

	while (!glfwWindowShouldClose(window))
	{
		currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		camera.updatePosition(window, deltaTime);

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		//glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
		//glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 40 * lights.size(), &lights[0]);

		// Set background colour
		//glClearColor(0.5, 0.5, 0.5, 1.0);

		computeShader.use();

		// fov
		computeShader.setFloat("fov", camera.fov);
		// camera pos
		computeShader.setVec3("camera.position", camera.position);
		// cameea target
		computeShader.setVec3("camera.target", camera.target);
		// camera up
		computeShader.setVec3("camera.up", camera.up);
		// camera right
		computeShader.setVec3("camera.right", camera.right);

		computeShader.setInt("sphereCount", engine.spheres.size());
		computeShader.setInt("lightCount", engine.lights.size());
		computeShader.setInt("triangleCount", engine.triangles.size());
		
		computeShader.setVec3("backgroundColor", glm::vec3(0));
		computeShader.setFloat("bVolume", engine.boundingVolume);

		// 32 invocations per work group for Nvidia
		glDispatchCompute((int)engine.TEXTURE_WIDTH/8, (int)engine.TEXTURE_HEIGHT/4, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaders.use();

		renderQuad();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteTextures(1, &engine.texture);

	glfwTerminate();
	return 0;
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.updateTarget(xoffset, yoffset);
}