/*
Name: Liam Conway
Date: 4/5/2018
Description: The folloing is an OpenGL program whic can read in models from a file and render them to the screen.
	In default specification it renders and inverted cube as the room, 3 normal cubes as table and chairs and 2 ornaments on the table.
	There are also 2 markers which represent the lights position in the room; they change color in respect to the lighting source they represent
		(e.g. red light = red marker).

	It includes:
		A controllable camera: WASD(movement), Up/Down(Zoom)
		2 Display modes: Single window viewport and multiview a 4 viewport mode. Toggle V.
		Disco Mode: Enable via the tweak bar
		Fully Adjustable Lighting Attributes: Control via the tweak bar
		Wireframe Mode: Control via tweak bar
N.B
requires GLEW, GLM, GLFW, AntTweakBar, Assimp(Open Asset Import Library)
assumes that the models are in a folder called models in the project folder e.g.Assignment2/Assignment2/models
requires (assimp-vc140-mt.dll) either linked or in the project folder Assignment2/Assignment2/assimp-vc140-mt.dll
requires (AntTweakBar.dll) either linked or in the project folder Assignment2/Assignment2/AntTweakBar.dll

*/
#include <cstdio>		// for C++ i/o
#include <iostream>
#include <string>
#include <cstddef>
using namespace std;	// to avoid having to use std::

#define GLEW_STATIC		// include GLEW as a static library
#include <GLEW/glew.h>	// include GLEW
#include <GLFW/glfw3.h>	// include GLFW (which includes the OpenGL header)
#include <glm/glm.hpp>	// include GLM (ideally should only use the GLM headers that are actually used)
#include <glm/gtx/transform.hpp>
using namespace glm;	// to avoid having to use glm::

#include <AntTweakBar.h>
#include "assimp/cimport.h"//Open Asset Import Library (Assimp) to load and display simple .obj files.
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "shader.h"
#include "Camera.h"

#define MOVEMENT_SENSITIVITY 0.05f		// camera movement sensitivity
#define ROTATION_SENSITIVITY 0.01f		// camera rotation sensitivity
#define ZOOM_SENSITIVITY 0.01f			// camera zoom sensitivity
#define MAX_LIGHTS 2

// struct for vertex attributes
typedef struct Vertex
{
	GLfloat position[3];
	GLfloat normal[3];
} Vertex;

// struct for mesh properties
typedef struct Mesh
{
	Vertex* pMeshVertices;		// pointer to mesh vertices
	GLint numberOfVertices;		// number of vertices in the mesh
	GLint* pMeshIndices;		// pointer to mesh indices
	GLint numberOfFaces;		// number of faces in the mesh
} Mesh;

// light and material structs
typedef struct Light
{
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 attenuation;
	int type;
} Light;

typedef struct Material
{
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float shininess;
} Material;

// Global variables
Mesh g_mesh;					// mesh
Mesh cube_mesh;					//loads a cube
Mesh Furniture_mesh;			//loads the mesh for the furniture
Mesh Ornament1_mesh;			//
Mesh Ornament2_mesh;			//

GLuint g_IBO[5];				// index buffer object identifier
GLuint g_VBO[5];				// vertex buffer object identifier
GLuint g_VAO[5];				// vertex array object identifier
GLuint g_shaderProgramID = 0;	// shader program identifier

// locations in shader
GLuint g_MVP_Index = 0;
GLuint g_MV_Index = 0;
GLuint g_V_Index = 0;

GLuint g_lightPositionIndex[MAX_LIGHTS];
GLuint g_lightDirectionIndex[MAX_LIGHTS];
GLuint g_lightAmbientIndex[MAX_LIGHTS];
GLuint g_lightDiffuseIndex[MAX_LIGHTS];
GLuint g_lightAttenuationIndex[MAX_LIGHTS];
GLuint g_lightSpecularIndex[MAX_LIGHTS];
GLuint g_lightTypeIndex[MAX_LIGHTS];

GLuint g_materialAmbientIndex = 0;
GLuint g_materialDiffuseIndex = 0;
GLuint g_materialSpecularIndex = 0;
GLuint g_materialShininessIndex = 0;

Camera g_camera;            // camera 

glm::mat4 g_modelMatrix[8];		// object's model matrix 1&3 spheres, 2 room , 4,5,6 Table/Chairs , 7&8 ornaments
glm::mat4 g_viewMatrix;			// view matrix
glm::mat4 g_viewMatrixOrtho;
glm::mat4 g_projectionMatrix;	// projection matrix
//glm::mat4 g_AltprojectionMatrix[4]; //handle ortho x3 + perspective

Light g_light[MAX_LIGHTS];					// light properties
Light g_lightBackup[MAX_LIGHTS];					// light properties

Material g_material[2];			// sphere material properties
Material cube_material;			// Room material properties
Material Furniture_material[3];	// Furniture material properties
Material Ornament_material[2];	// Ornament material properties

GLuint g_windowWidth = 800;		// window dimensions
GLuint g_windowHeight = 600;
bool g_wireFrame = false;		// wireframe on or off
bool BlueOff = false;			//Is the blue light on or off (false = on)
bool RedOff = false;			//Is the red light on or off (false = on)
bool ToggledBlue = false;		//Has the blue light change(on/off) been applied (false = no)
bool ToggledRed = false;		//Has the red light change(on/off) been applied (false = no)
bool DiscoMode = false;			//Is disco mode enabled (false = off)
bool jumpup = true;				//is true if the model is on its way up from the table / false if its on its way down
bool multiview = false;			//false is one viewport filling the screen / true is 4 viewports with different views

float g_scale = 1.0f;
float g_rotateAngleX = 0.0f;
float g_rotateAngleY[3] = { 0.0f, 0.0f, 0.0f };
float g_ypos[10] = { -1.75f, -1.9f, -1.65f, -1.75f, -1.45f, -1.55f, -1.35f, -1.45f, -1.25f, -1.25f};
int waiter = 0;
float l_xpos =2.0f;
float l_zpos =0.0f;

static void LightToggle()
{
	if (BlueOff == true && ToggledBlue == false)
	{
		//backup the lights current value
		g_lightBackup[0].position = g_light[0].position;
		g_lightBackup[0].ambient = g_light[0].ambient;
		g_lightBackup[0].diffuse = g_light[0].diffuse;
		g_lightBackup[0].specular = g_light[0].specular;
		g_lightBackup[0].attenuation = g_light[0].attenuation;
		g_lightBackup[0].type = g_light[0].type;
		//make the light have no contribution
		g_light[0].position = glm::vec3(2.0f, 1.0f, 0.0f);
		g_light[0].ambient = glm::vec3(0.0f, 0.0f, 0.0f);
		g_light[0].diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
		g_light[0].specular = glm::vec3(0.0f, 0.0f, 0.0f);
		g_light[0].attenuation = glm::vec3(1.0f, 0.0f, 0.0f);
		g_light[0].type = 0;
		//display message that the function is done
		cout << "Blue Light Disabled" << endl;
		//set flag so that this isn't repeated endlessly
		ToggledBlue = true;
	}
	if (BlueOff == false && ToggledBlue == true) 
	{
		g_light[0].position = g_lightBackup[0].position;
		g_light[0].ambient = g_lightBackup[0].ambient;
		g_light[0].diffuse = g_lightBackup[0].diffuse;
		g_light[0].specular = g_lightBackup[0].specular;
		g_light[0].attenuation = g_lightBackup[0].attenuation;
		g_light[0].type = g_lightBackup[0].type;
		//display message that the function is done
		cout << "Blue Light Enabled" << endl;
		ToggledBlue = false;
	}
	if (RedOff == true && ToggledRed == false)
	{
		//backup the lights current value
		g_lightBackup[1].position = g_light[1].position;
		g_lightBackup[1].ambient = g_light[1].ambient;
		g_lightBackup[1].diffuse = g_light[1].diffuse;
		g_lightBackup[1].specular = g_light[1].specular;
		g_lightBackup[1].attenuation = g_light[1].attenuation;
		g_lightBackup[1].type = g_light[1].type;
		//make the light have no contribution
		g_light[1].position = glm::vec3(2.0f, 1.0f, 0.0f);
		g_light[1].ambient = glm::vec3(0.0f, 0.0f, 0.0f);
		g_light[1].diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
		g_light[1].specular = glm::vec3(0.0f, 0.0f, 0.0f);
		g_light[1].attenuation = glm::vec3(1.0f, 0.0f, 0.0f);
		g_light[1].type = 0;
		//display message that the function is done
		cout << "Red Light Disabled" << endl;
		//set flag so that this isn't repeated endlessly
		ToggledRed = true;
	}
	if (RedOff == false && ToggledRed == true)
	{
		g_light[1].position = g_lightBackup[1].position;
		g_light[1].ambient = g_lightBackup[1].ambient;
		g_light[1].diffuse = g_lightBackup[1].diffuse;
		g_light[1].specular = g_lightBackup[1].specular;
		g_light[1].attenuation = g_lightBackup[1].attenuation;
		g_light[1].type = g_lightBackup[1].type;
		//display message that the function is done
		cout << "Red Light Enabled" << endl;
		ToggledRed = false;
	}

}

bool load_mesh(const char* fileName, Mesh* mesh)
{
	// load file with assimp 
	const aiScene* pScene = aiImportFile(fileName, aiProcess_Triangulate
		| aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);

	// check whether scene was loaded
	if (!pScene)
	{
		cout << "Could not load mesh." << endl;
		return false;
	}

	// get pointer to mesh 0
	const aiMesh* pMesh = pScene->mMeshes[0];

	// store number of mesh vertices
	mesh->numberOfVertices = pMesh->mNumVertices;

	// if mesh contains vertex coordinates
	if (pMesh->HasPositions())
	{
		// allocate memory for vertices
		mesh->pMeshVertices = new Vertex[pMesh->mNumVertices];

		// read vertex coordinates and store in the array
		for (int i = 0; i < pMesh->mNumVertices; i++)
		{
			const aiVector3D* pVertexPos = &(pMesh->mVertices[i]);

			mesh->pMeshVertices[i].position[0] = (GLfloat)pVertexPos->x;
			mesh->pMeshVertices[i].position[1] = (GLfloat)pVertexPos->y;
			mesh->pMeshVertices[i].position[2] = (GLfloat)pVertexPos->z;
		}
	}

	// if mesh contains normals
	if (pMesh->HasNormals())
	{
		// read normals and store in the array
		for (int i = 0; i < pMesh->mNumVertices; i++)
		{
			const aiVector3D* pVertexNormal = &(pMesh->mNormals[i]);

			mesh->pMeshVertices[i].normal[0] = (GLfloat)pVertexNormal->x;
			mesh->pMeshVertices[i].normal[1] = (GLfloat)pVertexNormal->y;
			mesh->pMeshVertices[i].normal[2] = (GLfloat)pVertexNormal->z;
		}
	}

	// if mesh contains faces
	if (pMesh->HasFaces())
	{
		// store number of mesh faces
		mesh->numberOfFaces = pMesh->mNumFaces;

		// allocate memory for vertices
		mesh->pMeshIndices = new GLint[pMesh->mNumFaces * 3];

		// read normals and store in the array
		for (int i = 0; i < pMesh->mNumFaces; i++)
		{
			const aiFace* pFace = &(pMesh->mFaces[i]);

			mesh->pMeshIndices[i * 3] = (GLint)pFace->mIndices[0];
			mesh->pMeshIndices[i * 3 + 1] = (GLint)pFace->mIndices[1];
			mesh->pMeshIndices[i * 3 + 2] = (GLint)pFace->mIndices[2];
		}
	}

	// release the scene
	aiReleaseImport(pScene);

	return true;
}

static void init(GLFWwindow* window)
{
	glEnable(GL_DEPTH_TEST);	// enable depth buffer test

	// create and compile our GLSL program from the shader files
	g_shaderProgramID = loadShaders("PerFragLightingVS.vert", "PerFragLightingFS.frag");

	// find the location of shader variables
	GLuint positionIndex = glGetAttribLocation(g_shaderProgramID, "aPosition");
	GLuint normalIndex = glGetAttribLocation(g_shaderProgramID, "aNormal");
	g_MVP_Index = glGetUniformLocation(g_shaderProgramID, "uModelViewProjectionMatrix");
	g_MV_Index = glGetUniformLocation(g_shaderProgramID, "uModelViewMatrix");
	g_V_Index = glGetUniformLocation(g_shaderProgramID, "uViewMatrix");

	//light 1
	g_lightPositionIndex[0] = glGetUniformLocation(g_shaderProgramID, "uLight[0].position");
	g_lightDirectionIndex[0] = glGetUniformLocation(g_shaderProgramID, "uLight[0].direction");
	g_lightAmbientIndex[0] = glGetUniformLocation(g_shaderProgramID, "uLight[0].ambient");
	g_lightDiffuseIndex[0] = glGetUniformLocation(g_shaderProgramID, "uLight[0].diffuse");
	g_lightSpecularIndex[0] = glGetUniformLocation(g_shaderProgramID, "uLight[0].specular");
	g_lightAttenuationIndex[0] = glGetUniformLocation(g_shaderProgramID, "uLight[0].attenuation");
	g_lightTypeIndex[0] = glGetUniformLocation(g_shaderProgramID, "uLight[0].type");
	//light 2
	g_lightPositionIndex[1] = glGetUniformLocation(g_shaderProgramID, "uLight[1].position");
	g_lightDirectionIndex[1] = glGetUniformLocation(g_shaderProgramID, "uLight[1].direction");
	g_lightAmbientIndex[1] = glGetUniformLocation(g_shaderProgramID, "uLight[1].ambient");
	g_lightDiffuseIndex[1] = glGetUniformLocation(g_shaderProgramID, "uLight[1].diffuse");
	g_lightSpecularIndex[1] = glGetUniformLocation(g_shaderProgramID, "uLight[1].specular");
	g_lightAttenuationIndex[1] = glGetUniformLocation(g_shaderProgramID, "uLight[1].attenuation");
	g_lightTypeIndex[1] = glGetUniformLocation(g_shaderProgramID, "uLight[1].type");

	g_materialAmbientIndex = glGetUniformLocation(g_shaderProgramID, "uMaterial.ambient");
	g_materialDiffuseIndex = glGetUniformLocation(g_shaderProgramID, "uMaterial.diffuse");
	g_materialSpecularIndex = glGetUniformLocation(g_shaderProgramID, "uMaterial.specular");
	g_materialShininessIndex = glGetUniformLocation(g_shaderProgramID, "uMaterial.shininess");

	// initialise model matrix to the identity matrix
	g_modelMatrix[0] = g_modelMatrix[1] = g_modelMatrix[2] = g_modelMatrix[3] = g_modelMatrix[4] = 
		g_modelMatrix[5] = g_modelMatrix[6] = g_modelMatrix[7] = glm::mat4(1.0f);

	// set camera's view matrix
	g_camera.setViewMatrix(glm::vec3(0, 0, 4.5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	g_viewMatrix = g_camera.getViewMatrix();
//	g_viewMatrixOrtho = glm::lookAt(glm::vec3(9, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspectRatio = static_cast<float>(width) / height;

	// set camera's projection matrix
	g_camera.setProjection(45.0f, aspectRatio, 0.1f, 100.0f);

	// initialise projection matrix
	g_projectionMatrix = glm::perspective(45.0f, aspectRatio, 0.1f, 100.0f);
	/*
	g_AltprojectionMatrix[0] = glm::perspective(45.0f, aspectRatio, 0.1f, 100.0f);
	g_AltprojectionMatrix[1] = glm::perspective(45.0f, aspectRatio, 0.1f, 100.0f);
	g_AltprojectionMatrix[2] = glm::perspective(45.0f, aspectRatio, 0.1f, 100.0f);
	g_AltprojectionMatrix[3] = glm::perspective(45.0f, aspectRatio, 0.1f, 100.0f);
	*/
	// load mesh
	g_mesh.pMeshVertices = NULL;
	g_mesh.pMeshIndices = NULL;
	cube_mesh.pMeshVertices = NULL;
	cube_mesh.pMeshIndices = NULL;
	load_mesh("models/sphere.obj", &g_mesh); //load the light marker mesh
	load_mesh("models/cubeInverted.obj", &cube_mesh); // load the room mesh
	load_mesh("models/cube.obj", &Furniture_mesh); // load the furniture mesh
	//load ornament 1 mesh
	load_mesh("models/suzanne.obj", &Ornament1_mesh);
	//load ormnament 2 mesh
	load_mesh("models/torus.obj", &Ornament2_mesh);

	// initialise point light properties
	//light 1
	g_light[0].position = glm::vec3(2.0f, 1.0f, 0.0f);
	g_light[0].ambient = glm::vec3(0.1f, 0.1f, 1.0f);
	g_light[0].diffuse = glm::vec3(0.1f, 0.1f, 1.0f);
	g_light[0].specular = glm::vec3(0.1f, 0.1f, 0.4f);
	g_light[0].attenuation = glm::vec3(1.0f, 0.0f, 0.0f);
	g_light[0].type = 0;
	//light 2
	g_light[1].position = glm::vec3(-2.0f, 1.0f, 0.0f);
	g_light[1].ambient = glm::vec3(1.0f, 0.1f, 0.1f);
	g_light[1].diffuse = glm::vec3(1.0f, 0.1f, 0.1f);
	g_light[1].specular = glm::vec3(0.1f, 0.1f, 0.1f);
	g_light[1].attenuation = glm::vec3(1.0f, 0.0f, 0.0f);
	g_light[1].type = 0;

	// initialise marker properties //Blue coloured light market
	g_material[0].ambient = glm::vec3(0.1f, 0.1f, 1.0f);
	g_material[0].diffuse = glm::vec3(0.1f, 0.1f, 0.1f);
	g_material[0].specular = glm::vec3(0.1f, 0.1f, 0.1f);
	g_material[0].shininess = 1.0f;

	// initialise marker properties //Red coloured light market
	g_material[1].ambient = glm::vec3(1.0f, 0.1f, 0.1f);
	g_material[1].diffuse = glm::vec3(0.1f, 0.1f, 0.1f);
	g_material[1].specular = glm::vec3(0.1f, 0.1f, 0.1f);
	g_material[1].shininess = 1.0f;

	// initialise room properties 
	//Grey Room
	cube_material.ambient = glm::vec3(0.1f, 0.1f, 0.1f); // indirect lighting
	cube_material.diffuse = glm::vec3(0.4f, 0.4f, 0.4f); // objects colour
	cube_material.specular = glm::vec3(0.1f, 0.1f, 0.1f); // reflective highlight
	cube_material.shininess = 128.0f;

	//initialise Furniture Properties
	//Green Table
	Furniture_material[0].ambient = glm::vec3(0.0f, 0.4f, 0.0f); // indirect lighting
	Furniture_material[0].diffuse = glm::vec3(0.0f, 0.4f, 0.0f); // objects colour
	Furniture_material[0].specular = glm::vec3(0.1f, 0.1f, 0.1f); // reflective highlight
	Furniture_material[0].shininess = 128.0f;
	//Yellow Chair
	Furniture_material[1].ambient = glm::vec3(0.4f, 0.4f, 0.0f); // indirect lighting
	Furniture_material[1].diffuse = glm::vec3(0.4f, 0.4f, 0.0f); // objects colour
	Furniture_material[1].specular = glm::vec3(0.1f, 0.1f, 0.1f); // reflective highlight
	Furniture_material[1].shininess = 128.0f;
	//Red Chair
	Furniture_material[2].ambient = glm::vec3(0.4f, 0.0f, 0.0f); // indirect lighting
	Furniture_material[2].diffuse = glm::vec3(0.4f, 0.0f, 0.0f); // objects colour
	Furniture_material[2].specular = glm::vec3(0.1f, 0.1f, 0.1f); // reflective highlight
	Furniture_material[2].shininess = 128.0f;

	//initialise Ornament 1 Properties // Colour Bronze
	Ornament_material[0].ambient = glm::vec3(0.4f, 0.25f, 0.1f); // indirect lighting
	Ornament_material[0].diffuse = glm::vec3(0.8f, 0.5f, 0.2f); // objects colour
	Ornament_material[0].specular = glm::vec3(0.1f, 0.1f, 0.1f); // reflective highlight
	Ornament_material[0].shininess = 20.0f;

	//initialise Ornament 2 Properties //Colour Orange
	Ornament_material[1].ambient = glm::vec3(0.3f, 0.1f, 0.3f); // indirect lighting
	Ornament_material[1].diffuse = glm::vec3(0.5f, 0.0f, 0.5f); // objects colour
	Ornament_material[1].specular = glm::vec3(0.1f, 0.1f, 0.1f); // reflective highlight
	Ornament_material[1].shininess = 20.0f;


	// generate identifier for VBOs and copy data to GPU
	glGenBuffers(1, &g_VBO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*g_mesh.numberOfVertices, g_mesh.pMeshVertices, GL_STATIC_DRAW);
	glGenBuffers(1, &g_VBO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*cube_mesh.numberOfVertices, cube_mesh.pMeshVertices, GL_STATIC_DRAW);
	glGenBuffers(1, &g_VBO[2]);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*Furniture_mesh.numberOfVertices, Furniture_mesh.pMeshVertices, GL_STATIC_DRAW);
	glGenBuffers(1, &g_VBO[3]);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*Ornament1_mesh.numberOfVertices, Ornament1_mesh.pMeshVertices, GL_STATIC_DRAW);
	glGenBuffers(1, &g_VBO[4]);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*Ornament2_mesh.numberOfVertices, Ornament2_mesh.pMeshVertices, GL_STATIC_DRAW);

	// generate identifier for IBO and copy data to GPU
	glGenBuffers(1, &g_IBO[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * 3 * g_mesh.numberOfFaces, g_mesh.pMeshIndices, GL_STATIC_DRAW);
	glGenBuffers(1, &g_IBO[1]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * 3 * cube_mesh.numberOfFaces, cube_mesh.pMeshIndices, GL_STATIC_DRAW);
	glGenBuffers(1, &g_IBO[2]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * 3 * Furniture_mesh.numberOfFaces, Furniture_mesh.pMeshIndices, GL_STATIC_DRAW);
	glGenBuffers(1, &g_IBO[3]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * 3 * Ornament1_mesh.numberOfFaces, Ornament1_mesh.pMeshIndices, GL_STATIC_DRAW);
	glGenBuffers(1, &g_IBO[4]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO[4]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * 3 * Ornament2_mesh.numberOfFaces, Ornament2_mesh.pMeshIndices, GL_STATIC_DRAW);

	//Sphere
	// generate identifiers for VAO
	glGenVertexArrays(1, &g_VAO[0]);

	// create VAO and specify VBO data
	glBindVertexArray(g_VAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO[0]);
	glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glVertexAttribPointer(normalIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

	glEnableVertexAttribArray(positionIndex);	// enable vertex attributes
	glEnableVertexAttribArray(normalIndex);

	//Room
	// generate identifiers for VAO
	glGenVertexArrays(1, &g_VAO[1]);

	//create VA) and specify VB) data
	glBindVertexArray(g_VAO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[1]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO[1]);
	glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glVertexAttribPointer(normalIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

	glEnableVertexAttribArray(positionIndex);	// enable vertex attributes
	glEnableVertexAttribArray(normalIndex);
	
	//Table&chairs
	// generate identifiers for VAO
	glGenVertexArrays(1, &g_VAO[2]);

	//create VA) and specify VB) data
	glBindVertexArray(g_VAO[2]);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[2]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO[2]);
	glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glVertexAttribPointer(normalIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

	glEnableVertexAttribArray(positionIndex);	// enable vertex attributes
	glEnableVertexAttribArray(normalIndex);

	//Ornament1
	// generate identifiers for VAO
	glGenVertexArrays(1, &g_VAO[3]);

	//create VA) and specify VB) data
	glBindVertexArray(g_VAO[3]);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[3]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO[3]);
	glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glVertexAttribPointer(normalIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

	glEnableVertexAttribArray(positionIndex);	// enable vertex attributes
	glEnableVertexAttribArray(normalIndex);


	//Ornament 2
	// generate identifiers for VAO
	glGenVertexArrays(1, &g_VAO[4]);

	//create VA) and specify VB) data
	glBindVertexArray(g_VAO[4]);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[4]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO[4]);
	glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glVertexAttribPointer(normalIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

	glEnableVertexAttribArray(positionIndex);	// enable vertex attributes
	glEnableVertexAttribArray(normalIndex);

}

// function used to update the scene
static void update_scene(GLFWwindow * window)
{
	// update the light markers to match the light colour
	g_material[0].ambient = g_light[0].ambient;
	g_material[1].ambient = g_light[1].ambient;


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

	//Disco Mode Variable Handler
	if (DiscoMode)
	{
		float scaleFactor = 0.1;
		float g_rotationSpeed[2] = { 0.5f, 0.7f };   // for speed of rotation on own axis
		//modify rotate(g_rotateAngleY)
		if (jumpup == true)
		{
			g_rotateAngleY[1] += g_rotationSpeed[0] * scaleFactor;
			//g_rotateAngleY[2] += g_rotationSpeed[1] * scaleFactor;
			if (waiter < 7)
			{
				waiter+= 2;
			}
		}
		if (!jumpup)
		{
			g_rotateAngleY[1] += g_rotationSpeed[0] * scaleFactor;
			//g_rotateAngleY[2] += g_rotationSpeed[1] * scaleFactor;
			if (waiter > 1)
			{
				waiter-=2;

			}
		}
		if (waiter == 8) { jumpup = false; }
		if (waiter == 0) { jumpup = true; }
	}

	// update model matrix
	if (DiscoMode)
	{
		//light 1
		g_modelMatrix[0] = glm::rotate(glm::radians(g_rotateAngleX), glm::vec3(1.0f, 0.0f, 0.0f))
			* glm::rotate((g_rotateAngleY[1]), glm::vec3(0.0f, 1.0f, 0.0f))
			* glm::translate(glm::vec3(2.0f, 1.0f, 0.0f))
			* glm::scale(glm::vec3(0.1, 0.1, 0.1));

		//light 2
		g_modelMatrix[2] = glm::rotate(glm::radians(g_rotateAngleX), glm::vec3(1.0f, 0.0f, 0.0f))
			* glm::rotate((g_rotateAngleY[1]), glm::vec3(0.0f, 1.0f, 0.0f))
			* glm::translate(glm::vec3(-2.0f, 1.0f, 0.0f))
			* glm::scale(glm::vec3(0.1, 0.1, 0.1));

		//move the lights
		g_light[0].position = glm::rotateY(glm::vec3(2.0f, 1.0f, 0.0f), (g_rotateAngleY[1]));
		g_light[1].position = glm::rotateY(glm::vec3(-2.0f, 1.0f, 0.0f), (g_rotateAngleY[1]));
		//g_light[1].position = glm::vec3(0.0f, 1.0f, 2.0f);
	}
	else
	{
		//light 1
		g_modelMatrix[0] = glm::rotate(glm::radians(g_rotateAngleX), glm::vec3(1.0f, 0.0f, 0.0f))
			* glm::rotate((g_rotateAngleY[1]), glm::vec3(0.0f, 1.0f, 0.0f))
			* glm::translate(glm::vec3(2.0f, 1.0f, 0.0f))
			* glm::scale(glm::vec3(0.1, 0.1, 0.1));

		//light 2
		g_modelMatrix[2] = glm::rotate(glm::radians(g_rotateAngleX), glm::vec3(1.0f, 0.0f, 0.0f))
			* glm::rotate((g_rotateAngleY[1]), glm::vec3(0.0f, 1.0f, 0.0f))
			* glm::translate(glm::vec3(-2.0f, 1.0f, 0.0f))
			* glm::scale(glm::vec3(0.1, 0.1, 0.1));
	}

	//room
	g_modelMatrix[1] = glm::rotate(glm::radians(g_rotateAngleX), glm::vec3(1.0f, 0.0f, 0.0f))
		* glm::rotate(glm::radians(g_rotateAngleY[0]), glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::scale(glm::vec3(5, 5, 5));
	
	//Table scale 0.5
	g_modelMatrix[3] = glm::rotate(glm::radians(g_rotateAngleX), glm::vec3(1.0f, 0.0f, 0.0f))
		* glm::rotate(glm::radians(g_rotateAngleY[0]), glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::translate(glm::vec3(0.0f, -3.0f, 0.0f))
		* glm::scale(glm::vec3(1.0, 1.0, 1.0));

	//Chair1 scale 0.25
	g_modelMatrix[4] = glm::rotate(glm::radians(g_rotateAngleX), glm::vec3(1.0f, 0.0f, 0.0f))
		* glm::rotate(glm::radians(g_rotateAngleY[0]), glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::translate(glm::vec3(-1.5f, -3.0f, 0.0f))
		* glm::scale(glm::vec3(0.25, 0.25, 0.25));
	//Chair2
	g_modelMatrix[5] = glm::rotate(glm::radians(g_rotateAngleX), glm::vec3(1.0f, 0.0f, 0.0f))
		* glm::rotate(glm::radians(g_rotateAngleY[0]), glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::translate(glm::vec3(1.5f, -3.0f, 0.0f))
		* glm::scale(glm::vec3(0.25, 0.25, 0.25));

	if (DiscoMode)
	{
		//Ornament1 scale 0.25
		g_modelMatrix[6] = glm::rotate(glm::radians(g_rotateAngleX), glm::vec3(1.0f, 0.0f, 0.0f))
			* glm::rotate((g_rotateAngleY[1]), glm::vec3(0.0f, 1.0f, 0.0f))
			* glm::translate(glm::vec3(0.5f, g_ypos[waiter], 0.0f))
			* glm::scale(glm::vec3(0.25, 0.25, 0.25));
		//Ornament2
		g_modelMatrix[7] = glm::rotate(glm::radians(g_rotateAngleX), glm::vec3(1.0f, 0.0f, 0.0f))
			* glm::rotate((g_rotateAngleY[1]), glm::vec3(0.0f, 1.0f, 0.0f))
			* glm::translate(glm::vec3(-0.5f, g_ypos[waiter+1], 0.0f))
			* glm::scale(glm::vec3(0.25, 0.25, 0.25));
	}
	else
	{
		//Ornament1 scale 0.25
		g_modelMatrix[6] = glm::rotate(glm::radians(g_rotateAngleX), glm::vec3(1.0f, 0.0f, 0.0f))
			* glm::rotate((g_rotateAngleY[1]), glm::vec3(0.0f, 1.0f, 0.0f))
			* glm::translate(glm::vec3(0.5f, g_ypos[waiter], 0.0f))
			* glm::scale(glm::vec3(0.25, 0.25, 0.25));
		//Ornament2
		g_modelMatrix[7] = glm::rotate(glm::radians(g_rotateAngleX), glm::vec3(1.0f, 0.0f, 0.0f))
			* glm::rotate((g_rotateAngleY[1]), glm::vec3(0.0f, 1.0f, 0.0f))
			* glm::translate(glm::vec3(-0.5f, g_ypos[waiter+1], 0.0f))
			* glm::scale(glm::vec3(0.25, 0.25, 0.25));
	}


}

// function used to render the scene
static void render_scene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// clear colour buffer and depth buffer
	//glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	//glEnable(GL_COLOR_MATERIAL);
	glUseProgram(g_shaderProgramID);	// use the shaders associated with the shader program
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glm::mat4 MVP; 
	glm::mat4 MV; 
	
	if (multiview == true)
	{
		//Render for viewports 0-i (top-left, top-right, bottom-left)
		for (int i = 0; i < 3; i++)
		{//if's handle changing the viewMatrix and setting the appropriate viewport info
			if (i == 0)
			{
				//top left viewport (+Y axis)
				glViewport(0, g_windowHeight / 2, g_windowWidth / 2, g_windowHeight / 2);
				//set viewMatrix
				g_viewMatrixOrtho = glm::lookAt(glm::vec3(0.01, 15, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
			}
			if (i == 1)
			{
				//top right viewport(Side +-X axis)
				glViewport(g_windowWidth / 2, g_windowHeight / 2, g_windowWidth / 2, g_windowHeight / 2);
				g_viewMatrixOrtho = glm::lookAt(glm::vec3(15, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
			}
			if (i == 2)
			{
				//bottom-left viewport (Front -Z axis)
				glViewport(0, 0, g_windowWidth / 2, g_windowHeight / 2);
				g_viewMatrixOrtho = glm::lookAt(glm::vec3(0, 0, 15), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
			}

			//light orb 1
			MVP = g_projectionMatrix* g_viewMatrixOrtho * g_modelMatrix[0];
			MV = g_viewMatrixOrtho * g_modelMatrix[0];
			glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
			glUniformMatrix4fv(g_V_Index, 1, GL_FALSE, &g_viewMatrixOrtho[0][0]);

			for (int i = 0; i < MAX_LIGHTS; i++)
			{
				glUniform3fv(g_lightPositionIndex[i], 1, &g_light[i].position[0]);
				glUniform3fv(g_lightAmbientIndex[i], 1, &g_light[i].ambient[0]);
				glUniform3fv(g_lightDiffuseIndex[i], 1, &g_light[i].diffuse[0]);
				glUniform3fv(g_lightSpecularIndex[i], 1, &g_light[i].specular[0]);
				glUniform3fv(g_lightAttenuationIndex[i], 1, &g_light[i].attenuation[0]);
				glUniform1i(g_lightTypeIndex[i], g_light[i].type);
			}

			glUniform3fv(g_materialAmbientIndex, 1, &g_material[0].ambient[0]);
			glUniform3fv(g_materialDiffuseIndex, 1, &g_material[0].diffuse[0]);
			glUniform3fv(g_materialSpecularIndex, 1, &g_material[0].specular[0]);
			glUniform1fv(g_materialShininessIndex, 1, &g_material[0].shininess);
			glDrawElements(GL_TRIANGLES, g_mesh.numberOfFaces * 3, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type

			//light orb 2
			MVP = g_projectionMatrix * g_viewMatrixOrtho * g_modelMatrix[2];
			MV = g_viewMatrixOrtho * g_modelMatrix[2];
			glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
			glUniformMatrix4fv(g_V_Index, 1, GL_FALSE, &g_viewMatrixOrtho[0][0]);

			for (int i = 0; i < MAX_LIGHTS; i++)
			{
				glUniform3fv(g_lightPositionIndex[i], 1, &g_light[i].position[0]);
				glUniform3fv(g_lightAmbientIndex[i], 1, &g_light[i].ambient[0]);
				glUniform3fv(g_lightDiffuseIndex[i], 1, &g_light[i].diffuse[0]);
				glUniform3fv(g_lightSpecularIndex[i], 1, &g_light[i].specular[0]);
				glUniform3fv(g_lightAttenuationIndex[i], 1, &g_light[i].attenuation[0]);
				glUniform1i(g_lightTypeIndex[i], g_light[i].type);
			}

			glUniform3fv(g_materialAmbientIndex, 1, &g_material[1].ambient[0]);
			glUniform3fv(g_materialDiffuseIndex, 1, &g_material[1].diffuse[0]);
			glUniform3fv(g_materialSpecularIndex, 1, &g_material[1].specular[0]);
			glUniform1fv(g_materialShininessIndex, 1, &g_material[1].shininess);

			glDrawElements(GL_TRIANGLES, g_mesh.numberOfFaces * 3, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type

			//Draw Room
			glBindVertexArray(g_VAO[1]);		// make VAO active

			// set uniform shader variables
			MVP = g_projectionMatrix * g_viewMatrixOrtho * g_modelMatrix[1];
			MV = g_viewMatrixOrtho * g_modelMatrix[1];
			glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
			glUniformMatrix4fv(g_V_Index, 1, GL_FALSE, &g_viewMatrixOrtho[0][0]);

			for (int i = 0; i < MAX_LIGHTS; i++)
			{
				glUniform3fv(g_lightPositionIndex[i], 1, &g_light[i].position[0]);
				glUniform3fv(g_lightAmbientIndex[i], 1, &g_light[i].ambient[0]);
				glUniform3fv(g_lightDiffuseIndex[i], 1, &g_light[i].diffuse[0]);
				glUniform3fv(g_lightSpecularIndex[i], 1, &g_light[i].specular[0]);
				glUniform3fv(g_lightAttenuationIndex[i], 1, &g_light[i].attenuation[0]);
				glUniform1i(g_lightTypeIndex[i], g_light[i].type);
			}

			glUniform3fv(g_materialAmbientIndex, 1, &cube_material.ambient[0]);
			glUniform3fv(g_materialDiffuseIndex, 1, &cube_material.diffuse[0]);
			glUniform3fv(g_materialSpecularIndex, 1, &cube_material.specular[0]);
			glUniform1fv(g_materialShininessIndex, 1, &cube_material.shininess);

			glDrawElements(GL_TRIANGLES, cube_mesh.numberOfFaces * 3, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type

			//Draw Table
			glBindVertexArray(g_VAO[2]);		// make VAO active

			// set uniform shader variables
			MVP = g_projectionMatrix * g_viewMatrixOrtho * g_modelMatrix[3];
			MV = g_viewMatrixOrtho * g_modelMatrix[3];
			glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
			glUniformMatrix4fv(g_V_Index, 1, GL_FALSE, &g_viewMatrixOrtho[0][0]);

			for (int i = 0; i < MAX_LIGHTS; i++)
			{
				glUniform3fv(g_lightPositionIndex[i], 1, &g_light[i].position[0]);
				glUniform3fv(g_lightAmbientIndex[i], 1, &g_light[i].ambient[0]);
				glUniform3fv(g_lightDiffuseIndex[i], 1, &g_light[i].diffuse[0]);
				glUniform3fv(g_lightSpecularIndex[i], 1, &g_light[i].specular[0]);
				glUniform3fv(g_lightAttenuationIndex[i], 1, &g_light[i].attenuation[0]);
				glUniform1i(g_lightTypeIndex[i], g_light[i].type);
			}

			glUniform3fv(g_materialAmbientIndex, 1, &Furniture_material[0].ambient[0]);
			glUniform3fv(g_materialDiffuseIndex, 1, &Furniture_material[0].diffuse[0]);
			glUniform3fv(g_materialSpecularIndex, 1, &Furniture_material[0].specular[0]);
			glUniform1fv(g_materialShininessIndex, 1, &Furniture_material[0].shininess);
			glDrawElements(GL_TRIANGLES, Furniture_mesh.numberOfFaces * 3, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type

			//Draw Chair1
			// set uniform shader variables
			MVP = g_projectionMatrix * g_viewMatrixOrtho * g_modelMatrix[4];
			MV = g_viewMatrixOrtho * g_modelMatrix[4];
			glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
			glUniformMatrix4fv(g_V_Index, 1, GL_FALSE, &g_viewMatrixOrtho[0][0]);

			for (int i = 0; i < MAX_LIGHTS; i++)
			{
				glUniform3fv(g_lightPositionIndex[i], 1, &g_light[i].position[0]);
				glUniform3fv(g_lightAmbientIndex[i], 1, &g_light[i].ambient[0]);
				glUniform3fv(g_lightDiffuseIndex[i], 1, &g_light[i].diffuse[0]);
				glUniform3fv(g_lightSpecularIndex[i], 1, &g_light[i].specular[0]);
				glUniform3fv(g_lightAttenuationIndex[i], 1, &g_light[i].attenuation[0]);
				glUniform1i(g_lightTypeIndex[i], g_light[i].type);
			}

			glUniform3fv(g_materialAmbientIndex, 1, &Furniture_material[1].ambient[0]);
			glUniform3fv(g_materialDiffuseIndex, 1, &Furniture_material[1].diffuse[0]);
			glUniform3fv(g_materialSpecularIndex, 1, &Furniture_material[1].specular[0]);
			glUniform1fv(g_materialShininessIndex, 1, &Furniture_material[1].shininess);

			glDrawElements(GL_TRIANGLES, Furniture_mesh.numberOfFaces * 3, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type

			//Draw Chair2
			// set uniform shader variables
			MVP = g_projectionMatrix * g_viewMatrixOrtho * g_modelMatrix[5];
			MV = g_viewMatrixOrtho * g_modelMatrix[5];
			glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
			glUniformMatrix4fv(g_V_Index, 1, GL_FALSE, &g_viewMatrixOrtho[0][0]);

			for (int i = 0; i < MAX_LIGHTS; i++)
			{
				glUniform3fv(g_lightPositionIndex[i], 1, &g_light[i].position[0]);
				glUniform3fv(g_lightAmbientIndex[i], 1, &g_light[i].ambient[0]);
				glUniform3fv(g_lightDiffuseIndex[i], 1, &g_light[i].diffuse[0]);
				glUniform3fv(g_lightSpecularIndex[i], 1, &g_light[i].specular[0]);
				glUniform3fv(g_lightAttenuationIndex[i], 1, &g_light[i].attenuation[0]);
				glUniform1i(g_lightTypeIndex[i], g_light[i].type);
			}

			glUniform3fv(g_materialAmbientIndex, 1, &Furniture_material[2].ambient[0]);
			glUniform3fv(g_materialDiffuseIndex, 1, &Furniture_material[2].diffuse[0]);
			glUniform3fv(g_materialSpecularIndex, 1, &Furniture_material[2].specular[0]);
			glUniform1fv(g_materialShininessIndex, 1, &Furniture_material[2].shininess);

			glDrawElements(GL_TRIANGLES, Furniture_mesh.numberOfFaces * 3, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type

			//Draw Ornament1
			glBindVertexArray(g_VAO[3]);		// make VAO active

			// set uniform shader variables
			MVP = g_projectionMatrix * g_viewMatrixOrtho * g_modelMatrix[6];
			MV = g_viewMatrixOrtho * g_modelMatrix[6];
			glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
			glUniformMatrix4fv(g_V_Index, 1, GL_FALSE, &g_viewMatrixOrtho[0][0]);

			for (int i = 0; i < MAX_LIGHTS; i++)
			{
				glUniform3fv(g_lightPositionIndex[i], 1, &g_light[i].position[0]);
				glUniform3fv(g_lightAmbientIndex[i], 1, &g_light[i].ambient[0]);
				glUniform3fv(g_lightDiffuseIndex[i], 1, &g_light[i].diffuse[0]);
				glUniform3fv(g_lightSpecularIndex[i], 1, &g_light[i].specular[0]);
				glUniform3fv(g_lightAttenuationIndex[i], 1, &g_light[i].attenuation[0]);
				glUniform1i(g_lightTypeIndex[i], g_light[i].type);
			}

			glUniform3fv(g_materialAmbientIndex, 1, &Ornament_material[0].ambient[0]);
			glUniform3fv(g_materialDiffuseIndex, 1, &Ornament_material[0].diffuse[0]);
			glUniform3fv(g_materialSpecularIndex, 1, &Ornament_material[0].specular[0]);
			glUniform1fv(g_materialShininessIndex, 1, &Ornament_material[0].shininess);

			glDrawElements(GL_TRIANGLES, Ornament1_mesh.numberOfFaces * 3, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type

			//Draw Ornament2
			glBindVertexArray(g_VAO[4]);		// make VAO active
			// set uniform shader variables
			MVP = g_projectionMatrix * g_viewMatrixOrtho * g_modelMatrix[7];
			MV = g_viewMatrixOrtho * g_modelMatrix[7];
			glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
			glUniformMatrix4fv(g_V_Index, 1, GL_FALSE, &g_viewMatrixOrtho[0][0]);

			for (int i = 0; i < MAX_LIGHTS; i++)
			{
				glUniform3fv(g_lightPositionIndex[i], 1, &g_light[i].position[0]);
				glUniform3fv(g_lightAmbientIndex[i], 1, &g_light[i].ambient[0]);
				glUniform3fv(g_lightDiffuseIndex[i], 1, &g_light[i].diffuse[0]);
				glUniform3fv(g_lightSpecularIndex[i], 1, &g_light[i].specular[0]);
				glUniform3fv(g_lightAttenuationIndex[i], 1, &g_light[i].attenuation[0]);
				glUniform1i(g_lightTypeIndex[i], g_light[i].type);
			}

			glUniform3fv(g_materialAmbientIndex, 1, &Ornament_material[1].ambient[0]);
			glUniform3fv(g_materialDiffuseIndex, 1, &Ornament_material[1].diffuse[0]);
			glUniform3fv(g_materialSpecularIndex, 1, &Ornament_material[1].specular[0]);
			glUniform1fv(g_materialShininessIndex, 1, &Ornament_material[1].shininess);

			glDrawElements(GL_TRIANGLES, Ornament2_mesh.numberOfFaces * 3, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type
		}



		//bottom-right viewport (Standard using camera)
		glViewport(g_windowWidth / 2, 0, g_windowWidth / 2, g_windowHeight / 2);
	}
	else 
	{
		glViewport(0, 0, g_windowWidth, g_windowHeight);
	}
	glDisable(GL_CULL_FACE);
	glBindVertexArray(g_VAO[0]);		// make VAO active

	// set uniform shader variables
	//light orb 1
	 MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[0];
	 MV = g_camera.getViewMatrix() * g_modelMatrix[0];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
	glUniformMatrix4fv(g_V_Index, 1, GL_FALSE, &g_viewMatrix[0][0]);

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		glUniform3fv(g_lightPositionIndex[i], 1, &g_light[i].position[0]);
		glUniform3fv(g_lightAmbientIndex[i], 1, &g_light[i].ambient[0]);
		glUniform3fv(g_lightDiffuseIndex[i], 1, &g_light[i].diffuse[0]);
		glUniform3fv(g_lightSpecularIndex[i], 1, &g_light[i].specular[0]);
		glUniform3fv(g_lightAttenuationIndex[i], 1, &g_light[i].attenuation[0]);
		glUniform1i(g_lightTypeIndex[i], g_light[i].type);
	}

	glUniform3fv(g_materialAmbientIndex, 1, &g_material[0].ambient[0]);
	glUniform3fv(g_materialDiffuseIndex, 1, &g_material[0].diffuse[0]);
	glUniform3fv(g_materialSpecularIndex, 1, &g_material[0].specular[0]);
	glUniform1fv(g_materialShininessIndex, 1, &g_material[0].shininess);
	glDrawElements(GL_TRIANGLES, g_mesh.numberOfFaces * 3, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type
	
	//light orb 2
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[2];
	MV = g_camera.getViewMatrix() * g_modelMatrix[2];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
	glUniformMatrix4fv(g_V_Index, 1, GL_FALSE, &g_viewMatrix[0][0]);

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		glUniform3fv(g_lightPositionIndex[i], 1, &g_light[i].position[0]);
		glUniform3fv(g_lightAmbientIndex[i], 1, &g_light[i].ambient[0]);
		glUniform3fv(g_lightDiffuseIndex[i], 1, &g_light[i].diffuse[0]);
		glUniform3fv(g_lightSpecularIndex[i], 1, &g_light[i].specular[0]);
		glUniform3fv(g_lightAttenuationIndex[i], 1, &g_light[i].attenuation[0]);
		glUniform1i(g_lightTypeIndex[i], g_light[i].type);
	}

	glUniform3fv(g_materialAmbientIndex, 1, &g_material[1].ambient[0]);
	glUniform3fv(g_materialDiffuseIndex, 1, &g_material[1].diffuse[0]);
	glUniform3fv(g_materialSpecularIndex, 1, &g_material[1].specular[0]);
	glUniform1fv(g_materialShininessIndex, 1, &g_material[1].shininess);

	glDrawElements(GL_TRIANGLES, g_mesh.numberOfFaces * 3, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type

	//Draw Room
	glBindVertexArray(g_VAO[1]);		// make VAO active

	// set uniform shader variables
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[1];
	MV = g_camera.getViewMatrix() * g_modelMatrix[1];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
	glUniformMatrix4fv(g_V_Index, 1, GL_FALSE, &g_viewMatrix[0][0]);

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		glUniform3fv(g_lightPositionIndex[i], 1, &g_light[i].position[0]);
		glUniform3fv(g_lightAmbientIndex[i], 1, &g_light[i].ambient[0]);
		glUniform3fv(g_lightDiffuseIndex[i], 1, &g_light[i].diffuse[0]);
		glUniform3fv(g_lightSpecularIndex[i], 1, &g_light[i].specular[0]);
		glUniform3fv(g_lightAttenuationIndex[i], 1, &g_light[i].attenuation[0]);
		glUniform1i(g_lightTypeIndex[i], g_light[i].type);
	}

	glUniform3fv(g_materialAmbientIndex, 1, &cube_material.ambient[0]);
	glUniform3fv(g_materialDiffuseIndex, 1, &cube_material.diffuse[0]);
	glUniform3fv(g_materialSpecularIndex, 1, &cube_material.specular[0]);
	glUniform1fv(g_materialShininessIndex, 1, &cube_material.shininess);

	glDrawElements(GL_TRIANGLES, cube_mesh.numberOfFaces * 3, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type
	
	//Draw Table
	glBindVertexArray(g_VAO[2]);		// make VAO active

	// set uniform shader variables
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[3];
	MV = g_camera.getViewMatrix() * g_modelMatrix[3];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
	glUniformMatrix4fv(g_V_Index, 1, GL_FALSE, &g_viewMatrix[0][0]);

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		glUniform3fv(g_lightPositionIndex[i], 1, &g_light[i].position[0]);
		glUniform3fv(g_lightAmbientIndex[i], 1, &g_light[i].ambient[0]);
		glUniform3fv(g_lightDiffuseIndex[i], 1, &g_light[i].diffuse[0]);
		glUniform3fv(g_lightSpecularIndex[i], 1, &g_light[i].specular[0]);
		glUniform3fv(g_lightAttenuationIndex[i], 1, &g_light[i].attenuation[0]);
		glUniform1i(g_lightTypeIndex[i], g_light[i].type);
	}

	glUniform3fv(g_materialAmbientIndex, 1, &Furniture_material[0].ambient[0]);
	glUniform3fv(g_materialDiffuseIndex, 1, &Furniture_material[0].diffuse[0]);
	glUniform3fv(g_materialSpecularIndex, 1, &Furniture_material[0].specular[0]);
	glUniform1fv(g_materialShininessIndex, 1, &Furniture_material[0].shininess);
	glDrawElements(GL_TRIANGLES, Furniture_mesh.numberOfFaces * 3, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type

	//Draw Chair1
	// set uniform shader variables
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[4];
	MV = g_camera.getViewMatrix() * g_modelMatrix[4];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
	glUniformMatrix4fv(g_V_Index, 1, GL_FALSE, &g_viewMatrix[0][0]);

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		glUniform3fv(g_lightPositionIndex[i], 1, &g_light[i].position[0]);
		glUniform3fv(g_lightAmbientIndex[i], 1, &g_light[i].ambient[0]);
		glUniform3fv(g_lightDiffuseIndex[i], 1, &g_light[i].diffuse[0]);
		glUniform3fv(g_lightSpecularIndex[i], 1, &g_light[i].specular[0]);
		glUniform3fv(g_lightAttenuationIndex[i], 1, &g_light[i].attenuation[0]);
		glUniform1i(g_lightTypeIndex[i], g_light[i].type);
	}

	glUniform3fv(g_materialAmbientIndex, 1, &Furniture_material[1].ambient[0]);
	glUniform3fv(g_materialDiffuseIndex, 1, &Furniture_material[1].diffuse[0]);
	glUniform3fv(g_materialSpecularIndex, 1, &Furniture_material[1].specular[0]);
	glUniform1fv(g_materialShininessIndex, 1, &Furniture_material[1].shininess);

	glDrawElements(GL_TRIANGLES, Furniture_mesh.numberOfFaces * 3, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type

	//Draw Chair2
	// set uniform shader variables
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[5];
	MV = g_camera.getViewMatrix() * g_modelMatrix[5];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
	glUniformMatrix4fv(g_V_Index, 1, GL_FALSE, &g_viewMatrix[0][0]);

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		glUniform3fv(g_lightPositionIndex[i], 1, &g_light[i].position[0]);
		glUniform3fv(g_lightAmbientIndex[i], 1, &g_light[i].ambient[0]);
		glUniform3fv(g_lightDiffuseIndex[i], 1, &g_light[i].diffuse[0]);
		glUniform3fv(g_lightSpecularIndex[i], 1, &g_light[i].specular[0]);
		glUniform3fv(g_lightAttenuationIndex[i], 1, &g_light[i].attenuation[0]);
		glUniform1i(g_lightTypeIndex[i], g_light[i].type);
	}

	glUniform3fv(g_materialAmbientIndex, 1, &Furniture_material[2].ambient[0]);
	glUniform3fv(g_materialDiffuseIndex, 1, &Furniture_material[2].diffuse[0]);
	glUniform3fv(g_materialSpecularIndex, 1, &Furniture_material[2].specular[0]);
	glUniform1fv(g_materialShininessIndex, 1, &Furniture_material[2].shininess);

	glDrawElements(GL_TRIANGLES, Furniture_mesh.numberOfFaces * 3, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type

	//Draw Ornament1
	glBindVertexArray(g_VAO[3]);		// make VAO active

	// set uniform shader variables
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[6];
	MV = g_camera.getViewMatrix() * g_modelMatrix[6];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
	glUniformMatrix4fv(g_V_Index, 1, GL_FALSE, &g_viewMatrix[0][0]);

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		glUniform3fv(g_lightPositionIndex[i], 1, &g_light[i].position[0]);
		glUniform3fv(g_lightAmbientIndex[i], 1, &g_light[i].ambient[0]);
		glUniform3fv(g_lightDiffuseIndex[i], 1, &g_light[i].diffuse[0]);
		glUniform3fv(g_lightSpecularIndex[i], 1, &g_light[i].specular[0]);
		glUniform3fv(g_lightAttenuationIndex[i], 1, &g_light[i].attenuation[0]);
		glUniform1i(g_lightTypeIndex[i], g_light[i].type);
	}

	glUniform3fv(g_materialAmbientIndex, 1, &Ornament_material[0].ambient[0]);
	glUniform3fv(g_materialDiffuseIndex, 1, &Ornament_material[0].diffuse[0]);
	glUniform3fv(g_materialSpecularIndex, 1, &Ornament_material[0].specular[0]);
	glUniform1fv(g_materialShininessIndex, 1, &Ornament_material[0].shininess);

	glDrawElements(GL_TRIANGLES, Ornament1_mesh.numberOfFaces * 3, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type

	//Draw Ornament2
	glBindVertexArray(g_VAO[4]);		// make VAO active
	// set uniform shader variables
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[7];
	MV = g_camera.getViewMatrix() * g_modelMatrix[7];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
	glUniformMatrix4fv(g_V_Index, 1, GL_FALSE, &g_viewMatrix[0][0]);

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		glUniform3fv(g_lightPositionIndex[i], 1, &g_light[i].position[0]);
		glUniform3fv(g_lightAmbientIndex[i], 1, &g_light[i].ambient[0]);
		glUniform3fv(g_lightDiffuseIndex[i], 1, &g_light[i].diffuse[0]);
		glUniform3fv(g_lightSpecularIndex[i], 1, &g_light[i].specular[0]);
		glUniform3fv(g_lightAttenuationIndex[i], 1, &g_light[i].attenuation[0]);
		glUniform1i(g_lightTypeIndex[i], g_light[i].type);
	}

	glUniform3fv(g_materialAmbientIndex, 1, &Ornament_material[1].ambient[0]);
	glUniform3fv(g_materialDiffuseIndex, 1, &Ornament_material[1].diffuse[0]);
	glUniform3fv(g_materialSpecularIndex, 1, &Ornament_material[1].specular[0]);
	glUniform1fv(g_materialShininessIndex, 1, &Ornament_material[1].shininess);

	glDrawElements(GL_TRIANGLES, Ornament2_mesh.numberOfFaces * 3, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type


	glFlush();	// flush the pipeline
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
	// toggle multiple viewport mode
	if (key == GLFW_KEY_V && action == GLFW_PRESS)
	{
		// toggle the flag to render 1 or 4 viewports
		if (multiview == true) { 
			multiview = false; 		
			//cout << multiview << endl;
			return;
		}
		if (multiview == false) { 
			multiview = true; 
			//cout << multiview << endl;
			return;
		}

		return;
	}
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	// pass mouse data to tweak bar
	TwEventMousePosGLFW(xpos, ypos);
}
static void cursor_position_callback1(GLFWwindow* window, double xpos, double ypos)
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

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		// use mouse to move camera, hence use disable cursor mode
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPosCallback(window, cursor_position_callback1);
	}
	else 
	{
		// don't use mouse to move camera, hence use disable cursor mode
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		glfwSetCursorPosCallback(window, cursor_position_callback);
	}

	// pass mouse data to tweak bar
	TwEventMouseButtonGLFW(button, action);
}

// error callback function
static void error_callback(int error, const char* description)
{
	cerr << description << endl;	// output error description
}

int main(void)
{
	GLFWwindow* window = NULL;	// pointer to a GLFW window handle
	TwBar *TweakBar;			// pointer to a tweak bar

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
	window = glfwCreateWindow(g_windowWidth, g_windowHeight, "Tutorial", NULL, NULL);

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

	// set key callback function
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// initialise AntTweakBar
	TwInit(TW_OPENGL_CORE, NULL);

	// give tweak bar the size of graphics window
	TwWindowSize(g_windowWidth, g_windowHeight);
	TwDefine(" TW_HELP visible=false ");	// disable help menu
	TwDefine(" GLOBAL fontsize=3 ");		// set large font size

	// create a tweak bar
	TweakBar = TwNewBar("Main");
	TwDefine(" Main label='Controls' refresh=0.02 text=light size='220 300' ");

	// create display entries
	TwAddVarRW(TweakBar, "Wireframe", TW_TYPE_BOOLCPP, &g_wireFrame, " group='Display' ");
	//Disco Mode
	TwAddVarRW(TweakBar, "Disco Mode", TW_TYPE_BOOLCPP, &DiscoMode, " group='Display' ");
	//Turn Lights On/off
	TwAddVarRW(TweakBar, "Light1 On/Off", TW_TYPE_BOOLCPP, &BlueOff, " group='Display' ");
	TwAddVarRW(TweakBar, "Light2 On/Off", TW_TYPE_BOOLCPP, &RedOff, " group='Display' ");


	// display a separator
	TwAddSeparator(TweakBar, "Light 1", NULL);

	// create light position // light 1
	TwAddVarRW(TweakBar, "LightPos: x.", TW_TYPE_FLOAT, &g_light[0].position[0], " group='Light1' min=-10.0 max=10.0 step=0.1");
	TwAddVarRW(TweakBar, "LightPos: y.", TW_TYPE_FLOAT, &g_light[0].position[1], " group='Light1' min=-10.0 max=10.0 step=0.1");
	TwAddVarRW(TweakBar, "LightPos: z.", TW_TYPE_FLOAT, &g_light[0].position[2], " group='Light1' min=-10.0 max=10.0 step=0.1");

	// create attenuation entries
	TwAddVarRW(TweakBar, "Constant.", TW_TYPE_FLOAT, &g_light[0].attenuation[0], " group='Light1' min=0.0 max=1.0 step=0.01 ");
	TwAddVarRW(TweakBar, "Linear.", TW_TYPE_FLOAT, &g_light[0].attenuation[1], " group='Light1' min=0.0 max=1.0 step=0.001 ");
	TwAddVarRW(TweakBar, "Quadratic.", TW_TYPE_FLOAT, &g_light[0].attenuation[2], " group='Light1' min=0.0 max=1.0 step=0.001 ");
	//Ambient,Specular, Diffuse
	TwAddVarRW(TweakBar, "Ambient Red.", TW_TYPE_FLOAT, &g_light[0].ambient[0], " group='Light1 Ambient' min=0.0 max=1.0 step=0.001 ");
	TwAddVarRW(TweakBar, "Ambient Green.", TW_TYPE_FLOAT, &g_light[0].ambient[1], " group='Light1 Ambient' min=0.0 max=1.0 step=0.001 ");
	TwAddVarRW(TweakBar, "Ambient Blue.", TW_TYPE_FLOAT, &g_light[0].ambient[2], " group='Light1 Ambient' min=0.0 max=1.0 step=0.001 ");
	TwAddVarRW(TweakBar, "Specular Red.", TW_TYPE_FLOAT, &g_light[0].specular[0], " group='Light1 Specular' min=0.0 max=1.0 step=0.001 ");
	TwAddVarRW(TweakBar, "Specular Green.", TW_TYPE_FLOAT, &g_light[0].specular[1], " group='Light1 Specular' min=0.0 max=1.0 step=0.001 ");
	TwAddVarRW(TweakBar, "Specluar Blue.", TW_TYPE_FLOAT, &g_light[0].specular[2], " group='Light1 Specular' min=0.0 max=1.0 step=0.001 ");
	TwAddVarRW(TweakBar, "Diffuse Red.", TW_TYPE_FLOAT, &g_light[0].diffuse[0], " group='Light1 Diffuse' min=0.0 max=1.0 step=0.001 ");
	TwAddVarRW(TweakBar, "Diffuse Green.", TW_TYPE_FLOAT, &g_light[0].diffuse[1], " group='Light1 Diffuse' min=0.0 max=1.0 step=0.001 ");
	TwAddVarRW(TweakBar, "Diffuse Blue.", TW_TYPE_FLOAT, &g_light[0].diffuse[2], " group='Light1 Diffuse' min=0.0 max=1.0 step=0.001 ");
	// display a separator
	TwAddSeparator(TweakBar, "Light 2", NULL);

	// create light position
	TwAddVarRW(TweakBar, "LightPos: x", TW_TYPE_FLOAT, &g_light[1].position[0], " group='Light2' min=-10.0 max=10.0 step=0.1");
	TwAddVarRW(TweakBar, "LightPos: y", TW_TYPE_FLOAT, &g_light[1].position[1], " group='Light2' min=-10.0 max=10.0 step=0.1");
	TwAddVarRW(TweakBar, "LightPos: z", TW_TYPE_FLOAT, &g_light[1].position[2], " group='Light2' min=-10.0 max=10.0 step=0.1");

	// create attenuation entries
	TwAddVarRW(TweakBar, "Constant", TW_TYPE_FLOAT, &g_light[1].attenuation[0], " group='Light2' min=0.0 max=1.0 step=0.01 ");
	TwAddVarRW(TweakBar, "Linear", TW_TYPE_FLOAT, &g_light[1].attenuation[1], " group='Light2' min=0.0 max=1.0 step=0.001 ");
	TwAddVarRW(TweakBar, "Quadratic", TW_TYPE_FLOAT, &g_light[1].attenuation[2], " group='Light2' min=0.0 max=1.0 step=0.001 ");
	//Ambient,Specular, Diffuse
	TwAddVarRW(TweakBar, "Ambient Red", TW_TYPE_FLOAT, &g_light[1].ambient[0], " group='Light2 Ambient' min=0.0 max=1.0 step=0.001 ");
	TwAddVarRW(TweakBar, "Ambient Green", TW_TYPE_FLOAT, &g_light[1].ambient[1], " group='Light2 Ambient' min=0.0 max=1.0 step=0.001 ");
	TwAddVarRW(TweakBar, "Ambient Blue", TW_TYPE_FLOAT, &g_light[1].ambient[2], " group='Light2 Ambient' min=0.0 max=1.0 step=0.001 ");
	TwAddVarRW(TweakBar, "Specular Red", TW_TYPE_FLOAT, &g_light[1].specular[0], " group='Light2 Specular' min=0.0 max=1.0 step=0.001 ");
	TwAddVarRW(TweakBar, "Specular Green", TW_TYPE_FLOAT, &g_light[1].specular[1], " group='Light2 Specular' min=0.0 max=1.0 step=0.001 ");
	TwAddVarRW(TweakBar, "Specluar Blue", TW_TYPE_FLOAT, &g_light[1].specular[2], " group='Light2 Specular' min=0.0 max=1.0 step=0.001 ");
	TwAddVarRW(TweakBar, "Diffuse Red", TW_TYPE_FLOAT, &g_light[1].diffuse[0], " group='Light2 Diffuse' min=0.0 max=1.0 step=0.001 ");
	TwAddVarRW(TweakBar, "Diffuse Green", TW_TYPE_FLOAT, &g_light[1].diffuse[1], " group='Light2 Diffuse' min=0.0 max=1.0 step=0.001 ");
	TwAddVarRW(TweakBar, "Diffuse Blue", TW_TYPE_FLOAT, &g_light[1].diffuse[2], " group='Light2 Diffuse' min=0.0 max=1.0 step=0.001 ");

	// initialise rendering states
	init(window);

	// the rendering loop
	while (!glfwWindowShouldClose(window))
	{
		update_scene(window);		// update the scene

		if (g_wireFrame)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		LightToggle();

		render_scene();		// render the scene

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		TwDraw();			// draw tweak bar(s)

		glfwSwapBuffers(window);	// swap buffers
		glfwPollEvents();			// poll for events
	}

	// clean up
	if (g_mesh.pMeshVertices)
		delete[] g_mesh.pMeshVertices;
	if (g_mesh.pMeshIndices)
		delete[] g_mesh.pMeshIndices;

	glDeleteProgram(g_shaderProgramID);
	glDeleteBuffers(1, &g_IBO[0]);
	glDeleteBuffers(1, &g_IBO[1]);
	glDeleteBuffers(1, &g_IBO[2]);
	glDeleteBuffers(1, &g_IBO[3]);
	glDeleteBuffers(1, &g_IBO[4]);
	glDeleteBuffers(1, &g_VBO[0]);
	glDeleteBuffers(1, &g_VBO[1]);
	glDeleteBuffers(1, &g_VBO[2]);
	glDeleteBuffers(1, &g_VBO[3]);
	glDeleteBuffers(1, &g_VBO[4]);
	glDeleteVertexArrays(1, &g_VAO[0]);
	glDeleteVertexArrays(1, &g_VAO[1]);
	glDeleteVertexArrays(1, &g_VAO[2]);
	glDeleteVertexArrays(1, &g_VAO[3]);
	glDeleteVertexArrays(1, &g_VAO[4]);

	// uninitialise tweak bar
	TwTerminate();

	// close the window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}

