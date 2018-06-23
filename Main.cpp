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

#include "text2d.hpp"

void error_callback(int error, const char* description)
{
	fputs(description, stderr); // gibt string aus (in dem Fall auf Konsole)
}
	float anglex = 0;
	float angley = 0;
	float anglez = 0;
		



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

const int PLAYER = 1;
const int NPC = 2;

const int BOARD_SIZE = 8;
int boardArray[BOARD_SIZE][BOARD_SIZE];
float boardChipZPositionArray[BOARD_SIZE][BOARD_SIZE];


GLuint VertexArrayIDBoard;
GLuint VertexArrayIDChip;
GLuint yellowTexture;
GLuint blueTexture;
GLuint woodTexture;

std::vector<glm::vec3> boardVertices;
std::vector<glm::vec2> boardUVs;
std::vector<glm::vec3> boardNormals;

std::vector<glm::vec3> chipVertices;
std::vector<glm::vec2> chipUvs;
std::vector<glm::vec3> chipNormals;

GLuint vertexBuffer;
GLuint normalBuffer;
GLuint textureBuffer;



glm::vec3 getChipTargetPosition(int i, int j) {
	return glm::vec3(1 + i * 2, 0, 1 + j * 2);
}

glm::vec3 getChipCurrentPosition(int i, int j) {
	return glm::vec3(1 + i * 2, 0, boardChipZPositionArray[i][j]);
}

void drawChip(glm::vec3 position, bool player, GLsizei arraysize) {
	glm::mat4 Save = Model;

	Model = glm::translate(Model, position);
	sendMVP();

	if (player == true) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, yellowTexture);
		glUniform1i(glGetUniformLocation(programID, "myTextureSampler"), 0);
	}
	else {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, blueTexture);
		glUniform1i(glGetUniformLocation(programID, "myTextureSampler"), 0);
	}
	glBindVertexArray(VertexArrayIDChip);
	glDrawArrays(GL_TRIANGLES, 0, arraysize);

	Model = Save;
}

void drawBoard() {
	glm::mat4 Save = Model;

	Model = glm::translate(Model, glm::vec3(8, 0, 8));
	Model = glm::rotate(Model, anglex, glm::vec3(1, 0, 0));
	Model = glm::rotate(Model, angley, glm::vec3(0, 1, 0));
	Model = glm::rotate(Model, anglez, glm::vec3(0, 0, 1));
	sendMVP();
	Model = Save;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, woodTexture);
	glUniform1i(glGetUniformLocation(programID, "myTextureSampler"), 0); // 1 integer wert

	glBindVertexArray(VertexArrayIDBoard);
	glDrawArrays(GL_TRIANGLES, 0, boardVertices.size());
}

void createArrayForBoard() {
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			boardArray[i][j] = 0;
		}
	}
}

void placeChipInBoard(int col, bool player) {
	int row = 0;
	for (int i = 0; i < BOARD_SIZE; i++) {
		if (boardArray[col][i] == 0) {
			row = i;
		}
	}

	if (player == true)
		boardArray[col][row] = PLAYER;
	else
		boardArray[col][row] = NPC;

	boardChipZPositionArray[col][row] = 0;
}


void drawChipsInBoardArray() {
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			if (boardArray[i][j] == PLAYER) {
				drawChip(getChipCurrentPosition(i,j), true, chipVertices.size());
			} else if (boardArray[i][j] == NPC) {
				drawChip(getChipCurrentPosition(i, j), false, chipVertices.size());
			}
		}
	}
}

void updateChipPositions() {
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			if (boardArray[i][j] == PLAYER || boardArray[i][j] == NPC) {
				if (boardChipZPositionArray[i][j] < getChipTargetPosition(i, j).z) {
					boardChipZPositionArray[i][j] += 0.05;
				}
			}
		}
	}
}


void deleteArrayForBoard() {
	for (int i = 0; i < BOARD_SIZE; ++i) {
		delete[] boardArray[i];
	}
	delete[] boardArray;
}


void loadBoardToMemory() {
	bool res = loadOBJ("ConnectFourBoard.obj", boardVertices, boardUVs, boardNormals);

	glGenVertexArrays(1, &VertexArrayIDBoard); // id wird an ein in methode erstelltes vbo object zugewiesen
	glBindVertexArray(VertexArrayIDBoard); // id wird an Objekt gebunden

	glGenBuffers(1, &vertexBuffer); // vbo bekommt buffer zugewiesen
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer); // buffer von typ arraybuffer wird gebunden
	glBufferData(GL_ARRAY_BUFFER, boardVertices.size() * sizeof(glm::vec3), // groesse des Buffers in Byte
		&boardVertices[0],
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
	glBufferData(GL_ARRAY_BUFFER, boardNormals.size() * sizeof(glm::vec3), &boardNormals[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(2); // ist im Shader so hinterlegt
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	
	glGenBuffers(1, &textureBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, boardUVs.size() * sizeof(glm::vec2), &boardUVs[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(1); // ist im Shader so hinterlegt
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

}


void loadChipToMemory() {
	bool res = loadOBJ("Chip.obj", chipVertices, chipUvs, chipNormals);
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
;
	glGenBuffers(1, &textureBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, chipUvs.size() * sizeof(glm::vec2), &chipUvs[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(1); // ist im Shader so hinterlegt
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

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


	case GLFW_KEY_1:
		if (action == GLFW_PRESS)
		placeChipInBoard(0, true);
		break;
	case GLFW_KEY_2:
		if (action == GLFW_PRESS)
		placeChipInBoard(1, true);
		break;
	case GLFW_KEY_3:
		if (action == GLFW_PRESS)
		placeChipInBoard(2, true);
		break;
	case GLFW_KEY_4:
		if (action == GLFW_PRESS)
		placeChipInBoard(3, true);
		break;
	case GLFW_KEY_5:
		if (action == GLFW_PRESS)
		placeChipInBoard(4, true);
		break;
	case GLFW_KEY_6:
		if (action == GLFW_PRESS)
		placeChipInBoard(5, true);
		break;
	case GLFW_KEY_7:
		if (action == GLFW_PRESS)
		placeChipInBoard(6, true);
		break;
	case GLFW_KEY_8:
		if (action == GLFW_PRESS)
		placeChipInBoard(7, true);
		break;


	default:
		break;
	}
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
	glClearColor(0.5f, 0.3f, 0.5f, 0.0f);

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

	woodTexture = loadBMP_custom("wood_texture.bmp");
	yellowTexture = loadBMP_custom("yellow_texture.bmp");
	blueTexture = loadBMP_custom("blue_texture.bmp");


	// Shader auch benutzen !
	glUseProgram(programID);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	loadBoardToMemory();

	loadChipToMemory();

	createArrayForBoard();

	initText2D("Holstein.DDS");
	
	// Eventloop
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(programID);

		// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
		Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
		
		// Camera matrix
		View = glm::lookAt(glm::vec3(8,20,14), // Camera is at (0,0,-5), in World Space
						   glm::vec3(8,0,8),  // and looks at the origin
						   glm::vec3(0,1,0)); // Head is up (set to 0,-1,0 to look upside-down)
		
		// Model matrix : an identity matrix (model will be at the origin)
		Model = glm::mat4(1.0f);

		Model = glm::rotate(Model, 15.0f, glm::vec3(1, 0, 0));

		glm::mat4 Save = Model;

		sendMVP();

		// change y to increase distance from board
		glm::vec3 lightPos = glm::vec3(8, 20, 12);

		glUniform3f(glGetUniformLocation(programID, "LightPosition_worldspace"), lightPos.x, lightPos.y, lightPos.z);

		updateChipPositions();

		drawBoard();

		drawChipsInBoardArray();
		
		drawCS();

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		printText2D("Test", 10, 500, 60);

		// Swap buffers
		glfwSwapBuffers(window);

		// Poll for and process events 
        glfwPollEvents();
	} 

	deleteArrayForBoard();

	cleanupText2D();

	glDeleteBuffers(1, &vertexBuffer);

	glDeleteBuffers(1, &normalBuffer);

	glDeleteBuffers(1, &textureBuffer);

	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	return 0;
}
