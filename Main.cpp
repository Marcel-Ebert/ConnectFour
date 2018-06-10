// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

// Befindet sich bei den OpenGL-Tutorials unter "common"
#include "shader.hpp"

// Wuerfel und Kugel
#include "objects.hpp"

void error_callback(int error, const char* description)
{
	fputs(description, stderr); // gibt string aus (in dem Fall auf Konsole)
}
	float anglex = 0;
	float angley = 0;
	float anglez = 0;
		
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (key)
	{
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GL_TRUE);
		break;

	case GLFW_KEY_X:
		anglex += 5;
		break;
	case GLFW_KEY_Y:
		angley += 5;
		break;
	case GLFW_KEY_Z:
		anglez += 5;
		break;


	default:
		break;
	}
}


// Diese Drei Matrizen global (Singleton-Muster), damit sie jederzeit modifiziert und
// an die Grafikkarte geschickt werden koennen
glm::mat4 Projection; //Bildschirm
glm::mat4 View; // Position+Rotation(+Skalierung) der Kamera in 3d-Raum
glm::mat4 Model; // Transformationsmatrix für Objekte
GLuint programID; 

void sendMVP()
{
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP = Projection * View * Model; 
	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform, konstant fuer alle Eckpunkte
	glUniformMatrix4fv(glGetUniformLocation(programID, "MVP"), 1, GL_FALSE, &MVP[0][0]);

	glUniformMatrix4fv(glGetUniformLocation(programID, "M"), 1, GL_FALSE, &Model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(programID, "V"), 1, GL_FALSE, &View[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(programID, "P"), 1, GL_FALSE, &Projection[0][0]);

}

#include "objloader.hpp"
#include "texture.hpp"

void drawCS()
{
	glm::mat4 Save = Model;
	Model = glm::scale(Save, glm::vec3(20, 0.001, 0.001));
	sendMVP();
	drawWireCube();

	Model = glm::scale(Save, glm::vec3(0.001, 20, 0.001));
	sendMVP();
	drawWireCube();

	Model = glm::scale(Save, glm::vec3(0.001, 0.001, 20));
	sendMVP();
	drawWireCube();

}

void drawSeg(GLfloat height)
{
	glm::mat4 Save = Model;

	Model = glm::translate(Model, glm::vec3(0, height / 2, 0));
	Model = glm::scale(Model, glm::vec3(height / 6, height / 2, height / 6));
	sendMVP();
	drawSphere(100, 100);
	Model = Save;

}

GLuint VertexArrayIDBoard;
GLuint VertexArrayIDChip;

void drawChip(glm::vec3 position, GLsizei arraysize) {
	glm::mat4 Save = Model;

	Model = glm::translate(Model, position);
	sendMVP();

	glBindVertexArray(VertexArrayIDChip);
	glDrawArrays(GL_TRIANGLES, 0, arraysize);

	Model = Save;
}

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		exit(EXIT_FAILURE);
	}

	// Fehler werden auf stderr ausgegeben, s. o.
	glfwSetErrorCallback(error_callback);

	// Open a window and create its OpenGL context
	// glfwWindowHint vorher aufrufen, um erforderliche Resourcen festzulegen
	GLFWwindow* window = glfwCreateWindow(1300, // Breite
										  1000,  // Hoehe
										  "Connect four", // Ueberschrift
										  NULL,  // windowed mode
										  NULL); // shared window

	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Make the window's context current (wird nicht automatisch gemacht)
    glfwMakeContextCurrent(window);

	// Initialize GLEW
	// GLEW ermöglicht Zugriff auf OpenGL-API > 1.1
	glewExperimental = true; // Needed for core profile

	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Auf Keyboard-Events reagieren
	glfwSetKeyCallback(window, key_callback);

	// Dark blue background
	glClearColor(0.5f, 0.3f, 1.0f, 0.0f);

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

	GLuint Texture = loadBMP_custom("mandrill.bmp");

	// Shader auch benutzen !
	glUseProgram(programID);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	bool res = loadOBJ("ConnectFourBoard.obj", vertices, uvs, normals);
	
	glGenVertexArrays(1, &VertexArrayIDBoard); // id wird an ein in methode erstelltes vbo object zugewiesen
	glBindVertexArray(VertexArrayIDBoard); // id wird an Objekt gebunden

	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer); // vbo bekommt buffer zugewiesen
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer); // buffer von typ arraybuffer wird gebunden
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), // groesse des Buffers in Byte
		&vertices[0], 
		GL_STATIC_DRAW); //kein dynamischer Inhalt - statisch gezeichnet
	
	glEnableVertexAttribArray(0); // 0 ist Standard -> Position in meisten Shadern
	glVertexAttribPointer(0, // Position in shader
		3, // Datenformat (3 Werte)
		GL_FLOAT, // Typ
		GL_FALSE, // normalisiert?
		0, // sind Punkte direkt hintereinander gespeichert? ja-0 
		(void*)0); // kann beliebigen Datentyp annehmen

	GLuint normalBuffer;
	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(2); // ist im Shader so hinterlegt
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	GLuint textureBuffer;
	glGenBuffers(1, &textureBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(1); // ist im Shader so hinterlegt
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);


	std::vector<glm::vec3> chipVertices;
	std::vector<glm::vec2> chipUvs;
	std::vector<glm::vec3> chipNormals;
	res = loadOBJ("Chip.obj", chipVertices, chipUvs, chipNormals);
	glGenVertexArrays(1, &VertexArrayIDChip); // id wird an ein in methode erstelltes vbo object zugewiesen
	glBindVertexArray(VertexArrayIDChip); // id wird an Objekt gebunden

	glGenBuffers(1, &vertexBuffer); // vbo bekommt buffer zugewiesen
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer); // buffer von typ arraybuffer wird gebunden
	glBufferData(GL_ARRAY_BUFFER, chipVertices.size() * sizeof(glm::vec3), // groesse des Buffers in Byte
		&chipVertices[0],
		GL_STATIC_DRAW); //kein dynamischer Inhalt - statisch gezeichnet

	glEnableVertexAttribArray(0); // 0 ist Standard -> Position in meisten Shadern
	glVertexAttribPointer(0, // Position in shader
		3, // Datenformat (3 Werte)
		GL_FLOAT, // Typ
		GL_FALSE, // normalisiert?
		0, // sind Punkte direkt hintereinander gespeichert? ja-0 
		(void*)0); // kann beliebigen Datentyp annehmen

	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, chipNormals.size() * sizeof(glm::vec3), &chipNormals[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(2); // ist im Shader so hinterlegt
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glGenBuffers(1, &textureBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, chipUvs.size() * sizeof(glm::vec2), &chipUvs[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(1); // ist im Shader so hinterlegt
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);


	// Eventloop
	while (!glfwWindowShouldClose(window))
	{

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT);

		// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
		Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
		
		// Camera matrix
		View = glm::lookAt(glm::vec3(0,15,15), // Camera is at (0,0,-5), in World Space
						   glm::vec3(0,0,0),  // and looks at the origin
						   glm::vec3(0,1,0)); // Head is up (set to 0,-1,0 to look upside-down)
		
		// Model matrix : an identity matrix (model will be at the origin)
		Model = glm::mat4(1.0f);

		Model = glm::rotate(Model, anglex, glm::vec3(1, 0, 0));
		Model = glm::rotate(Model, angley, glm::vec3(0, 1, 0));
		Model = glm::rotate(Model, anglez, glm::vec3(0, 0, 1));

		glm::mat4 Save = Model;

		sendMVP();

		glm::vec3 lightPos = glm::vec3(3, 4, -4);

		glUniform3f(glGetUniformLocation(programID, "LightPosition_worldspace"), lightPos.x, lightPos.y, lightPos.z);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glUniform1i(glGetUniformLocation(programID, "myTextureSampler"), 0); // 1 integer wert
		
		glBindVertexArray(VertexArrayIDBoard);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());
		
		
		drawChip(glm::vec3(1, 0, 1), chipVertices.size());

		drawChip(glm::vec3(3, 0, 3), chipVertices.size());

		drawChip(glm::vec3(5, 0, 5), chipVertices.size());



		Model = Save;
		drawCS();


		// Swap buffers
		glfwSwapBuffers(window);

		// Poll for and process events 
        glfwPollEvents();
	} 

	glDeleteBuffers(1, &vertexBuffer);

	glDeleteBuffers(1, &normalBuffer);

	glDeleteBuffers(1, &textureBuffer);

	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	return 0;
}

