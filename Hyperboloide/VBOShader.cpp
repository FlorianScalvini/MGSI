/********************************************************/
/*                     HyperboloideVBOShader.cpp                         */
/********************************************************/
/* Premiers pas avec OpenGL.                            */
/* Objectif : afficher a l'ecran un objet VBO avec ou shader    */
/* L'objet sera soit un hyperboloïde, soit une sphère soit un tore */
/********************************************************/
/* inclusion des fichiers d'en-tete Glut */
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>
#include "shader.hpp"
#include <string.h>

// Include GLM
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"
using namespace glm;
using namespace std;

#define MAXMERID 20
#define BUFFER_OFFSET(i) ((unsigned int *)NULL + (i))

GLfloat sommets[2*MAXMERID*3+2*3] ; // x 3 coordonnées avec 2 poles
GLuint indices[MAXMERID*4 + MAXMERID*3 + MAXMERID*3]; // pour QUads et Triangles
GLfloat normales[2*MAXMERID*3+2*3];

// initialisations

void genereVBO();
void deleteVBO();
void traceObjet();

// fonctions de rappel de glut
void affichage();
void clavier(unsigned char,int,int);
void mouse(int, int, int, int);
void mouseMotion(int, int);
void reshape(int,int);
void createGLUTMenus();
// misc
void drawString(const char *str, int x, int y, float color[4], void *font);
void showInfo();
void *font = GLUT_BITMAP_8_BY_13; // pour afficher des textes 2D sur l'ecran
// variables globales pour OpenGL
bool mouseLeftDown;
bool mouseRightDown;
bool mouseMiddleDown;
float mouseX, mouseY;
float cameraAngleX;
float cameraAngleY;
float cameraDistance=0.;
float angle=0; // Angle hyperboloide
int theta= 0; // Angle de rotation Maximal Vortex - Twister
int status = 0;
int rotationChoix = 1; // Choix rotation
float deformation = 1.5; // Deformation du cylindre en hérisson. = 1 : Cylindre parfait.
vec2 deforInter = vec2(-0.2,0.2);
// variables Handle d'opengl
//--------------------------
GLuint programID;   // handle pour le shader
GLuint MatrixIDMVP,MatrixIDView,MatrixIDModel,MatrixIDPerspective;    // handle pour la matrice MVP
GLuint VBO_sommets,VBO_normales, VBO_indices,VAO;
GLuint locCameraPosition ;

// location des VBO
//------------------
GLuint indexVertex=0,indexNormale=3 ;
//variable pour paramétrage éclairage
//--------------------------------------
vec3 cameraPosition(0.,0.,3.);
// le matériau
//---------------
GLfloat materialShininess=3.;
vec3 materialSpecularColor(1.,.1,1);  // couleur du materiau

// la lumière
//-----------
vec3 LightPosition(1.,0.,.5);
vec3 LightIntensities(1.,1.,1.);// couleur la lumiere
GLfloat LightAttenuation =1.;
GLfloat LightAmbientCoefficient=.1;
// MAtrices de transfo
glm::mat4 MVP;      // justement la voilà
glm::mat4 Model, View, Projection;    // Matrices constituant MVP

int screenHeight = 500;
int screenWidth = 500;

GLuint locAngle;
GLuint locDef;
GLuint locMaxmer;
GLuint locStatus;
GLuint locNormal;
GLuint locTheta;
GLuint locDeforInt;
GLuint locRotationChoix;

//-------------------------
void createObjet()
{
	/* METTRE ICI LA CONSTITUTION DU TABLEAU DES SOMMETS,
			DES NORMALES,
			DES COULEURS
			ET DU TABLEAU DES INDICES
	POUR L'HYPERBOLOIDE, LA SPHERE , LE TORE, ....
 */
float theta = 0.;
   for(int i=0; i<MAXMERID;i++)
    {
        /* Points du haut */
        sommets[i*3] = sin(theta);
        sommets[i*3+1] = 1;
        sommets[i*3+2] = cos(theta);

        /* Points du bas */
        sommets[i*3+MAXMERID*3] = sin(theta);
        sommets[i*3+MAXMERID*3+1] = -1;
        sommets[i*3+MAXMERID*3+2] = cos(theta);

        /* On incremente theta
        Selon le nombre de faces du contour de l hyperboloide */
        theta += 2*M_PI/MAXMERID;
    }


    /* Centre des couvercles */
    // Nord
    sommets[MAXMERID*3*2] = 0;
    sommets[MAXMERID*3*2+1] = 1;
    sommets[MAXMERID*3*2+2] = 0;

    // Sud
    sommets[MAXMERID*3*2+3] = 0;
    sommets[MAXMERID*3*2+4] = -1;
    sommets[MAXMERID*3*2+5] = 0;


    /// INDICES POUR TRIANGLES + QUADS
    // Tour faces quadragulaires
    for(int i=0; i<MAXMERID ; i++)
    {
        indices[i*4] = i;
        indices[i*4+1] = i+MAXMERID;
        indices[i*4+2] = (i+1)%MAXMERID + MAXMERID;
        indices[i*4+3] = (i+1)%MAXMERID;
    }

     // Tableau des indices du dessus
    for(int i=0 ; i<MAXMERID ; i++)
    {
        indices[MAXMERID*4 + i*3] = i;
        indices[MAXMERID*4 + i*3+1] = (i+1)%MAXMERID;
        indices[MAXMERID*4 + i*3+2] = MAXMERID*2;
    }

    // Tableau des indices du dessous
    for(int i=0 ; i<MAXMERID ; i++)
    {
        indices[MAXMERID*4 + MAXMERID*3 + i*3] = i+MAXMERID;
        indices[MAXMERID*4 + MAXMERID*3 + i*3+1] = MAXMERID*2+1;
        indices[MAXMERID*4 + MAXMERID*3 + i*3+2] = (i+1)%(MAXMERID)+MAXMERID;
    }



}

//----------------------------------------
void initOpenGL(void)
//----------------------------------------
{
  glCullFace (GL_BACK); // on spécifie queil faut éliminer les face arriere
  glEnable(GL_CULL_FACE); // on active l'élimination des faces qui par défaut n'est pas active
  glEnable(GL_DEPTH_TEST);
// le shader
   programID = LoadShaders( "DebutShader.vert", "DebutShader.frag" );

   // Get  handles for our matrix transformations "MVP" VIEW  MODELuniform
  MatrixIDMVP = glGetUniformLocation(programID, "MVP");
  locAngle = glGetUniformLocation(programID, "angle");
  locMaxmer = glGetUniformLocation(programID, "MAXMERID");
  locDef = glGetUniformLocation(programID, "deformation");
  locStatus = glGetUniformLocation(programID, "status");
  locTheta = glGetUniformLocation(programID, "thetaMax");
  locDeforInt = glGetUniformLocation(programID, "deforInter");
  locRotationChoix = glGetUniformLocation(programID, "rotationChoix");
//  MatrixIDView = glGetUniformLocation(programID, "VIEW");
 // MatrixIDModel = glGetUniformLocation(programID, "MODEL");
 // MatrixIDPerspective = glGetUniformLocation(programID, "PERSPECTIVE");

  // Projection matrix : 65 Field of View, 1:1 ratio, display range : 1 unit <-> 1000 units
  // ATTENTIOn l'angle est donné en radians si f GLM_FORCE_RADIANS est défini sinon en degré
  Projection = glm::perspective( glm::radians(60.f), 1.0f, 1.0f, 1000.0f);

/* on recupere l'ID */
locCameraPosition = glGetUniformLocation(programID, "cameraPosition");

}
//----------------------------------------
int main(int argc,char **argv)
//----------------------------------------
{

  /* initialisation de glut et creation
     de la fenetre */

  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE|GLUT_RGB);
  glutInitWindowPosition(200,200);
  glutInitWindowSize(screenWidth,screenHeight);
  glutCreateWindow("Hyperboloide VBO SHADER ");


// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

//info version GLSL
    std::cout << "***** Info GPU *****" << std::endl;
    std::cout << "Fabricant : " << glGetString (GL_VENDOR) << std::endl;
    std::cout << "Carte graphique: " << glGetString (GL_RENDERER) << std::endl;
    std::cout << "Version : " << glGetString (GL_VERSION) << std::endl;
    std::cout << "Version GLSL : " << glGetString (GL_SHADING_LANGUAGE_VERSION) << std::endl << std::endl;

	initOpenGL();


   createObjet();

 // construction des VBO a partir des tableaux de l'objet deja construit
  genereVBO();


  /* enregistrement des fonctions de rappel */
  glutDisplayFunc(affichage);
  glutKeyboardFunc(clavier);
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
  createGLUTMenus();
  glutMotionFunc(mouseMotion);

  /* Entree dans la boucle principale glut */
  glutMainLoop();

  glDeleteProgram(programID);
  deleteVBO();
  return 0;
}

void genereVBO ()
{
    glGenBuffers(1, &VAO);
    glBindVertexArray(VAO); // ici on bind le VAO , c'est lui qui recupèrera les configurations des VBO glVertexAttribPointer , glEnableVertexAttribArray...


    if(glIsBuffer(VBO_sommets) == GL_TRUE) glDeleteBuffers(1, &VBO_sommets);
    glGenBuffers(1, &VBO_sommets);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_sommets);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sommets),sommets , GL_STATIC_DRAW);
    glVertexAttribPointer ( indexVertex, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

    if(glIsBuffer(VBO_normales) == GL_TRUE) glDeleteBuffers(1, &VBO_normales);
    glGenBuffers(1, &VBO_normales);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_normales);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normales),normales , GL_STATIC_DRAW);
    glVertexAttribPointer ( indexNormale, 3, GL_FLOAT, GL_FALSE, 0, (void*)0  );

    if(glIsBuffer(VBO_indices) == GL_TRUE) glDeleteBuffers(1, &VBO_indices);
    glGenBuffers(1, &VBO_indices); // ATTENTIOn IBO doit etre un GL_ELEMENT_ARRAY_BUFFER
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO_indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices),indices , GL_STATIC_DRAW);

   glEnableVertexAttribArray(indexVertex);
   glEnableVertexAttribArray(indexNormale );



// une fois la config terminée
   // on désactive le dernier VBO et le VAO pour qu'ils ne soit pas accidentellement modifié
 glBindBuffer(GL_ARRAY_BUFFER, 0);
 glBindVertexArray(0);

}
//-----------------
void deleteVBO ()
//-----------------
{
    glDeleteBuffers(1, &VBO_sommets);
   glDeleteBuffers(1, &VBO_normales);
    glDeleteBuffers(1, &VBO_indices);
        glDeleteBuffers(1, &VAO);
}


/* fonction d'affichage */
void affichage()
{

  /* effacement de l'image avec la couleur de fond */
 /* Initialisation d'OpenGL */
  glClearColor(0.0,0.0,0.0,0.0);
  glClearDepth(10.0f);                         // 0 is near, >0 is far
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glColor3f(1.0,1.0,1.0);
  glPointSize(2.0);

     View       = glm::lookAt(   cameraPosition, // Camera is at (0,0,3), in World Space
                                            glm::vec3(0,0,0), // and looks at the origin
                                            glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                                             );
     Model = glm::mat4(1.0f);
     Model = glm::translate(Model,glm::vec3(0,0,cameraDistance));
     Model = glm::rotate(Model,glm::radians(cameraAngleX),glm::vec3(1, 0, 0) );
     Model = glm::rotate(Model,glm::radians(cameraAngleY),glm::vec3(0, 1, 0) );
     Model = glm::scale(Model,glm::vec3(.8, .8, .8));
     MVP = Projection * View * Model;
     traceObjet();        // trace VBO avec ou sans shader

 /* on force l'affichage du resultat */
   glutPostRedisplay();
   glutSwapBuffers();
}




//-------------------------------------
//Trace l'objet via le VAO
void traceObjet()
//-------------------------------------
{
 // Use  shader & MVP matrix   MVP = Projection * View * Model;
 glUseProgram(programID);

//on envoie les données necessaires aux shaders */
 glUniformMatrix4fv(MatrixIDMVP, 1, GL_FALSE, &MVP[0][0]);
 //glUniformMatrix4fv(MatrixIDView, 1, GL_FALSE,&View[0][0]);
 //glUniformMatrix4fv(MatrixIDModel, 1, GL_FALSE, &Model[0][0]);
 //glUniformMatrix4fv(MatrixIDPerspective, 1, GL_FALSE, &Projection[0][0]);
 glUniform3f(locCameraPosition,cameraPosition.x, cameraPosition.y, cameraPosition.z);
 glUniform1f(locAngle, angle);
 glUniform1i(locMaxmer, MAXMERID);
 glUniform1i(locRotationChoix, rotationChoix);
 glUniform1i(locStatus, status);
 glUniform1f(locDef, deformation);
 glUniform1f(locTheta, theta*(M_PI)/180);
 glUniform2f(locDeforInt, deforInter.x,deforInter.y);

//pour l'affichage
	glBindVertexArray(VAO); // on active le VAO
            glDrawElements(GL_QUADS, MAXMERID*4, GL_UNSIGNED_INT,BUFFER_OFFSET(0));
            glDrawElements(GL_TRIANGLES, MAXMERID*3*2, GL_UNSIGNED_INT,BUFFER_OFFSET(MAXMERID*4));
	glBindVertexArray(0);    // on desactive les VAO
  glUseProgram(0);         // et le pg

}

void reshape(int w, int h)
{
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);// ATTENTION GLsizei important - indique qu'il faut convertir en entier non négatif

    // set perspective viewing frustum
    float aspectRatio = (float)w / h;

    Projection = glm::perspective(glm::radians(60.0f),(float)(w)/(float)h, 1.0f, 1000.0f);
}

// Action du menu

void processMenuEvents(int option) {
    switch (option) {
        case 1: angle += 10;break;
        case 2: angle -= 10;break;
        case 3: deformation += 0.1; break;
        case 4: deformation -= 0.1; break;
        case 5: theta += 10; break;
        case 6: theta += 10; break;
        case 7: if (deforInter.x < deforInter.y ) deforInter.x += 0.1; break;
        case 8: if (deforInter.x > -deformation) deforInter.x -= 0.1; break;
        case 9: if(deformation > deforInter.y) deforInter.y += 0.1; break;
        case 10: if(deforInter.y > deforInter.x) deforInter.y -= 0.1; break;
        case 11: rotationChoix = 1;break;
        case 12: rotationChoix = 2;break;
        case 13: rotationChoix = 3;break;
        case 14: status = 0;break;
        case 15: status = 1; break;
        case 16: status = 2; break;
        case 17: status = 4; break;
        case 18: exit(0); break;
    }
    glutPostRedisplay();
}

// Creation Menu

void createGLUTMenus() {
    
    int menu, sub1, sub2, sub3;

    // create the menu and
    // tell glut that "processMenuEvents" will
    // handle the events

    sub3 = glutCreateMenu(processMenuEvents);
    glutAddMenuEntry("Augmenter X-Y-Z0 ",7);
    glutAddMenuEntry("Diminuer X-Y-Z0",8);
    glutAddMenuEntry("Augmenter X-Y-Z1",9);
    glutAddMenuEntry("Diminuer X-Y-Z1",10);

    sub2 = glutCreateMenu(processMenuEvents);
    glutAddMenuEntry("Rotation X",11);
    glutAddMenuEntry("Rotation Y",12);
    glutAddMenuEntry("Rotation Z",13);


    sub1 = glutCreateMenu(processMenuEvents);
    glutAddMenuEntry("None",14);
    glutAddMenuEntry("Taper",15);
    glutAddMenuEntry("Twister",16);
    glutAddMenuEntry("Vortex",17);

    menu = glutCreateMenu(processMenuEvents);

    //add entries to our menu
    glutAddMenuEntry("Augmenter Angle hyperboloide",1);
    glutAddMenuEntry("Diminuer Angle hyperboloide",2);
    glutAddMenuEntry("Augmenter deplacement herisson",3);
    glutAddMenuEntry("Diminuer deplacement herisson",4);
    glutAddMenuEntry("Augmenter Angle theta Max déformation",5);
    glutAddMenuEntry("Diminuer Angle theta Max déformation",6);
    glutAddSubMenu("Deformation", sub2);
    glutAddSubMenu("Intervalle deformation",sub3);
    glutAddSubMenu("Rotation", sub1);
    glutAddMenuEntry("Quit", 18);
    // attach the menu to the right button
    glutAttachMenu(GLUT_MIDDLE_BUTTON);
}

// Action clavier en doublons du menu 

void clavier(unsigned char touche,int x,int y)
{
  switch (touche)
    {
    case 'f': /* affichage du carre plein */
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      glutPostRedisplay();
      break;
    case 'e': /* affichage en mode fil de fer */
      glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
      glutPostRedisplay();
      break;
    case 'v' : /* Affichage en mode sommets seuls */
      glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
      glutPostRedisplay();
      break;
    case 'A': /* Augmenter angle */
      angle = angle+10;
      glutPostRedisplay();
      break;
    case 'a': /* Diminuer angle */
      angle = angle-10;
      glutPostRedisplay();
      break;
    case 'D':
        deformation += 0.1;
        glutPostRedisplay();
        break;
    case 'd':
        if(deformation > 0) deformation -= 0.1;
        glutPostRedisplay();
        break;
    case 'T':
        theta += 10;
        glutPostRedisplay();
        break;
    case 't':
        theta -= 10;
        glutPostRedisplay();
        break;
    case 'Z':
        if(status < 4) status += 1;
        else status = 0;
        glutPostRedisplay();
        break;
    case 'z':
        if(status > 0) status -= 1;
        else status = 4;
        glutPostRedisplay();
        break;
    case 'R':
        if(rotationChoix < 3) rotationChoix += 1;
        else rotationChoix = 1;
        glutPostRedisplay();
        break;
    case 'r':
        if (rotationChoix > 1) rotationChoix -= 1;
        else rotationChoix = 3;
        glutPostRedisplay();
        break;
    case 'U':
        if (deforInter.x < deforInter.y ) deforInter.x += 0.1;
        glutPostRedisplay();
        break;
    case 'u':
        if (deforInter.x > -deformation) deforInter.x -= 0.1;
        glutPostRedisplay();
        break;
    case 'I':
        if(deformation > deforInter.y) deforInter.y += 0.1;
        glutPostRedisplay();
        break;
    case 'i':
        if(deforInter.y > deforInter.x) deforInter.y -= 0.1;
        glutPostRedisplay();
        break;
                

    case 'q' : /*la touche 'q' permet de quitter le programme */
      exit(0);
    }
}



void mouse(int button, int state, int x, int y)
{
    mouseX = x;
    mouseY = y;

    if(button == GLUT_LEFT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseLeftDown = true;
        }
        else if(state == GLUT_UP)
            mouseLeftDown = false;
    }

    else if(button == GLUT_RIGHT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseRightDown = true;
        }
        else if(state == GLUT_UP)
            mouseRightDown = false;
    }

    else if(button == GLUT_MIDDLE_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseMiddleDown = true;
        }
        else if(state == GLUT_UP)
            mouseMiddleDown = false;
    }
}


void mouseMotion(int x, int y)
{
    if(mouseLeftDown)
    {
        cameraAngleY += (x - mouseX);
        cameraAngleX += (y - mouseY);
        mouseX = x;
        mouseY = y;
    }
    if(mouseRightDown)
    {
        cameraDistance += (y - mouseY) * 0.2f;
        mouseY = y;
    }

    glutPostRedisplay();
}
