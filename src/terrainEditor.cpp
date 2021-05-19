//| Name            | Akshay Sharma   |
//|---------------- | --------------- |
//| Student Number  | 7859678         |
//| Assignment      | Project         |
//| Course          | COMP 4490       |
//| Instructor      | John Braico     |
//
//
// This script contains the code for the terrain editor program

#include "common.h"
#include "utils.h"
#include "lookup.h"
#include "SimplexNoise.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

const char *WINDOW_TITLE = "Terrain Editor CPU";
const double FRAME_RATE_MS = 1000.0/60.0;

typedef glm::vec4  color4;
typedef glm::vec4  point4;

int currentCube = 0;
int octaves = 1;


Cube *c = new Cube();



const int NUM_DIVISIONS = 13;
GLuint VAOs[5];


bool showPoints = false;
float frequency = 1;
float sharpness = 5;

// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Xaxis;
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };

// Model-view and projection matrices uniform location
GLuint  programP, programC, programM;
GLuint  ModelViewP, ProjectionP, ColorP, drawModeP;
GLuint  ModelViewC, ProjectionC, ColorC, drawModeC;
GLuint  ModelViewM, ProjectionM, ColorM, LightPosM, ModelViewInverseTransposeM, drawModeM;

std::vector<point4> gridPoints;
std::vector<Point*> gridPointStates;

glm::mat4 model_view;

float scaleFact = 1.0f;
float tx = 0;
float ty = 0;
float tz = 0;

float position[3] = { 0.0 ,0.0 ,0.0 };

float posZ = 256.0f;

point4 cubeOrigin = point4(0, 0, 0, 1);
float cubeWidth = 0.1f;
bool drawMode = false;
bool eraser = false;

//----------------------------------------------------------------------------

// quad generates two triangles for each face and assigns colors
//    to the vertices

int Index = 0;
bool spin = false;
int num = 0;
float intensity = 1;
std::vector<point4> drawCube;

void calculateCube(point4 origin, float width) {
    drawCube.clear();

    drawCube.push_back(point4(origin.x + width, origin.y + width, origin.z + width, 1));
    drawCube.push_back(point4(origin.x - width, origin.y + width, origin.z + width, 1));
    drawCube.push_back(point4(origin.x + width, origin.y - width, origin.z + width, 1));
    drawCube.push_back(point4(origin.x - width, origin.y - width, origin.z + width, 1));
    drawCube.push_back(point4(origin.x + width, origin.y + width, origin.z - width, 1));
    drawCube.push_back(point4(origin.x - width, origin.y + width, origin.z - width, 1));
    drawCube.push_back(point4(origin.x + width, origin.y - width, origin.z - width, 1));
    drawCube.push_back(point4(origin.x - width, origin.y - width, origin.z - width, 1));

}

GLuint cubeIndices[36] = {
    0, 1, 2,  1, 2, 3,
    5, 4, 6,  5, 6, 7,
    4, 0, 2,  4, 2, 6,
    1, 5, 7,  1, 7, 3,
    3, 2, 6,  3, 6, 7,
    1, 0, 4,  1, 4, 7
};

void calculateSubCubeVertices() {

    float inc = 2.0f / NUM_DIVISIONS;
    for (float i = -1.0; i <= 1.0f; i+=inc) {
        for (float j = -1.0; j <= 1.0f; j += inc) {
            for (float k = -1.0; k <= 1.0f; k += inc) {
                point4 *p = &point4(i, j, k, 1.0);
                gridPoints.push_back(*p);

                
                float density = -1; //trivial noise function

                if (density >= 0) {
                    gridPointStates.push_back(new Point(*p, true, density));
                }
                else {
                    gridPointStates.push_back(new Point(*p, false, density));
                }
            }
        }
    }

}

std::vector<point4> cube;
std::vector<point4> triangleMesh;
std::vector<point4> meshNormals;
float lightPos[3] = { 0.0, 0.0, -10000000000000.0 };
point4 originCube = point4(0);

void updateCube() {
    cube.clear();
    cube.push_back(*c->getArray()[0]->getPoint()); cube.push_back(*c->getArray()[1]->getPoint());
    cube.push_back(*c->getArray()[1]->getPoint()); cube.push_back(*c->getArray()[2]->getPoint());
    cube.push_back(*c->getArray()[2]->getPoint()); cube.push_back(*c->getArray()[3]->getPoint());
    cube.push_back(*c->getArray()[3]->getPoint()); cube.push_back(*c->getArray()[0]->getPoint());
    cube.push_back(*c->getArray()[4]->getPoint()); cube.push_back(*c->getArray()[5]->getPoint());
    cube.push_back(*c->getArray()[5]->getPoint()); cube.push_back(*c->getArray()[6]->getPoint());
    cube.push_back(*c->getArray()[6]->getPoint()); cube.push_back(*c->getArray()[7]->getPoint());
    cube.push_back(*c->getArray()[7]->getPoint()); cube.push_back(*c->getArray()[4]->getPoint());
    cube.push_back(*c->getArray()[0]->getPoint()); cube.push_back(*c->getArray()[4]->getPoint());
    cube.push_back(*c->getArray()[1]->getPoint()); cube.push_back(*c->getArray()[5]->getPoint());
    cube.push_back(*c->getArray()[2]->getPoint()); cube.push_back(*c->getArray()[6]->getPoint());
    cube.push_back(*c->getArray()[3]->getPoint()); cube.push_back(*c->getArray()[7]->getPoint());
}

//----------------------------------------------------------------------------
void calculateMesh() {
    int caseNum = c->getCaseNumber();

    const int* triangles = &triTable[caseNum][0];
    int i = 0;
    int edge;
    while (triangles[i] != -1) {
        triangleMesh.push_back(c->getBetweenPoint(triangles[i++]));
        if (triangles[i] == -1) {
            break;
        }
        triangleMesh.push_back(c->getBetweenPoint(triangles[i++]));
        if (triangles[i] == -1) { 
            break;
        }
        triangleMesh.push_back(c->getBetweenPoint(triangles[i++]));
        
        glm::vec3 v1 = glm::vec3(triangleMesh[triangleMesh.size() - 1] - triangleMesh[triangleMesh.size() - 2]);
        glm::vec3 v2 = glm::vec3(triangleMesh[triangleMesh.size() - 2] - triangleMesh[triangleMesh.size() - 3]);
        point4 normal = -glm::vec4(glm::normalize(glm::cross(v1, v2)), 1.0f);
        
        meshNormals.push_back(normal);
        meshNormals.push_back(normal);
        meshNormals.push_back(normal);
    }
}



// OpenGL initialization
void
init()
{
    calculateCube(cubeOrigin, cubeWidth);
    calculateSubCubeVertices();

    for (int i = 0; i < gridPoints.size() - 1; i++) {
        point4 p = gridPoints[i];
        if (p.x >= 0.9 || p.y >= 0.9 || p.z >= 0.9) {
            continue;
        }

        c->updatePoints(gridPointStates, i, NUM_DIVISIONS + 1);
        updateCube();
        calculateMesh();
    }

    // Create a vertex array object
    glGenVertexArrays( 5, VAOs );

    GLuint vPosition;

    GLuint buffer;
    
    programP = InitShader("vshader_proj.glsl", "fshader_proj.glsl");
    glUseProgram(programP);
    GLuint vPositionP = glGetAttribLocation(programP, "vPosition");
    glBindVertexArray(VAOs[1]);

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4)*gridPoints.size(),
        &gridPoints[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(vPositionP);
    glVertexAttribPointer(vPositionP, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(0));

    ModelViewP = glGetUniformLocation(programP, "ModelView");
    ProjectionP = glGetUniformLocation(programP, "Projection");
    ColorP = glGetUniformLocation(programP, "Color");
    drawModeP = glGetUniformLocation(programP, "Draw");

    programC = InitShader("vshader_proj.glsl", "fshader_proj.glsl");
    glUseProgram(programC);
    GLuint vPositionC = glGetAttribLocation(programC, "vPosition");
    glBindVertexArray(VAOs[2]);

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4)* cube.size(),
        &cube[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(vPositionC);
    glVertexAttribPointer(vPositionC, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(0));

    glBindVertexArray(VAOs[4]);

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4)* drawCube.size(),
        &drawCube[0], GL_STATIC_DRAW);

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(vPositionC);
    glVertexAttribPointer(vPositionC, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(0));

    ModelViewC = glGetUniformLocation(programC, "ModelView");
    ProjectionC = glGetUniformLocation(programC, "Projection");
    ColorC = glGetUniformLocation(programC, "Color");
    drawModeC = glGetUniformLocation(programC, "Draw");

    programM = InitShader("vshader_projL.glsl", "fshader_projL.glsl");
    glUseProgram(programM);
    GLuint vPositionM = glGetAttribLocation(programM, "vPosition");
    glBindVertexArray(VAOs[3]);

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4)* (triangleMesh.size() + meshNormals.size()),
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4)* triangleMesh.size(), (triangleMesh.size() > 0) ? &triangleMesh[0] : NULL);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4)* triangleMesh.size(), sizeof(point4)* meshNormals.size(), (meshNormals.size() > 0) ? &meshNormals[0] : NULL);

    glEnableVertexAttribArray(vPositionM);
    glVertexAttribPointer(vPositionM, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(0));

    GLuint vNormal = glGetAttribLocation(programM, "vNormal");
    glEnableVertexAttribArray(vNormal);
    glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(sizeof(point4) * meshNormals.size()));


    ModelViewM = glGetUniformLocation(programM, "ModelView");
    ProjectionM = glGetUniformLocation(programM, "Projection");
    ColorM = glGetUniformLocation(programM, "Color");
    LightPosM = glGetUniformLocation(programM, "LightPos");
    ModelViewInverseTransposeM = glGetUniformLocation(programM, "ModelViewInverseTranspose");
    drawModeM = glGetUniformLocation(programM, "Draw");
    glUniform3f(LightPosM, 0, 0, -1.0f);

    glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
    glEnable( GL_DEPTH_TEST );

    glShadeModel(GL_FLAT);

    glClearColor( 0.53, 0.81, 0.92, 1 ); 
    glPointSize( 3.0f );
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 
}

//----------------------------------------------------------------------------


void recalculateTerrain() {
    triangleMesh.clear();
    meshNormals.clear();


    for (int i = 0; i < gridPoints.size() - 1; i++) {
        point4 p = gridPoints[i];
        if (p.x >= 0.9 || p.y >= 0.9 || p.z >= 0.9) {
            continue;
        }

        c->updatePoints(gridPointStates, i, NUM_DIVISIONS + 1);
        calculateMesh();
    }
    
    glUseProgram(programM);
    GLuint vPositionM = glGetAttribLocation(programM, "vPosition");
    glBindVertexArray(VAOs[3]);

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4) * (triangleMesh.size() + meshNormals.size()),
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * triangleMesh.size(), (triangleMesh.size() > 0) ? &triangleMesh[0] : NULL);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * triangleMesh.size(), sizeof(point4) * meshNormals.size(), (meshNormals.size() > 0) ? &meshNormals[0] : NULL);

    glEnableVertexAttribArray(vPositionM);
    glVertexAttribPointer(vPositionM, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(0));

    GLuint vNormal = glGetAttribLocation(programM, "vNormal");
    glEnableVertexAttribArray(vNormal);
    glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(sizeof(point4) * meshNormals.size()));
}

void reBufferCube() {
    calculateCube(originCube, cubeWidth);
    glUseProgram(programC);
    GLuint buffer;
    GLuint vPositionC = glGetAttribLocation(programC, "vPosition");
    glBindVertexArray(VAOs[4]);
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4) * drawCube.size(),
        &drawCube[0], GL_STATIC_DRAW);

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(vPositionC);
    glVertexAttribPointer(vPositionC, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(0));
    Point* p;
    bool hit = false;
    if (drawMode) {
        for (int i = 0; i < gridPointStates.size(); i++) {
            p = gridPointStates[i];

            if (p->getPoint()->x > originCube.x - cubeWidth && p->getPoint()->x < originCube.x + cubeWidth && p->getPoint()->y > originCube.y - cubeWidth && p->getPoint()->y < originCube.y + cubeWidth && p->getPoint()->z > originCube.z - cubeWidth && p->getPoint()->z < originCube.z + cubeWidth) {
                (eraser)? gridPointStates[i]->setState(false): gridPointStates[i]->setState(true);
                (eraser)? gridPointStates[i]->setDensity(-1) : gridPointStates[i]->setDensity(1);
                hit = true;
            }
        }
        if (hit) {
            recalculateTerrain();
            hit = !hit;
        }
    }
}

void
display( void )
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //  Generate tha model-view matrixn

    const glm::vec3 viewer_pos( 0, 0.0, 0.0 );
    glm::mat4 trans, rot, scale;
    trans = glm::translate(trans, -viewer_pos);
    trans = glm::translate(trans, glm::vec3(tx, ty, tz));
    scale = glm::scale(scale, glm::vec3(scaleFact));
    rot = glm::rotate(rot, glm::radians(Theta[Xaxis]), glm::vec3(1,0,0));
    rot = glm::rotate(rot, glm::radians(Theta[Yaxis]), glm::vec3(0,1,0));
    rot = glm::rotate(rot, glm::radians(Theta[Zaxis]), glm::vec3(0,0,1));
    model_view = trans * rot * scale ;
    glm::mat4 model_view_no_scale = trans * rot;

    glUseProgram(programP);
    glBindVertexArray(VAOs[1]);
    glUniformMatrix4fv(ModelViewP, 1, GL_FALSE, glm::value_ptr(model_view));
    (drawMode) ? glUniform1i(drawModeP, 1) : glUniform1i(drawModeP, 0);
    glUniform3f(ColorP, 0.7, 0.0, 0.0);
    if (showPoints) {
        glDrawArrays(GL_POINTS, 0, gridPoints.size());
    }
    glUseProgram(programC);
    glBindVertexArray(VAOs[4]);
    glUniformMatrix4fv(ModelViewP, 1, GL_FALSE, glm::value_ptr(model_view));
    (eraser) ? glUniform3f(ColorC, 1.0, 0.0, 0.0) : glUniform3f(ColorC, 0.0, 1.0, 0.0);
    (drawMode) ? glUniform1i(drawModeC, 1) : glUniform1i(drawModeC, 0);
    glDrawElements(GL_TRIANGLES, sizeof(cubeIndices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
    glUniform3f(ColorC, 0.0, 0.0, 0.0);
    for (int i = 0; i < sizeof(cubeIndices) / sizeof(GLuint); i += 3) {
        glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, (void*)(i * sizeof(GLuint)));
    }

    glUseProgram(programM);
    glBindVertexArray(VAOs[3]);
    glUniformMatrix4fv(ModelViewM, 1, GL_FALSE, glm::value_ptr(model_view));
    glUniformMatrix4fv(ModelViewInverseTransposeM, 1, GL_FALSE,
        glm::value_ptr(glm::transpose(glm::inverse(model_view_no_scale))));
    glUniform3f(ColorM, 0.2, 0.7, 0.15);
    (drawMode) ? glUniform1i(drawModeM, 1) : glUniform1i(drawModeM, 0);
    glUniform3f(LightPosM, lightPos[0], lightPos[1], lightPos[2]);
    glDrawArrays(GL_TRIANGLES, 0, triangleMesh.size()-num);
    /*glUniform3f(ColorM, 0.0, 0.0, 0.0);
    glDrawArrays(GL_LINE_STRIP, 0, triangleMesh.size());*/

    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
mouse( int button, int state, int x, int y )
{
    if ( state == GLUT_DOWN ) {
        glm::vec4 point;
        point4 pointTransform;
        Point* p;
        bool hit = false;
        float brushWidth = 0.1f;
        switch( button ) {
          case GLUT_LEFT_BUTTON: 
              Axis = Yaxis;
              break;
          case GLUT_MIDDLE_BUTTON:  Axis = Zaxis;  break;
          case GLUT_RIGHT_BUTTON:   Axis = Xaxis;  break;
       }
    }
}

//----------------------------------------------------------------------------

void
update( void )
{
    if (spin) {
        Theta[Axis] += 0.5;
    }
    if ( Theta[Axis] > 360.0 ) {
       Theta[Axis] -= 360.0;
    }
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
	case 033: // Escape Key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;
    case ' ':
        spin = !spin;
        break;
    case '+':
        scaleFact += 0.05;
        break;
    case '-':
        scaleFact -= 0.05;
        break;
    case 'w':
        Theta[Xaxis] += 1;
        break;
    case 's':
        Theta[Xaxis] -= 1;
        break;
    case 'a':
        Theta[Yaxis] += 1;
        break;
    case 'd':
        Theta[Yaxis] -= 1;
        break;
    case 'e':
        Theta[Zaxis] += 1;
        break;
    case 'r':
        Theta[Zaxis] -= 1;
        break;
    case 'i':
        originCube.y += 0.02;
        reBufferCube();
        break;
    case 'k':
        originCube.y -= 0.02;
        reBufferCube();
        break;
    case 'j':
        originCube.x -= 0.02;
        reBufferCube();
        break;
    case 'l':
        originCube.x += 0.02;
        reBufferCube();
        break;
    case 'u':
        originCube.z -= 0.02;
        reBufferCube();
        break;
    case 'o':
        originCube.z += 0.02;
        reBufferCube();
        break;
    case 't':
        intensity += 0.1;
        recalculateTerrain();
        break;
    case 'y':
        intensity -= 0.1;
        recalculateTerrain();
        break;
    case 'c':
        drawMode = !drawMode;
        break;
    case 'v':
        eraser = !eraser;
        break;
    case 'g':
        frequency += 0.2;
        recalculateTerrain();
        break;
    case 'f':
        frequency -= 0.2;
        recalculateTerrain();
        break;
    case '[':
        sharpness -= 0.1;
        std::cout << "sharpness: " << sharpness << std::endl;
        recalculateTerrain();
        break;
    case ']':
        sharpness += 0.1;
        std::cout << "sharpness: " << sharpness << std::endl;
        recalculateTerrain();
        break;
    case 'z':
        posZ -= 12;
        break;
    case 'x':
        posZ+= 12;
        break;
    case '1':
        showPoints=!showPoints;
        break;
    case '8':
        position[2] += 0.1;
        recalculateTerrain();
        break;
    case '2':
        position[2] -= 0.1;
        recalculateTerrain();
        break;
    case '4':
        position[0] -= 0.1;
        recalculateTerrain();
        break;
    case '6':
        position[0] += 0.1;
        recalculateTerrain();
        break;
    case '7':
        position[1] -= 0.1;
        recalculateTerrain();
        break;
    case '9':
        position[1] += 0.1;
        recalculateTerrain();
        break;
    case 'n':
        currentCube++;
        c->updatePoints(gridPointStates, currentCube, NUM_DIVISIONS + 1);
        updateCube();
        GLuint buffer;
        glUseProgram(programC);
        GLuint vPositionC = glGetAttribLocation(programC, "vPosition");
        glBindVertexArray(VAOs[2]);

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(point4) * cube.size(),
            &cube[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(vPositionC);
        glVertexAttribPointer(vPositionC, 4, GL_FLOAT, GL_FALSE, 0,
            BUFFER_OFFSET(0));
        break;
    }
}

void drag(int x, int y) {

}

//----------------------------------------------------------------------------

void
reshape( int width, int height )
{
    glViewport( 0, 0, width, height );

    GLfloat aspect = GLfloat(width)/height;
    glm::mat4  projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -2.0f, 2.0f);;

    glUseProgram(programP);
    glUniformMatrix4fv(ProjectionP, 1, GL_FALSE, glm::value_ptr(projection));

    glUseProgram(programC);
    glUniformMatrix4fv(ProjectionC, 1, GL_FALSE, glm::value_ptr(projection));

    glUseProgram(programM);
    glUniformMatrix4fv(ProjectionM, 1, GL_FALSE, glm::value_ptr(projection));
}
