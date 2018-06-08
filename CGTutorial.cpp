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

// Achtung, die OpenGL-Tutorials nutzen glfw 2.7, glfw kommt mit einem veränderten API schon in der Version 3 

// Befindet sich bei den OpenGL-Tutorials unter "common"
#include "shader.hpp"

// Wuerfel und Kugel
#include "objects.hpp"

// Erklären ObenGL-Statemachine, lowlevel
// Version 1: seit 1992, elegantes API für die Plattformunabhägige 3D-Programmierung 
// Version 2: seit 2002, Ergänzung durch Shader-Programme, die auf Grafikkarte laufen
// Version 3: seit 2008, Fixedfunction-Pipeline nur noch als Compability-Extension
// Version 4: seit 2010 noch kaum unterstützt
// Wir setzen nun auf Version 3, schwieriger zu erlernen als Version 1
// glm-Bibliothek für Matrix-Operationen (fehlen in Version3), glew-Bibliothek, um API-Funktionen > 1.1 nutzen zu können
// glfw-Bibliothek, um OpenGL plattformunabhängig nutzen zu können (z. B. Fenster öffnen)
#define UEBUNG1 /* feste Rotation */
// Pflichtteil: Ergebnis zeigen ...
// Kode zeilenweise erklären. (Orientiert sich an http://www.opengl-tutorial.org/, Evtl. diese Tutorials dort ausführen, um zu vertiefen.)
// Dort wird glfw in der Version 2 genutzt, wir verwenden die aktuelle Version 3 http://www.glfw.org/
// Schwarze Linien wg. Shader. Kurz erklaeren, was die Shader machen...
// Vorgehensweise erklaeren, Jeweils alte cpp-Datei nach CGTutorial.cpp.n kopieren, damit jede Aenderung nachvollziehbar bleibt.
// (Voraussetzung fuer Support)
// Aufgabe Rotation: glm::rotate    (http://glm.g-truc.net/glm.pdf)
#define UEBUNG2 /* variable Rotation ueber R-Taste */
// Eventloop, kann man muss man aber nicht in glfw (Alternativ Glut, dort muss man)
// Aufgabe: Hinweis globale Variable, Taste...
#define UEBUNG3 /* Kamerasteuerung */
// Aufgabe drei Unabhängige Rotationen, wird zu erstem Teil des Pflichtteils
// Hinweis auf andere Kaperasteuerungen ->  http://www.opengl-tutorial.org
#define UEBUNG4 /* Ausgemaltes Objekt und Z-Buffer */
// OpenGL-Befehle: http://wiki.delphigl.com/index.php/Hauptseite auf Deutsch!
// GLEW http://glew.sourceforge.net/
// Wireframe vs. Solid thematisieren, Z-Buffer wird noetig, um Verdeckungsproblem zu lösen
// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-4-a-colored-cube/
///////////////////////////////////////////////////////
#define UEBUNG5 /* Einlesen eines Objekts */
// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
// Bessere OBJ-Loader vorhanden, basieren aber auf OpenGL1.1
// Andere Formate möglich, z. B. 3ds, s. a. Link am Ende der Seite
// Wir nehmen Teapot, Anekdote zu Teapot:  von Newell 1975 (hat berühmtes Lehrbuch geschrieben) glut, 3dsmax, Toystory, Monster AG, ...
//
// Vertex Buffer Objects (VBO) (ab 1.5) enthalten alle Daten zu Oberflaechen eines Objekts (Eckpunktkoordinaten, Normalen, Texturkoordinaten -> Uebung 6)
// koennen in separatem Speicher einer Grafikkarte gehalten werden (haengt vom Modell ab)
// koennen ueber API-Aufrufe veraendert werden --> glBufferData, glMapBuffer (GL_STATIC_DRAW, um mitzuteilen, dass sich nichts aendern wird)
// Vertex Array Objects (VAO) (ab 3) kapseln mehrere VBOs eines Objects zwecks Optimierung und einfachrer Benutzung:
// Beim Zeichnen nur 2 Aufrufe: glBindVertexArray und glDrawArrays
//#define UEBUNG5TEDDY  /* Nach der Teddy-Übung fuer Uebung6 wieder ausschalten */
// Alternativ: Teddy wenn die Zeit reicht: google nach teddy chameleon, teapot.obj mit xxx.obj austauschen....
// Modellieren kann also auch einfach sein, Freies Tool Blender (open Source), Professionelle Werkzeuge Maya, 3dsmax, etc. (nicht gegenstand der LV)
#define UEBUNG6 /* Beleuchten, neuer Shader */
// Teddy-Modell hat keine Normalen, passt nicht zu Shadern, wieder zur Teekanne zurueck.
// Shader anschauen, Alter Shader "ColourFragmentShader" setzt Farbe direkt, wird interpoliert ("Gouraud-Shading")
// "TransformVertexShader" gibt Farben weiter, legt layout der Eingabe-Daten fest, verwendet MVP um Eckpunkte zu transvormieren ("Eine Matrix reicht")
// Neue Shader koennen mehr (Immer noch nicht, was die Fixed-Function-Pipeline konnte) 
// Normalen stehen senkrecht auf Oberflaechen, wichtig fuer die Beleuchtung
// Normalen muessen anders behandelt werden als Eckpunkte, deshalb Trennung von MVP in Model View Projection
// -> Shader verändern in Uebung 15
#define UEBUNG7 /* Texturieren */
// Farbtexturen sind digitale Rasterbilder (andere Texturen in Kombination moeglich "Multi-Texturing" nicht Gegenstand der Uebung -> VL Textur)
// Imageloader fuer png und jpg nicht in den Bibliotheken enthalten -> SOIL
// DDS-Format kompatibel zu Texturkompression der Hardware. Wir nehmen aber BMP !
// s. a.:  http://www.opengl-tutorial.org/beginners-tutorials/tutorial-5-a-textured-cube/, Achtung glfwLoadTexture2D gibt' nicht mehr in glfw-version 3
// Granularitaet: Textur pro Objekt. kann also gewechselt werden ! --> Uebung 8
// Texturierte Teekanne ist 2. Teil des Pflichtteils 
///////////////////////////////////////////////////////
#define UEBUNG8 /* Mehrere Objekte */
// Polygone bilden Objekt, Objekte haben eigene Transformationen ("Model"-Matrix) eigene Texturen, etc.
// VAOs abwechselnd binden, MVP-Matrizen abwechselnd setzen, ggf Texturen abwechselnd binden, OpenGL verwendet jeweils positive Zahlen als
// Namen, die man sich generieren lassen muss.
// Model-Matrix fuer jedes Objekt anders, Szenen haben meist hierarchische Struktur, Multiplikationskette von der Wurzel zum Blatt
// Roboter-Beispiel, um Ketten zu lernen
#define UEBUNG9 /* Koordinatenachse */
// Notwendigkeit Koordinatensysteme anzeigen zu lassen... -> drawCS-Funktion mit drei Balkenen
// Wie erhaelt man langen duennen Balken mit bekannten Geometrien ?
// Aufgabe: Balken (leider mit M, V, P, ... als Parameter) Hinweis ginge auch als Singleton...
// Skalierungsparameter ruhig ausprobieren.
// sieht man den Balken im Bauch der Teekanne ?
#define UEBUNG10 /* Koordinatenkreuz */
// Aufgabe: Drei Balken, spaeter entsteht die Notwendigkeit Matrizen zu sichern (evtl. Mechanismus von OpenGL1 erwaehnen)
#define UEBUNG11 /* Ein Robotersegment */
// Teekanne ausblenden, Kugel zeichnen, Transformationsreihenfolge nochmal thematisieren
// Aufgabe: -Funktion mit Parameter!
#define UEBUNG12 /* Drei Robotersegmente uebereinander */
// Aufgabe: statischer Roboter, Unterschied lokales vs. globales Translate
#define UEBUNG13 /* Rotationen in den Gelenken */
// Aufgabe Roboter mit Tastensteuerung, Reihenfolge der Transformationen von oben nach unten und umgekehrt mit Stiften erläutern
#define UEBUNG14 /* Lampe am Ende */
// Uebung 15 im StandardShading pixelshader
// Hier mal exemplarisch den Pixelshader aendern, um die Beleuchtung ansprechender zu machen
// Erwaehnen, dass man Parameter wie "MVP" kontrollieren koennte.
// Hier beginnt die Welt der Shader-programmierung, die nicht Gegenstand der Uebung ist.
// Lampe am Ende eines steuerbaren Roboters ist dritter Teil des Pflichtteils
// (5 Punkte: 3 Rotationen, Teekanne, texturiert, Roboter, Licht am Ende) 

void error_callback(int error, const char* description)
{
	fputs(description, stderr); // gibt string aus (in dem Fall auf Konsole)
}

#ifdef UEBUNG2
	float angle = 0;
#endif // UEBUNG2

#ifdef UEBUNG3
	float anglex = 0;
	float angley = 0;
	float anglez = 0;
#endif // UEBUNG3

#ifdef UEBUNG13
	float angle0 = 0;
	float angle1 = 0;
	float angle2 = 0;
	float angle3 = 0;
#endif // UEBUNG13

	
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (key)
	{
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GL_TRUE);
		break;
#ifdef UEBUNG2
	case GLFW_KEY_R:
		angle += 5;
		break;
#endif // UEBUNG2

#ifdef UEBUNG3
	case GLFW_KEY_X:
		anglex += 5;
		break;
	case GLFW_KEY_Y:
		angley += 5;
		break;
	case GLFW_KEY_Z:
		anglez += 5;
		break;
#endif // UEBUNG3


#ifdef UEBUNG13
	case GLFW_KEY_RIGHT:
		angle0 += 5;
		break;
	case GLFW_KEY_LEFT:
		angle1 += 5;
		break;
	case GLFW_KEY_UP:
		angle2 += 5;
		break;
	case GLFW_KEY_DOWN:
		angle3 += 5;
		break;
#endif // UEBUNG13


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

#ifdef UEBUNG6
	glUniformMatrix4fv(glGetUniformLocation(programID, "M"), 1, GL_FALSE, &Model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(programID, "V"), 1, GL_FALSE, &View[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(programID, "P"), 1, GL_FALSE, &Projection[0][0]);
#endif // UEBUNG6


}

#ifdef UEBUNG5
#include "objloader.hpp"
#endif
#ifdef UEBUNG7
#include "texture.hpp"
#endif // UEBUNG7

#ifdef UEBUNG9
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
#endif

#ifdef UEBUNG11
void drawSeg(GLfloat height)
{
	glm::mat4 Save = Model;

	Model = glm::translate(Model, glm::vec3(0, height / 2, 0));
	Model = glm::scale(Model, glm::vec3(height / 6, height / 2, height / 6));
	sendMVP();
	drawSphere(100, 100);
	Model = Save;

}
#endif

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
	GLFWwindow* window = glfwCreateWindow(1024, // Breite
										  768,  // Hoehe
										  "CG - Tutorial", // Ueberschrift
										  NULL,  // windowed mode
										  NULL); // shared windoe

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

#ifdef UEBUNG6
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
#else
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader");
#endif // UEBUNG6
#ifdef UEBUNG7
	GLuint Texture = loadBMP_custom("mandrill.bmp");
#endif // UEBUNG7

	// Shader auch benutzen !
	glUseProgram(programID);

#ifdef UEBUNG4
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
#endif // UEBUNG4

#ifdef UEBUNG5
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	bool res = loadOBJ("teapot.obj", vertices, uvs, normals);
	GLuint VertexArrayIDTeapot;
	glGenVertexArrays(1, &VertexArrayIDTeapot); // id wird an ein in methode erstelltes vbo object zugewiesen
	glBindVertexArray(VertexArrayIDTeapot); // id wird an Objekt gebunden

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

#ifdef UEBUNG6
	GLuint normalBuffer;
	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(2); // ist im Shader so hinterlegt
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

#ifdef UEBUNG7
	GLuint textureBuffer;
	glGenBuffers(1, &textureBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(1); // ist im Shader so hinterlegt
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

#endif // UEBUNG7
#endif // UEBUNG6
#endif // UEBUNG55

#ifdef UEBUNG14
	glm::mat4 lightTransform(1.0f);
#endif // UEBUNG14

	// Eventloop
	while (!glfwWindowShouldClose(window))
	{
#ifdef UEBUNG4
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT);
#endif // UEBUNG4


		// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
		Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
		
		// Camera matrix
		View = glm::lookAt(glm::vec3(0,0,-5), // Camera is at (0,0,-5), in World Space
						   glm::vec3(0,0,0),  // and looks at the origin
						   glm::vec3(0,1,0)); // Head is up (set to 0,-1,0 to look upside-down)
		
		// Model matrix : an identity matrix (model will be at the origin)
		Model = glm::mat4(1.0f);

#ifdef UEBUNG1
	#ifdef UEBUNG2
		//Model = glm::rotate(Model, angle, glm::vec3(0, 0, 1));
	#else
		Model = glm::rotate(Model, 30.0f, glm::vec3(0, 1, 1));
	#endif // UEBUNG2

		//Model = glm::rotate(Model, 45.0f, glm::vec3(1,1,1));
#endif

#ifdef UEBUNG3
		Model = glm::rotate(Model, anglex, glm::vec3(1, 0, 0));
		Model = glm::rotate(Model, angley, glm::vec3(0, 1, 0));
		Model = glm::rotate(Model, anglez, glm::vec3(0, 0, 1));
#endif // UEBUNG3

#ifdef UEBUNG8
		glm::mat4 Save = Model;
		Model = glm::translate(Model, glm::vec3(1.5, 0, 0));
#endif // UEBUNG8
#ifdef UEBUNG5

		Model = glm::scale(Model, glm::vec3(1.0/1000, 1.0/1000, 1.0/1000));
#endif // UEBUNG5

		sendMVP();

#ifdef UEBUNG6
#ifdef UEBUNG14
		glm::vec4 lightPos = lightTransform * glm::vec4(0, 0, 0, 1);
#else
		glm::vec3 lightPos = glm::vec3(3, 4, -4);
#endif // UEBUNG14
		glUniform3f(glGetUniformLocation(programID, "LightPosition_worldspace"), 
			lightPos.x, lightPos.y, lightPos.z);
#endif // UEBUNG6
#ifdef UEBUNG7
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glUniform1i(glGetUniformLocation(programID, "myTextureSampler"), 0); // 1 integer wert

#endif // UEBUNG7
		
		//drawCube();

#ifdef UEBUNG5
		glBindVertexArray(VertexArrayIDTeapot);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

#endif // UEBUNG5
#ifdef UEBUNG8
		Model = Save;
		Model = glm::scale(Model, glm::vec3(0.5, 0.5, 0.5));

#ifdef UEBUNG11
#ifdef UEBUNG13
		Model = glm::rotate(Model, angle0, glm::vec3(0, 1, 0));
		Model = glm::rotate(Model, angle1, glm::vec3(0, 0, 1));
#endif // UEBUNG13

		drawSeg(1.0);

#ifdef UEBUNG12
		Model = glm::translate(Model, glm::vec3(0, 1, 0));
#ifdef UEBUNG13
		Model = glm::rotate(Model, angle2, glm::vec3(0, 0, 1));
#endif // UEBUNG13
		drawSeg(0.8);
#ifdef UEBUNG13
		Model = glm::translate(Model, glm::vec3(0, 0.8, 0));
		Model = glm::rotate(Model, angle3, glm::vec3(0, 0, 1));
#endif // UEBUNG13
		drawSeg(0.6);
		lightTransform = glm::translate(Model, glm::vec3(0, 0.6, 0));

		Model = Save;
		// Model = glm::translate(Model, glm::vec3(0, 0.6, 0));
#endif // UEBUNG12

#else
		Model = Save;
		sendMVP();
		drawSphere(10, 10);
#endif // UEBUNG11
#endif // UEBUNG8

#ifdef UEBUNG9
		drawCS();
#endif // UEBUNG9

		// Swap buffers
		glfwSwapBuffers(window);

		// Poll for and process events 
        glfwPollEvents();
	} 

#ifdef UEBUNG5
	glDeleteBuffers(1, &vertexBuffer);
#ifdef UEBUNG6
	glDeleteBuffers(1, &normalBuffer);
#endif // UEBUNG6
#ifdef UEBUNG7
	glDeleteBuffers(1, &textureBuffer);
#endif // UEBUNG7


#endif // UEBUNG5


	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	return 0;
}

