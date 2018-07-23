// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// TODO kleiner delay bevor gegner setzt
// Bug: Spiel beendet nicht richtig

//for console output
#include <iostream>
//for delay
#include <chrono>
#include <thread>
//std::this_thread::sleep_for(std::chrono::milliseconds(1000));

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

#include "objloader.hpp"
#include "texture.hpp"

#include "text2d.hpp"

void error_callback(int error, const char* description)
{
	fputs(description, stderr); // gibt string aus (in dem Fall auf Konsole)
}
float anglex = 0;
float angley = 0;
float anglez = 0;
float camangle = 17; //17 to remove board from view




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


bool gameOver = false;
bool playerWon = false;
bool computerTurn = false;
const int PLAYER = 1;
const int NPC = 2;

const int NPC_WAIT_TIME_IN_FRAMES = 60;
int currentNPCWaitTime = NPC_WAIT_TIME_IN_FRAMES;

const double CAMERA_MOVEMENT_SPEED = 0.05;

const int BOARD_SIZE = 8;
int boardArray[BOARD_SIZE][BOARD_SIZE];
float boardChipZPositionArray[BOARD_SIZE][BOARD_SIZE];

//save cordinates of last move
int lastMoveColumn = -1;
int lastMoveRow = -1;
//number of moves
int totalMoves = 0;

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

	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, chipNormals.size() * sizeof(glm::vec3), &chipNormals[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(2); // ist im Shader so hinterlegt
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, chipUvs.size() * sizeof(glm::vec2), &chipUvs[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(1); // ist im Shader so hinterlegt
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);


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

	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, boardNormals.size() * sizeof(glm::vec3), &boardNormals[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(2); // ist im Shader so hinterlegt
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, boardUVs.size() * sizeof(glm::vec2), &boardUVs[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(1); // ist im Shader so hinterlegt
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindVertexArray(VertexArrayIDBoard);
	glDrawArrays(GL_TRIANGLES, 0, boardVertices.size());
}

void createZPositionArrayForBoard() {
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			boardChipZPositionArray[i][j] = 0;

		}

	}

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
				drawChip(getChipCurrentPosition(i, j), true, chipVertices.size());
			}
			else if (boardArray[i][j] == NPC) {
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

void deleteZPositionArray() {
	for (int i = 0; i < BOARD_SIZE; ++i) {
		delete[] boardChipZPositionArray[i];
	}
	delete[] boardChipZPositionArray;
}

void restartGame() {
	gameOver = false;
	playerWon = false;

	createArrayForBoard();
	createZPositionArrayForBoard();

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



//returns current player based on who did the last move
int getCurrentplayer() {
	if (boardArray[lastMoveColumn][lastMoveRow] == 1) {
		return 2;
	}
	if (boardArray[lastMoveColumn][lastMoveRow] == 2) {
		return 1;
	}
	else {
		return rand() % 1 + 1;
	}
}
//return true if board is full
bool isBoardFullCheck() {
	if (totalMoves == BOARD_SIZE * BOARD_SIZE) {
		return true;
	}
	return false;
}

// return die row die den nächsten freien spot hat. Falls in der column die unteren 3 steine gespielt sind(also 7 6 und 5) return 4. return -1 falls column voll.
int checkDepth(int column) {
	for (int i = BOARD_SIZE - 1; i >= 0; i--) {
		if (boardArray[column][i] == 0) {
			return i;
		}
	}
	return -1;
}
//kopie des brettes für kalkulationen für den bot
int boardArrayCopy[BOARD_SIZE][BOARD_SIZE];
//kopiert den current state vom brett in die kopie
void copyTheBoard() {
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			boardArrayCopy[i][j] = boardArray[i][j];

		}

	}

}
//checkt falls durch das platzieren eines steines der momentane spieler das spiel gewonnen haette. return true falls winning move.
bool isWinningMove(int column, int player) {

	copyTheBoard();
	int depth = checkDepth(column);
	if (depth != -1) {
		boardArrayCopy[column][depth] = player;


	}
	return false;

}

bool hasWonCopy(int player) {
	if (hasDiagonalWinCopy(player) && hasHorizontalWinCopy(player) && hasVerticalWinCopy(player)) {
		return true;
	}
	else {
		return false;
	}

}

bool hasDiagonalWinCopy(int P) {
	int directions[4][2] = { { 1,0 },{ 1,-1 },{ 1,1 },{ 0,1 } };
	for (int i = 0; i < 4; i++) {
		int dx = directions[i][0];
		int dy = directions[i][1];
		for (int x = 0; x < BOARD_SIZE; x++) {
			for (int y = 0; y < BOARD_SIZE; y++) {
				int lastx = x + 3 * dx;
				int lasty = y + 3 * dy;
				if (0 <= lastx && lastx < BOARD_SIZE && 0 <= lasty && lasty < BOARD_SIZE) {
					int w = P;
					if (w != 0 && w == boardArrayCopy[x + dx][y + dy]
						&& w == boardArrayCopy[x + 2 * dx][y + 2 * dy]
						&& w == boardArrayCopy[lastx][lasty]) {
						return true;
					}
				}
			}
		}
	}
	return false; // no winner
}

bool hasHorizontalWinCopy(int P) {
	for (int row = 0; row < BOARD_SIZE; row++) {
		for (int col = 0; col < BOARD_SIZE - 4; col++) {
			bool isWin = true;
			for (int i = 0; i < 4 && isWin; i++) {
				isWin = (boardArrayCopy[row][col + i] == P);
			}
			if (isWin) { return true; }
		}
	}
	return false;
}

bool hasVerticalWinCopy(int P) {
	for (int col = 0; col < BOARD_SIZE; col++) {
		for (int row = 0; row < BOARD_SIZE - 4; row++) {
			bool isWin = true;
			for (int i = 0; i < 4 && isWin; i++) {
				isWin = (boardArrayCopy[row + i][col] == P);
			}
			if (isWin) { return true; }
		}
	}
	return false;
}


//play a chip for a player in a given column on the first free space in that column
bool playChip(int column, int player) {
	int depth = checkDepth(column);
	if (depth != -1) {
		if (player == 1) {
			if (isWinningMove(column, player)) {
				gameOver = true;
				playerWon = true;

			}
			boardArray[column][depth] = 1;
			lastMoveColumn = column;
			lastMoveRow = depth;
			totalMoves++;
			if (isBoardFullCheck()) {
				gameOver = true;
			}
			return true;
		}
		else {
			if (isWinningMove(column, player)) {
				gameOver = true;
				playerWon = false;

			}
			boardArray[column][depth] = 2;
			lastMoveColumn = column;
			lastMoveRow = depth;
			totalMoves++;
			if (isBoardFullCheck()) {
				gameOver = true;
			}
			return true;
		}
		return false;
	}
	else { return false; }
}

int totalMovesCopy = 0;
void copyTotalMoves() {
	totalMovesCopy = totalMoves;
}
//playchip but for the copy board to calculate for the pc
bool playChipCopy(int column, int player) {
	int depth = checkDepth(column);
	if (depth != -1) {
		if (player == 1) {
			boardArrayCopy[column][depth] = 1;
			totalMovesCopy++;
			return true;
		}
		else {
			boardArrayCopy[column][depth] = 2;
			totalMovesCopy++;
			return true;
		}
		return false;
	}
	else { return false; }
}

//playchip but for the copy board to calculate for the pc
void removeChipCopy(int column) {
	int depth = checkDepth(column);

	boardArrayCopy[column][depth - 1] = 0;
	totalMovesCopy--;
}

//checkdepth but for copy
int checkCopy(int column) {
	for (int i = BOARD_SIZE - 1; i >= 0; i--) {
		if (boardArrayCopy[column][i] == 0) {
			return i;
		}
	}
	return -1;
}

int negamax(Position p, int  depth) {
	int  best = -INFINITY;
	if (endPosition(p) || depth <= 0)
		return  evaluate(p);

	//Move[]  moves = generateMoves(p);
	for (int i = 0; i < BOARD_SIZE; i++) {
		//makeMove(p, moves[i]);
		int value = -negamax(p, depth - 1);
		// undoMove(p, moves[i]);
		if (value > best)
			best = value;
	}
	return  best;
}


int negamax(int depth) {
	int  best = -INFINITY;
	if (isBoardFullCheck() || depth <= 0)
		return  evaluate(p);

	//Move[]  moves = generateMoves(p);
	for (int i = 0; i < BOARD_SIZE; i++) {
		//makeMove(p, moves[i]);
		int value = -negamax(p, depth - 1);
		// undoMove(p, moves[i]);
		if (value > best)
			best = value;
	}
	return  best;
}

int evaluateCopy() {
	int score = 0;

	// very good move for computer
	if (hasWon(mySeed)) {
		return Integer.MAX_VALUE;
	}

	// very bad move, player wins!
	if (hasWon(oppSeed)) {
		return -Integer.MIN_VALUE;
	}

	// counts open 3 in a row for the computer and the player
	int myThree = openThree(mySeed);
	int oppThree = openThree(oppSeed);

	score = myThree - oppThree; // if negative, player advantage, otherwise computer advantage

	return score;
}

int[] minmax(int depth, Seed player) {
	// Generate possible next moves in a List of int[2] of {row, col}.
	List<int[]> nextMoves = generateMoves();

	// mySeed is maximizing; while oppSeed is minimizing
	int bestScore = (player == mySeed) ? Integer.MIN_VALUE : Integer.MAX_VALUE;
	int currentScore;
	int bestRow = -1;
	int bestCol = -1;

	if (nextMoves.isEmpty() || depth == 0) {
		// Gameover or depth reached, evaluate score
		bestScore = evaluate();
	}
	else {
		for (int[] move : nextMoves) {
			// Try this move for the current "player"
			cells[move[0]][move[1]].setContent(player);
			if (player == mySeed) { // mySeed (computer) is maximizing player
				currentScore = minmax(depth - 1, oppSeed)[0];
				if (currentScore > bestScore) {
					bestScore = currentScore;
					bestRow = move[0];
					bestCol = move[1];
				}
			}
			else { // oppSeed is minimizing player
				currentScore = minmax(depth - 1, mySeed)[0];
				if (currentScore < bestScore) {
					bestScore = currentScore;
					bestRow = move[0];
					bestCol = move[1];
				}
			}
			// Undo move
			cells[move[0]][move[1]].setContent(Seed.EMPTY);
		}
	}
	return new int[] { bestScore, bestRow, bestCol };
}

/**
* Find all valid next moves. Return List of moves in int[2] of {row, col}
* or empty list if gameover
*/
List<int[]> generateMoves() {
	List<int[]> nextMoves = new ArrayList<int[]>(); // allocate List

	boolean[] columns = { false,false,false,false,false,false,false };

	// If gameover, i.e., no next move
	if (hasWon(mySeed) || hasWon(oppSeed)) {
		return nextMoves; // return empty list
	}

	// Search for possible moves and add to the List
	for (int row = GameMain.ROWS - 1; row >= 0; row--) {
		for (int col = 0; col < GameMain.COLS; col++) {
			if (columns[col] == false && cells[row][col].getContent() == Seed.EMPTY) {
				columns[col] = true;
				nextMoves.add(new int[] { row, col });
			}
		}
	}

	return nextMoves;
}

//hotseat, return true if move succesful, false otherwise

bool hotSeat(int column) {

	return playChip(column, getCurrentplayer());


}
//modes; current mode = true, rest false; for more info look at key callback
bool hotseat = false;
bool vscomputer = false;
bool menu = true;
int difficulty = 0; //schwierigkeitsgrad von computer gegner. 0= random moves,1=plays winning moves, playes against winning player moves, otherwise random

void easyNPC() {
	int v2 = rand() % BOARD_SIZE; //rng zwischen 0-7(0 1 2 3 4 5 6 7)
	while (checkDepth(v2) == -1) {
		v2 = rand() % BOARD_SIZE;
	}
	playChip(v2, 2);
}
void mediumNPC() {
	bool ididmymove = false;
	for (int i = 0; i < BOARD_SIZE; i++) {
		if (isWinningMove(i, 2)) {
			playChip(i, 2);
			ididmymove = true;
			break;
		}
	}
	if (!ididmymove) {
		for (int i = 0; i < BOARD_SIZE; i++) {
			if (isWinningMove(i, 1)) {
				playChip(i, 2);
				ididmymove = true;
				break;
			}
		}
	}
	if (!ididmymove) {
		easyNPC();
	}
}
void hardNPC() {
	copyTheBoard();
	negamax(0, 2, 1);
}



void computerMove() {
	if (currentNPCWaitTime >= 0) {
		currentNPCWaitTime--;
		return;
	}
	else {
		// reset wait time for next time 
		currentNPCWaitTime = NPC_WAIT_TIME_IN_FRAMES;
	}

	if (difficulty == 0) {
		easyNPC();
	}
	else if (difficulty == 1) {
		mediumNPC();
	}
	else if (difficulty == 2) {
		hardNPC();
	}

	computerTurn = false;
}


void drawGameInfoText() {
	if (gameOver) {
		if (playerWon) {
			printText2D("Player won!", 10, 500, 60);
			printText2D("Press 0 to go back to Menu", 420, 5, 12);
		}
		else {
			if (isBoardFullCheck()) {
				printText2D("Game tied!", 10, 500, 60);
				printText2D("Press 0 to go back to Menu", 420, 5, 12);
			}
			else if (hotseat) {
				printText2D("PLAYER2 won!", 10, 500, 60);
				printText2D("Press 0 to go back to Menu", 420, 5, 12);
			}
			else if (vscomputer) {
				printText2D("NPC won!", 10, 500, 60);
				printText2D("Press 0 to go back to Menu", 420, 5, 12);
			}

		}
	}
	else if (menu) {
		printText2D("MENU", 300, 530, 60);
		printText2D("1. Hotseat", 50, 430, 35);
		printText2D("2. vs easy NPC", 50, 330, 35);
		printText2D("3. vs medium NPC", 50, 230, 35);
		printText2D("4. vs hard NPC", 50, 130, 35);
		printText2D("5. Quit", 50, 30, 35);

	}
	else {
		printText2D("Player", 50, 550, 40);
		printText2D("vs", 350, 550, 40);
		printText2D("Press 0 to go back to Menu", 420, 5, 12);
		printText2D("1  2  3  4  5  6  7  8", 160, 530, 22);

		if (vscomputer) {
			printText2D("NPC", 500, 550, 40);
		}
		else {
			printText2D("Player2", 500, 550, 40);
		}
	}
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (key)
	{
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
		if (action == GLFW_PRESS) {
			if (!gameOver) {
				if (hotseat) {
					hotSeat(0);
				}
				else if (vscomputer && computerTurn == false) {
					hotSeat(0);

					computerTurn = true;
				}
				else if (menu) {
					hotseat = true;
					vscomputer = false;
					menu = !menu;

				}
			}
		}
		break;
	case GLFW_KEY_2:
		if (action == GLFW_PRESS) {
			if (!gameOver) {
				if (hotseat) {
					hotSeat(1);
				}
				else if (vscomputer && computerTurn == false) {
					hotSeat(1);

					computerTurn = true;
				}
				else if (menu) {
					menu = !menu;
					hotseat = false;
					vscomputer = true;
					difficulty = 0;

				}
			}
		}
		break;
	case GLFW_KEY_3:
		if (action == GLFW_PRESS) {
			if (!gameOver) {
				if (hotseat) {
					hotSeat(2);
				}
				else if (vscomputer && computerTurn == false) {
					hotSeat(2);
					computerTurn = true;

				}
				else if (menu) {
					menu = !menu;
					hotseat = false;
					vscomputer = true;
					difficulty = 1;
				}
			}
		}
		break;
	case GLFW_KEY_4:
		if (action == GLFW_PRESS) {
			if (!gameOver) {
				if (hotseat) {
					hotSeat(3);
				}
				else if (vscomputer && computerTurn == false) {
					hotSeat(3);
					computerTurn = true;
				}
				else if (menu) {
					menu = !menu;
					hotseat = false;
					vscomputer = true;
					difficulty = 2;
				}
			}
		}
		break;
	case GLFW_KEY_5:
		if (action == GLFW_PRESS) {
			if (!gameOver) {
				if (hotseat) {
					hotSeat(4);
				}
				else if (vscomputer && computerTurn == false) {
					hotSeat(4);
					computerTurn = true;

				}
				else if (menu) {
					glfwSetWindowShouldClose(window, GL_TRUE);
					break;
				}
			}
		}
		break;
	case GLFW_KEY_6:
		if (action == GLFW_PRESS) {
			if (!gameOver) {
				if (hotseat) {
					hotSeat(5);
				}
				else if (vscomputer && computerTurn == false) {
					hotSeat(5);
					computerTurn = true;

				}
			}
		}
		break;
	case GLFW_KEY_7:
		if (action == GLFW_PRESS) {
			if (!gameOver) {
				if (hotseat) {
					hotSeat(6);
				}
				else if (vscomputer && computerTurn == false) {
					hotSeat(6);
					computerTurn = true;

				}
			}
		}
		break;
	case GLFW_KEY_8:
		if (action == GLFW_PRESS) {
			if (!gameOver) {
				if (hotseat) {
					hotSeat(7);
				}
				else if (vscomputer && computerTurn == false) {
					hotSeat(7);
					computerTurn = true;

				}
			}
		}
		break;
	case GLFW_KEY_0:
	case GLFW_KEY_ESCAPE:
		if (action == GLFW_PRESS) {
			if (!menu) {
				hotseat = false;
				vscomputer = false;
				menu = !menu;
				restartGame();
			}

		}
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

	createZPositionArrayForBoard();

	initText2D("Holstein.DDS");

	// Eventloop
	while (!glfwWindowShouldClose(window))
	{
		// call computermove every frame if computer has turn to count down timer
		if (computerTurn) {
			computerMove();
		}


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(programID);

		// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
		Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

		//bewegt das board in und aus dem frame heraus
		if (menu && camangle < 17) { camangle += CAMERA_MOVEMENT_SPEED; }
		else if (!menu && camangle > 0) { camangle -= CAMERA_MOVEMENT_SPEED; }

		// Camera matrix
		View = glm::lookAt(glm::vec3(8, 20, 14), // Cam era is at (0,0,-5), in World Space
			glm::vec3(8, camangle, 8),  // and looks at the origin
			glm::vec3(0, 1, 0)); // Head is up (set to 0,-1,0 to look upside-down)

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

		drawGameInfoText();

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);



		// Swap buffers
		glfwSwapBuffers(window);

		// Poll for and process events 
		glfwPollEvents();
	}

	// this raises an exception
	//deleteArrayForBoard(); 
	//deleteZPositionArray();

	cleanupText2D();

	glDeleteBuffers(1, &vertexBuffer);

	glDeleteBuffers(1, &normalBuffer);

	glDeleteBuffers(1, &textureBuffer);

	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	return 0;
}