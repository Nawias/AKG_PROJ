/* OpenGL 4.0: Oświetlenie per-pixel, import obiektu z programu Anim8or
 * Biblioteki pomocnicze GLEW, freeglut, GLM
 * (C) 2021 Grzegorz Łukawski, Politechnika Świętokrzyska
 *
 * UWAGA!
 * W celu kompilacji należy ustawić odpowiednie ścieżki do katalogów z bibliotekami GLEW, freeglut i GLM
 *
 * Opcja w menu Visual Studio (dla języka angielskiego):
 *		Project -> PROG_NAME Properties...
 *		C/C++ -> Additional include directories
 *		Linker -> Additional library directories
 *
 * Opcja w menu Visual Studio (dla języka polskiego):
 *		Projekt -> Właściwości PROG_NAME...
 *		C/C++ -> Dodatkowe katalogi plików nagłówkowych
 *		Konsolidator -> Dodatkowe katalogi biblioteki
 *
 * W celu kompilacji programu z pomocą Code::Blocks należy dodać ścieżki dla bibliotek GLEW i freeglut:
 *		1) dla kompilatora: Project -> Build Options... -> Search directories -> Compiler
 *		2) dla linkera: Project -> Build Options... -> Search directories -> Linker
 *
 * W opcjach projektu należy dodać następujące biblioteki dla linkera:
 * 		Project -> Build Options... -> Linker settings -> Link libraries
 *			opengl32
 *			glew32
 *			freeglut
 */
#pragma comment(lib, "freeglut.lib")

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#include <gl/glew.h>
#include <gl/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp> 
#include <time.h>

// Dołączenie klasy kamery
#include "Camera.h"

// Dołączenie definicji struktury i obiektu 3D:
#include "Anim8orExport.h"
#include "city.c"

// Tylko dla Visual Studio:
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")
#pragma comment(lib, "opengl32.lib")

//Rozmiar okna
int window_width = 960, window_height = 600;

// Obiekt kamery
Camera camera;

// Macierz modelu, widoku i rzutowania:
glm::mat4 MatM, MatV, MatP, MatMc1, MatMc2, MatMc3, MatMc4, MatMc5, MatMc6,MatMc7,MatMc8;
GLuint MatMLoc, MatVLoc, MatMVPLoc;
// Bufor wierzchołków do renderingu:
GLuint CarVAO;
GLuint WindshieldVAO;

// Program shaderowy:
GLuint ShaderProgram;
// Położenie źródła światła:
glm::vec3 LightPosition = { 5000.0f, 1000.0f, 2000.0f };
GLuint LightPosLoc;
//Kolor źródła światła
glm::vec3 LightColor = { 1.0f, 1.0f, 0.6f };
GLuint LightColorLoc;
GLuint ViewPosLoc;


// Wygenerowanie obiektu VAO na podstawie jednej siatki (mesh) obiektu Anim8or:
GLuint Anim8orMeshToVAO(Anim8orMesh* mesh) {
	// Bufor z wierzchołkami:
	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * mesh->nVertices, mesh->coordinates, GL_STATIC_DRAW);
	// Bufor z normalnymi:
	GLuint nbo = 0;
	glGenBuffers(1, &nbo);
	glBindBuffer(GL_ARRAY_BUFFER, nbo);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * mesh->nVertices, mesh->normals, GL_STATIC_DRAW);
	// Bufor ze współrzędnymi tekstur:
	GLuint tbo = 0;
	glGenBuffers(1, &tbo);
	glBindBuffer(GL_ARRAY_BUFFER, tbo);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float) * mesh->nVertices, mesh->texcoords, GL_STATIC_DRAW);

	glm::vec4* colors = new glm::vec4[(mesh->nVertices)];
	for (int i = 0; i < mesh->nIndices/3; i++) {
		float r = (mesh->materials[mesh->matindices[i]].diffuse)[0];
		float g = (mesh->materials[mesh->matindices[i]].diffuse)[1];
		float b = (mesh->materials[mesh->matindices[i]].diffuse)[2];
		float a = (mesh->materials[mesh->matindices[i]].Brilliance);
		colors[mesh->indices[i * 3]] = glm::vec4(r, g, b, a);
		colors[mesh->indices[i * 3+1]] = glm::vec4(r, g, b, a);
		colors[mesh->indices[i * 3+2]] = glm::vec4(r, g, b, a);
	}
	GLuint cbo = 0;
	glGenBuffers(1, &cbo);
	glBindBuffer(GL_ARRAY_BUFFER, cbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float) * mesh->nVertices, colors, GL_STATIC_DRAW);

	// Przygotowanie VAO:
	GLuint CarVAO;
	glGenVertexArrays(1, &CarVAO);
	glBindVertexArray(CarVAO);
	// Index 0 -> współrzędne wierzchołków:
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	// Index 1 -> wektory normalne:
	glBindBuffer(GL_ARRAY_BUFFER, nbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	// Index 2 -> współrzędne tekstury:
	glBindBuffer(GL_ARRAY_BUFFER, tbo);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	// Index 3 -> kolory wierzchołków:
	glBindBuffer(GL_ARRAY_BUFFER, cbo);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, NULL);

	// Odblokowanie atrybutów:
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	return(CarVAO);
}

// Sprawdzenie błędów linkowania:
bool CheckLinkErrors(GLuint prog) {
	int res, logLen;
	char buff[1024];

	glGetProgramiv(prog, GL_LINK_STATUS, &res);
	if (res == GL_FALSE) {
		cout << "GL_LINK_STATUS ERROR!" << endl;
		glGetProgramInfoLog(prog, 1024, &logLen, buff);
		cout << buff << endl;
		return(false);
	}
	return(true);
}
// Sprawdzenie błędów kompilacji:
bool CheckCompileErrors(GLuint shad) {
	int res, logLen;
	char buff[1024];

	glGetShaderiv(shad, GL_COMPILE_STATUS, &res);
	if (res == GL_FALSE) {
		cout << "GL_COMPILE_STATUS ERROR!" << endl;
		glGetShaderInfoLog(shad, 1024, &logLen, buff);
		cout << buff << endl;
		return(false);
	}
	return(true);
}

// Uniwersalna funkcja wczytująca dane z pliku:
string LoadTextFile(string fileName) {
	ifstream plix;
	plix.open(fileName, ios::in);
	if (!plix.is_open()) {
		cout << "Open file error: " << fileName << endl;
		system("pause");
		exit(0);
	}
	// Odczyt całości pliku:
	string line, buff;
	while (!plix.eof()) {
		getline(plix, line);
		buff += line + "\n";
	}
	return(buff);
}

void InitScene() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// ustawienie modelu
	MatM = glm::mat4(1.0f);
	MatMc1 = glm::translate(MatM, glm::vec3(50.0f, 0.0f, 0.0f));
	MatMc1 = glm::rotate(MatMc1, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));


	MatMc2 = glm::translate(MatM, glm::vec3(-50.0f, 0.0f, 0.0f));
	MatMc2 = glm::rotate(MatMc2, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));


	MatMc3 = glm::translate(MatM, glm::vec3(0.0f, 0.0f, 50.0f));
	MatMc3 = glm::rotate(MatMc3, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	MatMc4 = glm::translate(MatM, glm::vec3(0.0f, 0.0f, -50.0f));
	MatMc4 = glm::rotate(MatMc4, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	MatMc5 = glm::translate(MatM, glm::vec3(-50.0f, 0.0f, -50.0f));
	MatMc5 = glm::rotate(MatMc5, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	MatMc6 = glm::translate(MatM, glm::vec3(50.0f, 0.0f, -50.0f));
	MatMc6 = glm::rotate(MatMc6, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	MatMc7 = glm::translate(MatM, glm::vec3(-50.0f, 0.0f, 50.0f));
	MatMc7 = glm::rotate(MatMc7, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	MatMc8 = glm::translate(MatM, glm::vec3(50.0f, 0.0f, 50.0f));
	MatMc8 = glm::rotate(MatMc8, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	

	// Przygotowanie kamery
	camera = Camera(glm::vec3(0, 2, 0));
	// Przytowanie VAO dla obiektu 3D:
	CarVAO = Anim8orMeshToVAO(object_city.meshes[0]);

	// Wczytanie kodu shaderów z plików:
	string VertSource = LoadTextFile("simple.vert");
	string FragSource = LoadTextFile("simple.frag");
	const char* src[1];

	// Inicjalizacja shaderów:
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	src[0] = VertSource.c_str();
	glShaderSource(vs, 1, src, NULL);
	glCompileShader(vs);

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	src[0] = FragSource.c_str();
	glShaderSource(fs, 1, src, NULL);
	glCompileShader(fs);

	if (!CheckCompileErrors(vs) || !CheckCompileErrors(fs)) {
		cout << "Shader compile error!" << endl;
		system("pause");
		exit(0);
	}
	// Utworzenie programu i połączenie:
	ShaderProgram = glCreateProgram();
	glAttachShader(ShaderProgram, fs);
	glAttachShader(ShaderProgram, vs);
	glLinkProgram(ShaderProgram);

	if (!CheckLinkErrors(ShaderProgram)) {
		cout << "Shader linking error!" << endl;
		system("pause");
		exit(0);
	}

	glUseProgram(ShaderProgram);
	// Zapamiętanie lokalizacji macierzy na później:
	MatMVPLoc = glGetUniformLocation(ShaderProgram, "MatMVP");
	MatMLoc = glGetUniformLocation(ShaderProgram, "MatM");
	MatVLoc = glGetUniformLocation(ShaderProgram, "MatV");
	// Analogicznie położenie światła:
	LightPosLoc = glGetUniformLocation(ShaderProgram, "lightPos");
	ViewPosLoc = glGetUniformLocation(ShaderProgram, "viewPos");
	LightColorLoc = glGetUniformLocation(ShaderProgram, "lightColor");
}

bool key_W = false;
bool key_S = false;
bool key_A = false;
bool key_D = false;

void OnKeyboardDown(unsigned char key, int x, int y) {
	switch (key) {
	case 'w':
		key_W = true;
		break;
	case 's':
		key_S = true;
		break;
	case 'a':
		key_A = true;
		break;
	case 'd':
		key_D = true;
		break;
	}
}
void OnKeyboardUp(unsigned char key, int x, int y) {
	switch (key) {
	case 'w':
		key_W = false;
		break;
	case 's':
		key_S = false;
		break;
	case 'a':
		key_A = false;
		break;
	case 'd':
		key_D = false;
		break;
	}
}

int mouse_X = window_width/2;
int mouse_Y = window_height/2;
int last_mouse_Y = mouse_Y;
int last_mouse_X = mouse_X;
bool mouseInWindow = false;

void CenterMousePointer() {
	if (mouseInWindow) {
		if (mouse_X < 100 || mouse_X > window_width - 100) {  //you can use values other than 100 for the screen edges if you like, kind of seems to depend on your mouse sensitivity for what ends up working best
			last_mouse_X = window_width / 2;   //centers the last known position, this way there isn't an odd jump with your cam as it resets
			last_mouse_Y = window_height / 2;
			glutWarpPointer(window_width / 2, window_height / 2);  //centers the cursor
		}
		else if (mouse_Y < 100 || mouse_Y > window_height - 100) {
			last_mouse_X = window_width / 2;
			last_mouse_Y = window_height / 2;
			glutWarpPointer(window_width / 2, window_height / 2);
		}
	}
}

void OnMouseEnter(int state) {
	mouseInWindow = (state == GLUT_ENTERED);
}

void OnMouseMove(int x, int y) {
	if (mouseInWindow) {
		mouse_X = x;
		mouse_Y = y;
	}
}

void handleWorldBorders() {
	glm::vec3 pos = camera.Position;
	float yaw = camera.Yaw;
	if (pos.y > 50.0f)
		pos.y = 5.0f;
	if (pos.y < 1.0f)
		pos.y = 1.0f;


	if (pos.x >= 25) {
		yaw += 90.0f;
		float tmp = pos.x;
		pos.x = -pos.z;
		pos.z = tmp - 50;
		LightPosition = glm::rotate(LightPosition,glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	} else if (pos.x < -25) {
		yaw += 90.0f;
		float tmp = pos.x;
		pos.x = -pos.z;
		pos.z = tmp + 50;
		LightPosition = glm::rotate(LightPosition, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	if (pos.z >= 25) {
		yaw -= 90.0f;
		float tmp = pos.z;
		pos.z = -pos.x;
		pos.x = tmp - 50;
		LightPosition = glm::rotate(LightPosition,glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	} else if (pos.z < -25) {
		yaw -= 90.0f;
		float tmp = pos.z;
		pos.z = -pos.x;
		pos.x = tmp + 50;
		LightPosition = glm::rotate(LightPosition,glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	camera.Position = pos;
	camera.Yaw = yaw;

	printf("Player: x:%f y:%f z:%f yaw:%f\nLight: x:%f y:%f z:%f\n", camera.Position.x, camera.Position.y, camera.Position.z,camera.Yaw, LightPosition.x, LightPosition.y, LightPosition.z);
}

void OnIdle() {
	static int last_time;
	// Time in milliseconds:
	int now_time = glutGet(GLUT_ELAPSED_TIME);

	if (last_time > 0) {
		float DeltaTime = (now_time - last_time) / 1000.0f;

		if (key_A ^ key_D) camera.ProcessKeyboard(key_A ? LEFT : RIGHT, DeltaTime);
		if (key_W ^ key_S) camera.ProcessKeyboard(key_W ? FORWARD : BACKWARD, DeltaTime);
		
		float xoffset = mouse_X - last_mouse_X;
		float yoffset = last_mouse_Y - mouse_Y;
		last_mouse_X = mouse_X;
		last_mouse_Y = mouse_Y;
		camera.ProcessMouseMovement(xoffset, yoffset);
		handleWorldBorders();

	}
	// Copy for measuring the next frame:
	last_time = now_time;
	// Refresh frame:
	glutPostRedisplay();
	CenterMousePointer();
}

void OnRender() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Wyliczenie i przekazanie macierzy:
	glUniformMatrix4fv(MatMLoc, 1, GL_FALSE, &MatM[0][0]);
	glUniformMatrix4fv(MatVLoc, 1, GL_FALSE, &MatV[0][0]);
	glm::mat4 MatMVP = MatP * camera.GetViewMatrix() * MatM;
	glUniformMatrix4fv(MatMVPLoc, 1, GL_FALSE, &MatMVP[0][0]);
	// Położenie źródła światła:
	glUniform3fv(LightPosLoc, 1, &LightPosition[0]);
	// Kolor źródła światła
	glUniform3fv(LightColorLoc, 1, &LightColor[0]);

	glUniform3fv(ViewPosLoc, 1, &(camera.Position)[0]);

	glBindVertexArray(CarVAO);
	glDrawElements(GL_TRIANGLES, object_city.meshes[0]->nIndices, GL_UNSIGNED_INT, object_city.meshes[0]->indices);

	glUniformMatrix4fv(MatMLoc, 1, GL_FALSE, &MatMc1[0][0]);
	glDrawElements(GL_TRIANGLES, object_city.meshes[0]->nIndices, GL_UNSIGNED_INT, object_city.meshes[0]->indices);
	/**/

	glUniformMatrix4fv(MatMLoc, 1, GL_FALSE, &MatMc2[0][0]);
	glDrawElements(GL_TRIANGLES, object_city.meshes[0]->nIndices, GL_UNSIGNED_INT, object_city.meshes[0]->indices);
	/**/

	glUniformMatrix4fv(MatMLoc, 1, GL_FALSE, &MatMc3[0][0]);
	glDrawElements(GL_TRIANGLES, object_city.meshes[0]->nIndices, GL_UNSIGNED_INT, object_city.meshes[0]->indices);

	/**/
	glUniformMatrix4fv(MatMLoc, 1, GL_FALSE, &MatMc4[0][0]);
	glDrawElements(GL_TRIANGLES, object_city.meshes[0]->nIndices, GL_UNSIGNED_INT, object_city.meshes[0]->indices);

	/**/
	glUniformMatrix4fv(MatMLoc, 1, GL_FALSE, &MatMc5[0][0]);
	glDrawElements(GL_TRIANGLES, object_city.meshes[0]->nIndices, GL_UNSIGNED_INT, object_city.meshes[0]->indices);

	/**/
	glUniformMatrix4fv(MatMLoc, 1, GL_FALSE, &MatMc6[0][0]);
	glDrawElements(GL_TRIANGLES, object_city.meshes[0]->nIndices, GL_UNSIGNED_INT, object_city.meshes[0]->indices);

	/**/
	glUniformMatrix4fv(MatMLoc, 1, GL_FALSE, &MatMc7[0][0]);
	glDrawElements(GL_TRIANGLES, object_city.meshes[0]->nIndices, GL_UNSIGNED_INT, object_city.meshes[0]->indices);

	/**/
	glUniformMatrix4fv(MatMLoc, 1, GL_FALSE, &MatMc8[0][0]);
	glDrawElements(GL_TRIANGLES, object_city.meshes[0]->nIndices, GL_UNSIGNED_INT, object_city.meshes[0]->indices);



	glutSwapBuffers();
}

void OnResize(int width, int height) {
	// Viewport:
	glViewport(0, 0, width, height);
	// Wygenerowanie nowej macierzy rzutowania:
	MatP = glm::perspective<float>(glm::radians(60.0f), (float)width / (float)height, 0.3f, 100.0f);
	window_width = width;
	window_height = height;
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(window_width, window_height);
	glutCreateWindow("City Walk");

	// Inicjalizacja biblioteki GLEW i sprawdzenie dostępności OpenGL 4.0:
	if (glewInit() != GLEW_OK) {
		cout << "GLEW init FAIL!" << endl;
		exit(0);
	}
	if (!GLEW_VERSION_4_0) {
		cout << "No OpenGL 4.0+ support!" << endl;
		exit(0);
	}

	// Inicjalizacja sceny:
	InitScene();

	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize);
	glutKeyboardFunc(OnKeyboardDown);
	glutKeyboardUpFunc(OnKeyboardUp);
	glutPassiveMotionFunc(OnMouseMove);
	glutEntryFunc(OnMouseEnter);
	glutIdleFunc(OnIdle);

	glutMainLoop();
	return(0);
}
