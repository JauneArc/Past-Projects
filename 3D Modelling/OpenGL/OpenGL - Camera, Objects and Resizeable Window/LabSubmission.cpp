#include <cstdio>		// for C++ i/o
#include <iostream>
#include <string>
#include <sstream>
using namespace std;	// to avoid having to use std::

#define GLEW_STATIC		// include GLEW as a static library
#include <GLEW/glew.h>	// include GLEW
#include <GLFW/glfw3.h>	// include GLFW (which includes the OpenGL header)
#include <glm/glm.hpp>	// include GLM (ideally should only use the GLM headers that are actually used)
using namespace glm;	// to avoid having to use glm::

static int x = 1;
static int toggle = 0;

#include "shader.h"


// global variables
GLfloat g_vertexPos[] = {
	+0.0f, +0.0f, +0.0f, //Traingle 1
	+1.0f, +1.0f, +0.0f,
	-1.0f, +1.0f, +0.0f,

	+0.0f, +0.0f, +0.0f, //Triangle 2
	-1.0f, -1.0f, +0.0f,
	+1.0f, -1.0f, +0.0f,

	-0.1f, +0.5f, +0.0f, //Points 3 draw in the middle of the top triangle
	-0.0f, +0.5f, +0.0f,
	+0.1f, +0.5f, +0.0f,

	-0.1f, -0.5f, +0.0f, //Rectangle Pt1
	+0.1f, -0.25f, +0.0f,
	-0.1f, -0.25f, +0.0f,

	-0.1f, -0.5f, +0.0f, //Rectangle Pt2
	+0.1f, -0.5f, +0.0f,
	+0.1f, -0.25f, +0.0f

	+0.0f, +0.2f, +0.0f, //Line 1
	+0.0f, +0.4f, +0.0f, 

	+0.0f, -0.2f, +0.0f, //Line 2
	+0.0f, -0.4f, +0.0f
};

GLfloat g_vertexColors[] = {
	1.0f, 0.64f, 0.0f,		// vertex 1 triangle 1
	1.0f, 0.64f, 0.0f,		// vertex 2 triangle 1
	1.0f, 0.64f, 0.0f,		// vertex 3 triangle 1

	1.0f, 0.64f, 0.0f,		// vertex 1 triangle 2
	1.0f, 0.64f, 0.0f,		// vertex 2 triangle 2
	1.0f, 0.64f, 0.0f,		// vertex 3 triangle 2

	0.5f, 0.0f, 0.0f,		// vertex 1 Point 1
	0.5f, 0.0f, 0.0f,		// vertex 2 Point 2
	0.5f, 0.0f, 0.0f,		// vertex 3 Point 3

	1.0f, 0.0f, 1.0f,		// vertex 1 Triangle Strip
	0.29f, 0.0f, 0.5f,		// vertex 2 Triangle Strip
	0.29f, 0.0f, 0.5f,		// vertex 3 Triangle Strip
	1.0f, 0.0f, 1.0f,		// vertex 4 Triangle Strip
	0.29f, 0.0f, 0.5f,		// vertex 5 Triangle Strip
	0.29f, 0.0f, 0.5f,		// vertex 6 Triangle Strip

	0.0f, 1.0f, 1.0f,		// vertex 1 Line 1
	0.0f, 1.0f, 1.0f,		// vertex 2 Line 1
	0.0f, 1.0f, 1.0f,		// vertex 1 Line 2
	0.0f, 1.0f, 1.0f,		// vertex 2 Line 2
	0.0f, 1.0f, 1.0f,		// vertex 2 Line 2

};

GLuint g_VBO[2];		// vertex buffer object identifiers
GLuint g_VAO = 0;		// vertex array object identifier
GLuint g_shaderProgramID = 0;	// shader program identifier

static void init()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);	// set clear background colour

										// create and compile our GLSL program from the shader files
	g_shaderProgramID = loadShaders("ColorVS.vert", "ColorFS.frag");

	glGenBuffers(2, g_VBO);	// generate unused VBO identifiers
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[0]);		// bind the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertexPos), g_vertexPos, GL_STATIC_DRAW); 		// copy data to buffer
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[1]);		// bind the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertexColors), g_vertexColors, GL_STATIC_DRAW); 	// copy data to buffer

	glGenVertexArrays(1, &g_VAO);	// generate unused VAO identifier
	glBindVertexArray(g_VAO);		// create VAO
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[0]);		// bind the VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// specify the form of the data
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[1]);		// bind the VBO
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);		// specify the form of the data

	glEnableVertexAttribArray(0);	// enable vertex attributes
	glEnableVertexAttribArray(1);

}

// function used to render the scene
static void render_scene()
{
	glClear(GL_COLOR_BUFFER_BIT);	// clear colour buffer

	glUseProgram(g_shaderProgramID);		// use the shaders associated with the shader program

	glBindVertexArray(g_VAO);		// bind the vertex array object

	glDrawArrays(GL_TRIANGLES, 0, 6);	// display the vertices based on the primitive type
	glPointSize(10);
	glDrawArrays(GL_POINTS, 6, 3); // display verticies based on primitive tpye
	glPointSize(1);
	glDrawArrays(GL_TRIANGLE_STRIP, 9, 6);
	glLineWidth(5);
	glDrawArrays(GL_LINES, 15, 2);
	glDrawArrays(GL_LINES, 17, 2);
	glLineWidth(1);

	glFlush();	// flush the pipeline
}

// key press or release callback function
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// quit if the ESCAPE key was press
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) 
	{
		// set flag to close the window
		glfwSetWindowShouldClose(window, GL_TRUE);
		return;
	}
	// toggle wireframe mode
	if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		if (toggle == 0)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			toggle++;
			return;
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			toggle--;
			return;
		}
	}

	// change the background colour if b is pressed (cycles black, white, red, green, blue
	if (key == GLFW_KEY_B && action == GLFW_PRESS)
	{
		if (x == 0)
		{
			//set background black
			// set flag to change the background colour
			glClearColor(0.0, 0.0, 0.0, 1.0);
			x++;
			return;
		}
		if (x == 1)
		{
			//set background white
			// set flag to change the background colour
			glClearColor(1.0, 1.0, 1.0, 0.0);
			x++;
			return;
		}
		if (x == 2)
		{
			//set background red 
			// set flag to change the background colour
			glClearColor(1.0, 0.0, 0.0, 1.0);
			x++;
			return;
		}
		if (x == 3)
		{
			//set background green
			// set flag to change the background colour
			glClearColor(0.0, 1.0, 0.0, 1.0);
			x++;
			return;
		}
		if (x == 4)
		{
			//set background blue
			// set flag to change the background colour
			glClearColor(0.0, 0.0, 1.0, 1.0);
			x = 0;
			return;
		}
	}
}

// mouse press or release callback function
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// quit if the right mouse button was pressed
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		// set flag to close the window
		glfwSetWindowShouldClose(window, GL_TRUE);
		return;
	}

	// print mouse coords to console if the left mouse button was pressed
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		printf("Position %f, %f \n", xpos, ypos);
		return;
	}

}

//change the window title when the widow is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	//get the frame buffer size
	int width1, height1;
	glfwGetFramebufferSize(window, &width1, &height1);
	//create a stringstream to store the data in
	std::stringstream s;
	s << "Lab Submission - " << width1 << "x" << height1;
	//window = glfwCreateWindow(width1, height1, s.str().c_str(), NULL, NULL);
	//char* NewWindowName = "Lab Submission - %i %i", width, height;
	//update the window title
	glfwSetWindowTitle(window, s.str().c_str());
	return;
}

// error callback function
static void error_callback(int error, const char* description)
{
    cerr << description << endl;	// output error description
}

int main(void)
{
	GLFWwindow* window = NULL;	// pointer to a GLFW window handle

	glfwSetErrorCallback(error_callback);	// set error callback function

											// initialise GLFW
	if (!glfwInit())
	{
		// if failed to initialise GLFW
		exit(EXIT_FAILURE);
	}

	// minimum OpenGL version 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// create a window and its OpenGL context
	window = glfwCreateWindow(1024, 768, "Lab Submission - 1024x768", NULL, NULL);

	// if failed to create window
	if (window == NULL)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);	// set window context as the current context
	glfwSwapInterval(1);			// swap buffer interval

									// initialise GLEW
	if (glewInit() != GLEW_OK)
	{
		// if failed to initialise GLEW
		cerr << "GLEW initialisation failed" << endl;
		exit(EXIT_FAILURE);
	}

	// set callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// initialise rendering states
	init();

	// the rendering loop
	while (!glfwWindowShouldClose(window))
	{
		render_scene();				// render the scene

		glfwSwapBuffers(window);	// swap buffers
		glfwPollEvents();			// poll for events
	}

	// clean up
	glDeleteProgram(g_shaderProgramID);
	glDeleteBuffers(2, g_VBO);
	glDeleteVertexArrays(1, &g_VAO);

	// close the window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}

