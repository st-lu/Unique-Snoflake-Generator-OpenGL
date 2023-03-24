//FERN


/*
- Survolarea unui obiect folosind coordonate sferice.
*/
#include <windows.h>  // biblioteci care urmeaza sa fie incluse
#include <stdlib.h> // necesare pentru citirea shader-elor
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <GL/glew.h> // glew apare inainte de freeglut
#include <GL/freeglut.h> // nu trebuie uitat freeglut.h
#include "loadShaders.h"
#include "glm/glm/glm.hpp"  
#include "glm/glm/gtc/matrix_transform.hpp"
#include "glm/glm/gtx/transform.hpp"
#include "glm/glm/gtc/type_ptr.hpp"
#include <vector>;
using namespace std;

// identificatori 
GLuint
VaoId,
VboId,
EboId,
ProgramId,
viewLocation,
projLocation,
codColLocation;
float const PI = 3.141592;
float const sqrt3 = 1.732051;

// variabile
int codCol;

// variabile pentru matricea de vizualizare
float Refx = 0.0f, Refy = 0.0f, Refz = 0.0f;
float alpha = 0.0f, beta = 0.0f, dist = 300.0f;
float Obsx, Obsy, Obsz;
float Vx = 0.0f, Vy = 1.0f, Vz = 0.0f;
float incr_alpha1 = 0.01, incr_alpha2 = 0.01;

// variabile pentru matricea de proiectie
float width = 800, height = 600, xwmin = -200.f, xwmax = 200, ywmin = -200, ywmax = 200, znear = 1, fov = 30;

// vectori
glm::vec3 Obs, PctRef, Vert;

// matrice utilizate
glm::mat4 view, projection;


enum SNOWFLAKE_TYPE {
	SIMPLE_PRISM,
	STELLAR_PLATE,
	STELLAR_DENDRITE,
	SECTORED_PLATES,
	FERN_DENDRITES
};

struct point {
	float x, y, z;

	point(float xx, float yy, float zz) {
		x = xx;
		y = yy;
		z = zz;
	}

	point() {
		x = 0.0;
		y = 0.0;
		z = 0.0;
	}

	void setPoint(float xx, float yy, float zz) {
		x = xx;
		y = yy;
		z = zz;
	}
};

struct transform {
	point translate;
	float rotationAngle;
	float scale;
};

point origin(0.0f, 0.0f, 0.0f);
float thickness = 3.0;
float colorR = 1.0;
float colorG = 1.0;
float colorB = 1.0;
float coreRadius = 5.0;
float minHexRadius = 100.0;
SNOWFLAKE_TYPE randomType = FERN_DENDRITES;
int noOfExtrusions = 0;
vector<int> stellarDendriteSizes;
vector<float> fernDendriteSize;
vector<float> fernDendritesDist;
vector<float> fernDendritesAngle;
float d1 = 5.0;
float d2 = 5.0;
float d3 = 5.0;

void processNormalKeys(unsigned char key, int x, int y)
{
	switch (key) {
	case '-':
		dist -= 5.0;
		break;
	case '+':
		dist += 5.0;
		break;
	}
	if (key == 27)
		exit(0);
}
void processSpecialKeys(int key, int xx, int yy)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
		beta -= 0.01;
		break;
	case GLUT_KEY_RIGHT:
		beta += 0.01;
		break;
	case GLUT_KEY_UP:
		alpha += incr_alpha1;
		if (abs(alpha - PI / 2) < 0.05)
		{
			incr_alpha1 = 0.f;
		}
		else
		{
			incr_alpha1 = 0.01f;
		}
		break;
	case GLUT_KEY_DOWN:
		alpha -= incr_alpha2;
		if (abs(alpha + PI / 2) < 0.05)
		{
			incr_alpha2 = 0.f;
		}
		else
		{
			incr_alpha2 = 0.01f;
		}
		break;
	}
}

void makeHexagon(vector<point>& vertices, float minHexRadius, point origin) {

	//SIMPLE PRISM

	//CORE

/*
	 C	________ B
	  /        \
  D /	   		\ A
	 \				/
	  \________/
	 E          F
*/

	point pnt;

	//A
	pnt.setPoint(origin.x + minHexRadius, origin.y, origin.z);
	vertices.push_back(pnt);

	//B
	pnt.setPoint(origin.x + minHexRadius / 2, origin.y + (minHexRadius * sqrt3 / 2), origin.z);
	vertices.push_back(pnt);

	//C
	pnt.setPoint(origin.x - minHexRadius / 2, origin.y + (minHexRadius * sqrt3 / 2), origin.z);
	vertices.push_back(pnt);

	//D
	pnt.setPoint(origin.x - minHexRadius, origin.y, origin.z);
	vertices.push_back(pnt);

	//E
	pnt.setPoint(origin.x - minHexRadius / 2, origin.y - (minHexRadius * sqrt3 / 2), origin.z);
	vertices.push_back(pnt);

	//F
	pnt.setPoint(origin.x + minHexRadius / 2, origin.y - (minHexRadius * sqrt3 / 2), origin.z);
	vertices.push_back(pnt);
}

void makePrism(vector<point>& vertices, float& d1, float& d2, float& d3, point origin) {
	//
	//			A
	//			|   ----> d3
	//		B__|__E ---> d3
	//       |
	//       |
	//       |  -----> d2
	//       |
	//  C____|_____D  ---> d1
	//       O

	point pnt;

	//A 
	pnt.setPoint(origin.x, origin.y + d2 + d3, origin.z);
	vertices.push_back(pnt);

	//B
	pnt.setPoint(origin.x - d3, origin.y + d2, origin.z);
	vertices.push_back(pnt);

	//C
	pnt.setPoint(origin.x - d1, origin.y, origin.z);
	vertices.push_back(pnt);

	//D
	pnt.setPoint(origin.x + d1, origin.y, origin.z);
	vertices.push_back(pnt);

	//E
	pnt.setPoint(origin.x + d3, origin.y + d2, origin.z);
	vertices.push_back(pnt);

}

void addVertices(GLfloat Vertices[], vector<point>& vertices, int size, int offset, int thickness) {

	int ssize = size / 14;

	//FATA
	for (int ii = ssize; ii < ssize * 2; ii++) {
		//point
		Vertices[offset + 0 + ii * 7] = vertices[ii % ssize].x;
		Vertices[offset + 1 + ii * 7] = vertices[ii % ssize].y;
		Vertices[offset + 2 + ii * 7] = vertices[ii % ssize].z + thickness;
		Vertices[offset + 3 + ii * 7] = 1.0f;

		//color
		Vertices[offset + 4 + ii * 7] = colorR;
		Vertices[offset + 5 + ii * 7] = colorG;
		Vertices[offset + 6 + ii * 7] = colorB;
	}

	//SPATE
	for (int ii = 0; ii < ssize; ii++) {
		//point
		Vertices[offset + 0 + ii * 7] = vertices[ii % 6].x;
		Vertices[offset + 1 + ii * 7] = vertices[ii % 6].y;
		Vertices[offset + 2 + ii * 7] = vertices[ii % 6].z - thickness;
		Vertices[offset + 3 + ii * 7] = 1.0f;

		//color
		Vertices[offset + 4 + ii * 7] = colorR;
		Vertices[offset + 5 + ii * 7] = colorG;
		Vertices[offset + 6 + ii * 7] = colorB;
	}
}

void makeFace(vector<char>& indices, float a, float b, float c, float d) {
	/*
		A _______ B
		 |\      |
		 |   \   |
	  D |______\| C


	*/
	indices.push_back(a);
	indices.push_back(b);
	indices.push_back(c);

	indices.push_back(c);
	indices.push_back(d);
	indices.push_back(a);

}

void CreateVBO(void)
{
	//SIMPLE PRISM + STELLAR PLATE

	// CORE

	vector<point> vertices;
	makeHexagon(vertices, minHexRadius, origin);

	GLfloat Vertices[2 * 6 * 7 + 2 * 5 * 7];
	addVertices(Vertices, vertices, 2 * 6 * 7, 0, thickness);

	//SECTORED PLATES

	//PRISM

	vertices.clear();
	makePrism(vertices, d1, d2, d3, origin);
	addVertices(Vertices, vertices, 2 * 5 * 7, 2 * 6 * 7, thickness);

	// 10 dreptunghiuri * 2 triunghiuri each
	/*GLubyte Indices[10 * 3 * 2];
	vector<char> indices;*/

	GLubyte Indices[] =
	{
		//CORE

		//spate
		0, 1, 2,		2, 3, 0,
		3, 4, 5,		5, 0, 3,

		//lateral
		2, 3, 8,		8, 9, 3,
		3, 4, 9,		9, 10, 4,

		//bottom
		4, 5, 10,	10, 11, 5,

		//lateral
		1, 0, 6,		6, 7, 1,
		0, 5, 6,		6, 11, 5,

		//top
		1, 2, 7,		7, 8, 2,

		//fata
		6, 7, 8,		8, 9, 6,
		9, 10, 11,	11, 6, 9,


		//PRISM

		//spate
		12, 13, 16,		13, 14, 15,
		15, 16, 13,

		//lateral
		12, 13, 17,		17, 18, 13,
		13, 14, 18,		18, 19, 14,

		//bottom
		14, 15, 19,		19, 20, 15,

		//lateral
		16, 15, 20,		20, 21, 16,
		12, 16, 21,		21, 17, 12,

		//fata
		17, 18, 21,		18, 19, 20,
		20, 21, 18


	};


	// generare VAO/buffere
	glGenBuffers(1, &VboId); // atribute
	glGenBuffers(1, &EboId); // indici

	// legare+"incarcare" buffer
	glBindBuffer(GL_ARRAY_BUFFER, VboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW); // "copiere" in bufferul curent
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW); // "copiere" indici in bufferul curent

	// se activeaza lucrul cu atribute; 
	glEnableVertexAttribArray(0); // atributul 0 = pozitie
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1); // atributul 1 = culoare
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
}
void DestroyVBO(void)
{
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VboId);
	glDeleteBuffers(1, &EboId);
}
void CreateShaders(void)
{
	ProgramId = LoadShaders("vertShader.vert", "fragShader.frag");
	glUseProgram(ProgramId);
}
void DestroyShaders(void)
{
	glDeleteProgram(ProgramId);
}
void Initialize(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // culoarea de fond a ecranului

	srand(time(NULL));
	//randomize the radius of the base hexagon
	coreRadius = 5 + (rand() % 100);
	minHexRadius = coreRadius;

	d1 = 10 + (rand() % (int)(coreRadius >= 8 ? coreRadius / 8 : coreRadius/4));
	d2 = coreRadius + (rand() % 150);
	d3 = 5 + (rand() % (int)(1 + coreRadius / 4));

	// Creare VBO+shader
	CreateVBO();
	CreateShaders();

	// Locatii ptr shader
	viewLocation = glGetUniformLocation(ProgramId, "viewShader");
	projLocation = glGetUniformLocation(ProgramId, "projectionShader");
	codColLocation = glGetUniformLocation(ProgramId, "codCol");


	if (randomType == SIMPLE_PRISM || randomType == STELLAR_PLATE || randomType == STELLAR_DENDRITE || randomType == SECTORED_PLATES) {

		switch (randomType) {
		case SIMPLE_PRISM:
			noOfExtrusions = 0;
			break;
		case STELLAR_PLATE:
			noOfExtrusions = 1;
			break;
		case STELLAR_DENDRITE:
		case SECTORED_PLATES:
			noOfExtrusions = 7;
			break;
		default:
			break;
		}


		for (int ii = 0; ii < noOfExtrusions; ii++) {
			codCol = 1;
			glUniform1i(codColLocation, codCol);

			srand(time(NULL));
			if ((int)(minHexRadius / 4) > 0) {
				int ssize = minHexRadius / 4 + (rand() % (int)(minHexRadius / 4));
				minHexRadius = ssize;
				stellarDendriteSizes.push_back(ssize);
			}
			else {
				noOfExtrusions = ii;
				break;
			}
		}
	}

	if (randomType == FERN_DENDRITES) {
		srand(time(NULL));
		int rest = coreRadius;
		int ii = 0;
		while (rest < d2) {
			int dist = 10 + rand() % (int)((2+ (d2 - rest)) / 2);
			fernDendritesDist.push_back(rest);
			float ssize = 5 + rand() % 50;
			rest += dist;
			fernDendriteSize.push_back(ssize / 100.0f);
			float angle = (rand() % (int)(2 * PI * 1e3)) * 1.0f/1e3;
			fernDendritesAngle.push_back(angle);
		}
	}

}

void createExtrusions(int level, point origin, float hexRadius, float prevRadius, vector<transform>& trfVector) {
	float scale = hexRadius / coreRadius;
	float rotAngle = 0;
	vector<point> verticesOffset;
	makeHexagon(verticesOffset, prevRadius, origin);
	for (int ij = 0; ij < 6; ij++) {
		point crtOffset = verticesOffset[ij];
		transform trf;

		trf.translate = crtOffset;
		trf.scale = scale;
		trf.rotationAngle = rotAngle;
		trfVector.push_back(trf);

		rotAngle += PI / 3 / level;

		if (level < noOfExtrusions) {
			createExtrusions(level + 1, crtOffset, stellarDendriteSizes[level], hexRadius, trfVector);
		}
	}
}

void createFernExtrusions(point origin, float alpha, float rotAngle, vector<transform>& trfVector) {
	for (int ii = 0; ii < fernDendriteSize.size(); ii++) {
		transform trfL, trfR;

		/*

			|\
			| \
		 a |  \	c
			|   \
			|____\alpha
				b
		*/


		float c = fernDendritesDist[ii];
		float a = c * cos(alpha);
		float b = c * sin(alpha);
		point transl(a, b, origin.z);
		trfL.translate = transl;
		trfL.scale = fernDendriteSize[ii];
		if(rotAngle == 0)
			trfL.rotationAngle = alpha - fernDendritesAngle[ii];
		else
			trfL.rotationAngle = alpha - (PI - fernDendritesAngle[ii]);
		trfVector.push_back(trfL);
	}
}

void RenderFunction(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glBindVertexArray(VaoId);
	glBindBuffer(GL_ARRAY_BUFFER, VboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);

	//pozitia observatorului
	Obsx = Refx + dist * cos(alpha) * cos(beta);
	Obsy = Refy + dist * cos(alpha) * sin(beta);
	Obsz = Refz + dist * sin(alpha);

	// reperul de vizualizare
	glm::vec3 Obs = glm::vec3(Obsx, Obsy, Obsz);   // se schimba pozitia observatorului	
	glm::vec3 PctRef = glm::vec3(Refx, Refy, Refz); // pozitia punctului de referinta
	glm::vec3 Vert = glm::vec3(Vx, Vy, Vz); // verticala din planul de vizualizare 
	view = glm::lookAt(Obs, PctRef, Vert);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

	// matricea de proiectie, pot fi testate si alte variante
	projection = glm::infinitePerspective(fov, GLfloat(width) / GLfloat(height), znear);
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, &projection[0][0]);

	// Fetele
	codCol = 0;
	glUniform1i(codColLocation, codCol);

	vector<transform> trfArray;

	//sectoring
	if (randomType == SECTORED_PLATES || randomType == FERN_DENDRITES) {

		codCol = 0;
		glUniform1i(codColLocation, codCol);

		float rotAngle = -PI / 2;

		for (int ij = 0; ij < 6; ij++) {
			codCol = 0;
			glUniform1i(codColLocation, codCol);

			glm::mat4 rotMatrix;
			if (randomType == FERN_DENDRITES)
				rotMatrix = glm::rotate(glm::mat4(1.0f), (rotAngle + PI / 2), glm::vec3(0.0, 0.0, 1.0));
			else
				rotMatrix = glm::rotate(glm::mat4(1.0f), (rotAngle), glm::vec3(0.0, 0.0, 1.0));


			view = glm::lookAt(Obs, PctRef, Vert) /** matrTransl * resizeMatrix*/ * rotMatrix;
			glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

			glDrawElements(GL_TRIANGLES, 48, GL_UNSIGNED_BYTE, (void*)(60));
			glEnd();

			if (randomType == FERN_DENDRITES) {

				trfArray.clear();
				createFernExtrusions(origin, rotAngle, 0 /*(rotAngle - PI / 6)*/, trfArray);
				for (int ii = 0; ii < trfArray.size(); ii++) {
					codCol = 0;
					glUniform1i(codColLocation, codCol);
					transform crtTrf = trfArray[ii];
					glm::mat4 matrTransl = glm::translate(glm::mat4(1.0f), glm::vec3(crtTrf.translate.x, crtTrf.translate.y, 0.0f));
					glm::mat4 resizeMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(crtTrf.scale, crtTrf.scale, 0.75f));
					glm::mat4 rotMatrix = glm::rotate(glm::mat4(1.0f), crtTrf.rotationAngle, glm::vec3(0.0, 0.0, 1.0));

					view = glm::lookAt(Obs, PctRef, Vert) * matrTransl * rotMatrix * resizeMatrix;
					glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

					glDrawElements(GL_TRIANGLES, 48, GL_UNSIGNED_BYTE, (void*)(60));
					glEnd();
				}

				trfArray.clear();
				createFernExtrusions(origin, rotAngle, 1/*(rotAngle - 5 * PI / 6)*/, trfArray);
				for (int ii = 0; ii < trfArray.size(); ii++) {
					codCol = 0;
					glUniform1i(codColLocation, codCol);
					transform crtTrf = trfArray[ii];
					glm::mat4 matrTransl = glm::translate(glm::mat4(1.0f), glm::vec3(crtTrf.translate.x, crtTrf.translate.y, 0.0f));
					glm::mat4 resizeMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(crtTrf.scale, crtTrf.scale, 0.75f));
					glm::mat4 rotMatrix = glm::rotate(glm::mat4(1.0f), crtTrf.rotationAngle, glm::vec3(0.0, 0.0, 1.0));

					view = glm::lookAt(Obs, PctRef, Vert) * matrTransl * rotMatrix * resizeMatrix;
					glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

					glDrawElements(GL_TRIANGLES, 48, GL_UNSIGNED_BYTE, (void*)(60));
					glEnd();
				}
			}

			rotAngle += PI / 3;
		}
	}

	if (randomType == SIMPLE_PRISM || randomType == STELLAR_PLATE || randomType == STELLAR_DENDRITE || randomType == SECTORED_PLATES) {
		//float radiusOffset = coreRadius;

		codCol = 0;
		glUniform1i(codColLocation, codCol);

		//base
		glm::mat4 rotMatrix = glm::rotate(glm::mat4(1.0f), PI / 3, glm::vec3(0.0, 0.0, 1.0));
		view = glm::lookAt(Obs, PctRef, Vert) * rotMatrix;
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
		glDrawElements(GL_TRIANGLES, 60, GL_UNSIGNED_BYTE, 0);

		trfArray.clear();

		if (randomType == STELLAR_DENDRITE || randomType == STELLAR_PLATE) {
			createExtrusions(1, origin, stellarDendriteSizes[0], coreRadius, trfArray);
		}

		if (randomType == SECTORED_PLATES) {
			createExtrusions(1, origin, stellarDendriteSizes[0], d2, trfArray);
		}

		for (int ii = 0; ii < trfArray.size(); ii++) {
			transform crtTrf = trfArray[ii];
			glm::mat4 matrTransl = glm::translate(glm::mat4(1.0f), glm::vec3(crtTrf.translate.x, crtTrf.translate.y, 0.0f));
			glm::mat4 resizeMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(crtTrf.scale, crtTrf.scale, 0.75f));
			glm::mat4 rotMatrix = glm::rotate(glm::mat4(1.0f), crtTrf.rotationAngle, glm::vec3(0.0, 0.0, 1.0));

			view = glm::lookAt(Obs, PctRef, Vert) * matrTransl * rotMatrix * resizeMatrix;
			glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

			glDrawElements(GL_TRIANGLES, 60, GL_UNSIGNED_BYTE, 0);
			glEnd();
		}

	}

	glutSwapBuffers();
	glFlush();
}
void Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
}
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1200, 900);
	glutCreateWindow("Generare fulg de zapada unic");
	glewInit();
	Initialize();
	glutDisplayFunc(RenderFunction);
	glutIdleFunc(RenderFunction);
	glutKeyboardFunc(processNormalKeys);
	glutSpecialFunc(processSpecialKeys);
	glutCloseFunc(Cleanup);
	glutMainLoop();
}

