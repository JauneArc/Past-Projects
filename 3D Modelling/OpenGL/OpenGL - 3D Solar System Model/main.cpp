/*
By: Liam Conway
Last Worked On: 5/4/18 3:30pm
Program Puspose: To create a solar system from cubes using translation and matrixes
Not Working: Getting the fourth planet to fade (the commented out implementation modifies the alpha for all the cubes
*/

#include <cstdio>       // for C++ i/o
#include <iostream>
#include <string>		//for useing strings
#include <cstddef>
using namespace std;    // to avoid having to use std::

#define GLEW_STATIC     // include GLEW as a static library
#include <GLEW/glew.h>  // include GLEW
#include <GLFW/glfw3.h> // include GLFW (which includes the OpenGL header)
#include <glm/glm.hpp>  // include GLM (ideally should only use the GLM headers that are actually used)
#include <glm/gtx/transform.hpp>
using namespace glm;    // to avoid having to use glm::

#include "shader.h"
#include "Camera.h"

#define PI 3.14159265
#define MAX_SLICES 50
#define MIN_SLICES 8
#define MAX_VERTICES (MAX_SLICES+2)*3   // a triangle fan should have a minimum of 3 vertices
#define CIRCLE_RADIUS 3.0
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000
#define MOVEMENT_SENSITIVITY 0.05f		// camera movement sensitivity
#define ROTATION_SENSITIVITY 0.01f		// camera rotation sensitivity
#define ZOOM_SENSITIVITY 0.01f			// camera zoom sensitivity

// struct for vertex attributes
struct Vertex
{
	GLfloat position[3];
	GLfloat color[3];
};

// global variables

GLfloat g_vertices_circle[MAX_VERTICES] = {
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f
};

GLfloat g_colors_circle[MAX_VERTICES] = {
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f
};

GLuint g_slices = MAX_SLICES;   // number of circle slices

Vertex g_vertices[] = {
	// vertex 1
	-0.5f, 0.5f, 0.5f,  // position
	1.0f, 0.0f, 1.0f,   // colour
						// vertex 2
	-0.5f, -0.5f, 0.5f, // position
	1.0f, 0.0f, 0.0f,   // colour
						// vertex 3
	0.5f, 0.5f, 0.5f,   // position
	1.0f, 1.0f, 1.0f,   // colour
						// vertex 4
	0.5f, -0.5f, 0.5f,  // position
	1.0f, 1.0f, 0.0f,   // colour
						// vertex 5
	-0.5f, 0.5f, -0.5f, // position
	0.0f, 0.0f, 1.0f,   // colour
						// vertex 6
	-0.5f, -0.5f, -0.5f,// position
	0.0f, 0.0f, 0.0f,   // colour
						// vertex 7
	0.5f, 0.5f, -0.5f,  // position
	0.0f, 1.0f, 1.0f,   // colour
						// vertex 8
	0.5f, -0.5f, -0.5f, // position
	0.0f, 1.0f, 0.0f,   // colour
};

GLuint g_indices[] = {
	0, 1, 2,    // triangle 1
	2, 1, 3,    // triangle 2
	4, 5, 0,    // triangle 3
	0, 5, 1,    // ...
	2, 3, 6,
	6, 3, 7,
	4, 0, 6,
	6, 0, 2,
	1, 5, 3,
	3, 5, 7,
	5, 4, 7,
	7, 4, 6,    // triangle 12
};

GLuint g_IBO = 0;               // index buffer object identifier
GLuint g_VBO[3];                // vertex buffer object identifier
GLuint g_VAO[2];                // vertex array object identifier
GLuint g_shaderProgramID = 0;   // shader program identifier
GLuint g_MVP_Index = 0;         // location in shader
GLuint g_alphaIndex;            // for transparency of 4th planet
glm::mat4 g_modelMatrix[5];     // planets object model matrices
glm::mat4 g_modelMatrixCircle[5];// circle model matrices
glm::mat4 g_modelMatrixSubPlanets[5];// object matrices for sub-planets (moon, disc etc)
glm::mat4 g_viewMatrix;         // view matrix
glm::mat4 g_projectionMatrix;   // projection matrix

Camera g_camera;            // camera 
bool g_perspectiveTopDown = false; //used for focusing the camera between top-down and perspective views

float g_orbitSpeed[5] = { 0.3f, 0.5f, 0.4f, 0.2f, 0.1f };       // for speed of rotation around sun
float g_rotationSpeed[5] = { 0.07f, 0.7f, 3.0f, 5.0f, 1.0f };   // for speed of rotation on own axis
float g_scaleSize[5] = { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };        // for scaling the orbiting planets
float g_axisOfRotation[5] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, };  // for offsetting the axis of rotation
float g_alpha = 0.5f;       // transparency of 4th planet
bool g_enableAnimation = true;


void generate_rings()
{
	float angle = PI * 2 / static_cast<float>(g_slices);    // used to generate x and y coordinates
	float scale_factor = static_cast<float>(WINDOW_HEIGHT) / WINDOW_WIDTH;  // scale to make it a circle instead of an elipse
	int index = 0;  // vertex index

	g_vertices_circle[3] = CIRCLE_RADIUS * scale_factor;    // set x coordinate of vertex 1

															// generate vertex coordinates for triangle fan
	for (int i = 2; i < g_slices + 2; i++)
	{
		// multiply by 3 for a vertex (x,y,z)
		index = i * 3;

		g_vertices_circle[index] = CIRCLE_RADIUS * cos(angle) * scale_factor;
		g_vertices_circle[index + 1] = CIRCLE_RADIUS * sin(angle);
		g_vertices_circle[index + 2] = 0.0f;

		g_colors_circle[index] = 1.0f;
		g_colors_circle[index + 1] = 0.0f;
		g_colors_circle[index + 2] = 0.0f;

		// update 
		angle += PI * 2 / static_cast<float>(g_slices);
	}

	//get rid of line
	g_vertices_circle[0] = g_vertices_circle[3];
	g_vertices_circle[1] = g_vertices_circle[4];
	g_vertices_circle[2] = g_vertices_circle[5];
}

static void init(GLFWwindow* window)
{
	glClearColor(0.0, 0.0, 0.0, 1.0);   // set clear background colour

	glEnable(GL_DEPTH_TEST);    // enable depth buffer test
	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

	// create and compile our GLSL program from the shader files
	g_shaderProgramID = loadShaders("MVP_VS.vert", "ColorFS.frag");

	// find the location of shader variables
	GLuint positionIndex = glGetAttribLocation(g_shaderProgramID, "aPosition");
	GLuint colorIndex = glGetAttribLocation(g_shaderProgramID, "aColor");
	g_MVP_Index = glGetUniformLocation(g_shaderProgramID, "uModelViewProjectionMatrix");
	//g_alphaIndex = glGetUniformLocation(g_shaderProgramID, "uAlpha");

	// initialise model matrix to the identity matrix
	g_modelMatrix[0] = g_modelMatrix[1] = g_modelMatrix[2] = g_modelMatrix[3] = g_modelMatrix[4] = glm::mat4(1.0f);
	g_modelMatrixCircle[0] = g_modelMatrixCircle[1] = g_modelMatrixCircle[2] = g_modelMatrixCircle[3] = g_modelMatrixCircle[4] = glm::mat4(1.0f);
	g_modelMatrixSubPlanets[2] = g_modelMatrixSubPlanets[3] = glm::mat4(1.0f);;

	// set camera's view matrix
	g_camera.setViewMatrix(glm::vec3(0, 3, 14), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspectRatio = static_cast<float>(width) / height;

	// set camera's projection matrix
	g_camera.setProjection(45.0f, aspectRatio, 0.1f, 100.0f);

	// initialise projection matrix
	g_projectionMatrix = glm::perspective(45.0f, aspectRatio, 0.1f, 100.0f);

	// generate identifier for VBO and copy data to GPU
	glGenBuffers(1, &g_VBO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertices), g_vertices, GL_STATIC_DRAW);

	// generate identifier for IBO and copy data to GPU
	glGenBuffers(1, &g_IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_indices), g_indices, GL_STATIC_DRAW);

	// generate identifiers for VAO
	glGenVertexArrays(1, &g_VAO[0]);

	// create VAO and specify VBO data
	glBindVertexArray(g_VAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO);
	// interleaved attributes
	glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color)));

	glEnableVertexAttribArray(positionIndex);   // enable vertex attributes
	glEnableVertexAttribArray(colorIndex);

	// generate planetary paths
	generate_rings();

	// create VBO and buffer the data
	glGenBuffers(1, &g_VBO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * (g_slices + 2), g_vertices_circle, GL_STATIC_DRAW);

	glGenBuffers(1, &g_VBO[2]);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * (g_slices + 2), g_colors_circle, GL_STATIC_DRAW);

	// create VAO and specify VBO data
	glGenVertexArrays(1, &g_VAO[1]);
	glBindVertexArray(g_VAO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[1]);
	glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);  // specify the form of the data
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[2]);
	glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, 0, 0); // specify the form of the data

	glEnableVertexAttribArray(positionIndex);   // enable vertex attributes
	glEnableVertexAttribArray(colorIndex);
}

//Generates a random value between 0.1 and 0.9
double generateRandomFloat(float min, float max)
{
	return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
}

// function used to update the scene
static void update_scene(GLFWwindow * window)
{
	// static variables for rotation angles
	static float orbitAngle[5] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, };
	static float rotationAngle[5] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	float scaleFactor = 0.05;

	// variables to store forward/back and strafe movement
	float moveForward = 0;
	float strafeRight = 0;
	float zoom = 0;

	// update movement variables based on keyboard input
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		moveForward += 1 * MOVEMENT_SENSITIVITY;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		moveForward -= 1 * MOVEMENT_SENSITIVITY;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		strafeRight -= 1 * MOVEMENT_SENSITIVITY;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		strafeRight += 1 * MOVEMENT_SENSITIVITY;

	// update zoom based on keyboard input
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		zoom -= 1 * ZOOM_SENSITIVITY;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		zoom += 1 * ZOOM_SENSITIVITY;

	if (zoom > 0 || zoom < 0)
		g_camera.updateFOV(zoom);				// change camera FOV

	g_camera.update(moveForward, strafeRight);	// update camera
	

	orbitAngle[0] += g_orbitSpeed[0] * scaleFactor;
	orbitAngle[1] += g_orbitSpeed[1] * scaleFactor;
	orbitAngle[2] += g_orbitSpeed[2] * scaleFactor;
	orbitAngle[3] += g_orbitSpeed[3] * scaleFactor;
	orbitAngle[4] += g_orbitSpeed[4] * scaleFactor;

// update rotation angles
	rotationAngle[0] += g_rotationSpeed[0] * scaleFactor;
	rotationAngle[1] += g_rotationSpeed[1] * scaleFactor;
	rotationAngle[2] += g_rotationSpeed[2] * scaleFactor;
	rotationAngle[3] += g_rotationSpeed[3] * scaleFactor;
	rotationAngle[4] += g_rotationSpeed[4] * scaleFactor;

// update model matrix - planets
	g_modelMatrix[0] = glm::rotate(rotationAngle[0], glm::vec3(0.0f, 1.0f, 0.0f));

	g_modelMatrix[1] = glm::translate(glm::vec3(g_axisOfRotation[1], 0.0f, 0.0f))   //moves the axis of rotation along x-axis
		* glm::rotate(orbitAngle[1], glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::translate(glm::vec3(2.0f, 0.0f, 0.0f))
		* glm::rotate(rotationAngle[1], glm::vec3(0.0f, -1.0f, 0.0f))       
		* glm::rotate(glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f))     //rotates into a diamond shape
		* glm::rotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f))     //rotates into a diamond shape
		* glm::scale(glm::vec3(g_scaleSize[1], g_scaleSize[1], g_scaleSize[1]));

	g_modelMatrix[2] = glm::translate(glm::vec3(g_axisOfRotation[2], 0.0f, 0.0f))
		* glm::rotate(orbitAngle[2], glm::vec3(0.0f, -1.0f, 0.0f))
		* glm::translate(glm::vec3(4.0f, 0.0f, 0.0f))
		* glm::rotate(rotationAngle[2], glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::scale(glm::vec3(g_scaleSize[2], g_scaleSize[2], g_scaleSize[2]));

	g_modelMatrix[3] = glm::translate(glm::vec3(g_axisOfRotation[3], 0.0f, 0.0f))
		* glm::rotate(orbitAngle[3], glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::translate(glm::vec3(6.0f, 0.0f, 0.0f))
		* glm::rotate(rotationAngle[3], glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::scale(glm::vec3(g_scaleSize[3], g_scaleSize[3], g_scaleSize[3]));

	g_modelMatrix[4] = glm::translate(glm::vec3(g_axisOfRotation[4], 0.0f, 0.0f))
		* glm::rotate(orbitAngle[4], glm::vec3(0.0f, 1.0f, 0.0f))  
		* glm::translate(glm::vec3(8.0f, 0.0f, 0.0f))
		* glm::rotate(rotationAngle[4], glm::vec3(0.0f, -1.0f, 0.0f))
		* glm::scale(glm::vec3(g_scaleSize[4], g_scaleSize[4], g_scaleSize[4]));

// update model matrix - orbit paths
	g_modelMatrixCircle[1] = glm::translate(glm::vec3(g_axisOfRotation[1], 0.0f, 0.0f)) * glm::scale(glm::vec3(0.68f, 0.68f, 0.68f)) * glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	g_modelMatrixCircle[2] = glm::translate(glm::vec3(g_axisOfRotation[2], 0.0f, 0.0f)) * glm::scale(glm::vec3(1.35f, 1.35f, 1.35f)) * glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	g_modelMatrixCircle[3] = glm::translate(glm::vec3(g_axisOfRotation[3], 0.0f, 0.0f)) * glm::scale(glm::vec3(2.0f, 2.0f, 2.0f)) * glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	g_modelMatrixCircle[4] = glm::translate(glm::vec3(g_axisOfRotation[4], 0.0f, 0.0f)) * glm::scale(glm::vec3(2.7f, 2.7f, 2.7f)) * glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

// update model matrix - ring
	g_modelMatrixSubPlanets[2] = glm::translate(glm::vec3(g_axisOfRotation[1], 0.0f, 0.0f))
		* glm::scale(glm::vec3(0.35f, 0.35f, 0.35f))
		* glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
// update model matrix - moon
//try to make the moon rotate closer to the earth ??? which translate to change
	g_modelMatrixSubPlanets[3] = glm::translate(glm::vec3(g_axisOfRotation[3], 0.0f, 0.0f)) // i think it's this translate
		* glm::rotate(orbitAngle[3], glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::translate(glm::vec3(6.0f, 0.0f, 0.0f))//translate relative to planet 0
		* glm::rotate(rotationAngle[3], glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::scale(glm::vec3(g_scaleSize[3], g_scaleSize[3], g_scaleSize[3]));
}

// function used to render the scene
static void render_scene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear colour buffer and depth buffer

	glUseProgram(g_shaderProgramID);    // use the shaders associated with the shader program

	glm::mat4 MVP = glm::mat4(1.0f);    //ModelViewProjection matrix to be shared. Initialized to identity

	glBindVertexArray(g_VAO[1]);            // make VAO active
//Circle 1
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrixCircle[1];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawArrays(GL_LINE_LOOP, 0, g_slices + 2);    // display the vertices based on the primitive type

//Circle 2                                              
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrixCircle[2];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawArrays(GL_LINE_LOOP, 0, g_slices + 2);    // display the vertices based on the primitive type

//Circle 3                                              
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrixCircle[3];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawArrays(GL_LINE_LOOP, 0, g_slices + 2);    // display the vertices based on the primitive type

//Circle 4                                              
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrixCircle[4];;
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawArrays(GL_LINE_LOOP, 0, g_slices + 2);    // display the vertices based on the primitive type

// Ring Object2
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[2] * g_modelMatrixSubPlanets[2];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, g_slices + 2); // display the vertices based on the primitive type

	glBindVertexArray(g_VAO[0]);        // make VAO active

// Object 1
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[0];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);   // display the vertices based on their indices and primitive type

// Object 2
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[1];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);   // display the vertices based on their indices and primitive type

// Object 3
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[2];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);   // display the vertices based on their indices and primitive type

// Object 4
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[3];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	//glUniform1fv(g_alphaIndex, 1, &g_alpha);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);   // display the vertices based on their indices and primitive type


// Object 5
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[4];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);   // display the vertices based on their indices and primitive type

// Moon Object3
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrixSubPlanets[3] * g_modelMatrix[4];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);   // display the vertices based on their indices and primitive type

	glFlush();  // flush the pipeline
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	// variables to store mouse cursor coordinates
	static double previous_xpos = xpos;
	static double previous_ypos = ypos;
	double delta_x = xpos - previous_xpos;
	double delta_y = ypos - previous_ypos;

	// pass mouse movement to camera class
	g_camera.updateRotation(delta_x * ROTATION_SENSITIVITY, delta_y * ROTATION_SENSITIVITY);

	// update previous mouse coordinates
	previous_xpos = xpos;
	previous_ypos = ypos;
}

// key press or release callback function
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// quit if the ESCAPE key was press
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		// set flag to close the window
		glfwSetWindowShouldClose(window, GL_TRUE);
		return;
	}
	// toggle animation
	else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		static int count = 1;

		if (count % 2 == 0)
			g_enableAnimation = true;
		else
			g_enableAnimation = false;

		count++;
	}
	// render in perspective view
	else if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		cout << "Perspective-View" << endl << endl;
		// set camera's view matrix
		g_camera.setViewMatrix(glm::vec3(0, 3, 14), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		if (g_perspectiveTopDown)
		{
			g_camera.updateRotation(0, 33);
			g_perspectiveTopDown = false;
		}
		render_scene();
	}
	// render from top view
	else if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		cout << "Top-View" << endl << endl;
		// set camera's view matrix
		g_camera.setViewMatrix(glm::vec3(0, 15.0f, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, -1.0f));
		if (!g_perspectiveTopDown)
		{
			g_camera.updateRotation(0, -33);
			g_perspectiveTopDown = true;
		}
		render_scene();
	}
	// render from eye-level view
	/*
	else if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		cout << "Eye-level View" << endl << endl;
		// set camera's view matrix
		g_camera.setViewMatrix(glm::vec3(0, 0, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		if (g_perspectiveTopDown)
		{
			g_camera.updateRotation(0, 33);
			g_perspectiveTopDown = false;
		}
		render_scene();
	}
	*/
	// Randomize size, orbit speed, axis rotation speed of planets
	else if (key == GLFW_KEY_R && action == GLFW_PRESS) {

		// Randomize planet size
		g_scaleSize[1] = generateRandomFloat(0.1, 0.75);
		g_scaleSize[2] = generateRandomFloat(0.1, 0.75);
		g_scaleSize[3] = generateRandomFloat(0.1, 0.75);
		g_scaleSize[4] = generateRandomFloat(0.1, 0.75);

		// Randomize speed of rotation (on planets own axis)
		g_rotationSpeed[1] = generateRandomFloat(0.1, 2.0);
		g_rotationSpeed[2] = generateRandomFloat(0.1, 2.0);
		g_rotationSpeed[3] = generateRandomFloat(0.1, 2.0);
		g_rotationSpeed[4] = generateRandomFloat(0.1, 2.0);

		// Randomize speed of rotation around sun
		g_orbitSpeed[1] = generateRandomFloat(0.1, 0.7);
		g_orbitSpeed[2] = generateRandomFloat(0.1, 0.7);
		g_orbitSpeed[3] = generateRandomFloat(0.1, 0.7);
		g_orbitSpeed[4] = generateRandomFloat(0.1, 0.7);

		// Randomize offset for axis of rotation
		g_axisOfRotation[1] = generateRandomFloat(-0.5, 0.5);
		g_axisOfRotation[2] = generateRandomFloat(-0.5, 0.5);
		g_axisOfRotation[3] = generateRandomFloat(-0.5, 0.5);
		g_axisOfRotation[4] = generateRandomFloat(-0.5, 0.5);

		render_scene();
	}
}

// error callback function
static void error_callback(int error, const char* description)
{
	cerr << description << endl;    // output error description
}

int main(void)
{
	GLFWwindow* window = NULL;  // pointer to a GLFW window handle

	glfwSetErrorCallback(error_callback);   // set error callback function

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
	window = glfwCreateWindow(1500, 1000, "Assignment 2", NULL, NULL);

	// if failed to create window
	if (window == NULL)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); // set window context as the current context
	glfwSwapInterval(1);            // swap buffer interval

									// initialise GLEW
	if (glewInit() != GLEW_OK)
	{
		// if failed to initialise GLEW
		cerr << "GLEW initialisation failed" << endl;
		exit(EXIT_FAILURE);
	}

	// set key callback function
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);

	// use sticky mode to avoid missing state changes from polling
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// use mouse to move camera, hence use disable cursor mode
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// initialise rendering states
	init(window);

	// variables for simple time management
	float lastUpdateTime = glfwGetTime();
	float currentTime = lastUpdateTime;

	// the rendering loop
	while (!glfwWindowShouldClose(window))
	{
		currentTime = glfwGetTime();

// only update if more than 0.02 seconds since last update
		if (currentTime - lastUpdateTime > 0.02)
		{
			if (g_enableAnimation) { update_scene(window); }      // update the scene
			render_scene();     // render the scene

			glfwSwapBuffers(window);    // swap buffers
			glfwPollEvents();           // poll for events

			lastUpdateTime = currentTime;   // update last update time
		}
	}

	// clean up
	glDeleteProgram(g_shaderProgramID);
	glDeleteBuffers(1, &g_IBO);
	glDeleteBuffers(1, &g_VBO[0]);
	glDeleteBuffers(1, &g_VBO[1]);
	glDeleteVertexArrays(1, &g_VAO[0]);
	glDeleteVertexArrays(1, &g_VAO[1]);

	// close the window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}
