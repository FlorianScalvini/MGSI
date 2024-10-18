/********************************************************/
/*                     CubeVBOShader.cpp                         */
/********************************************************/
/* Premiers pas avec OpenGL.                            */
/* Objectif : afficher a l'ecran uncube avec ou sans shader    */
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
//#include "../utilstexture/sdlglutils.h"

// Include GLM
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"
using namespace glm;
using namespace std;

#define NB_R 40
#define NB_r 20
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

GLfloat sommets[(NB_R+1)*(NB_r+1)*3] ; // x 3 coordonnées (+1 acr on double les dernierspoints pour avoir des coord de textures <> pour les points de jonctions)
GLuint indices[NB_R*NB_r*6]; // x6 car pour chaque face quadrangulaire on a 6 indices (2 triangles=2x 3 indices)

GLfloat coordTexture[(NB_R+1)*(NB_r+1)*2] ; // x 2 car U+V par sommets

GLfloat normales[(NB_R+1)*(NB_r+1)*3];
GLfloat bitangente[(NB_R+1)*(NB_r+1)*3];
GLfloat tangente[(NB_R+1)*(NB_r+1)*3];


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

// variables Handle d'opengl 
//--------------------------
GLuint programID;   // handle pour le shader
GLuint MatrixIDMVP,MatrixIDView,MatrixIDModel,MatrixIDPerspective;    // handle pour la matrice MVP
GLuint VBO_sommets,VBO_normales, VBO_indices,VBO_UVtext,VBO_bitangente, VBO_tangente,VAO;
GLuint locCameraPosition;
GLuint locAmbient; 
GLuint locLightPos;
GLuint locMatShininess;
GLuint locAttenuation;
/*
declaration des variables utilisées pour les handles
*/

// location des VBO
//------------------
GLuint indexVertex=0, indexUVTexture=2, indexNormale=3, indexTangente = 4, indexBitangente = 5;

//variable pour paramétrage eclairage
//--------------------------------------
vec3 cameraPosition(0.,0.,3.);
vec3 attenuation(1,0.1,0.01);
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

glm::mat4 MVP;      // Matrice MVP
glm::mat4 Model, View, Projection;    // Matrices constituant MVP



int screenHeight = 500;
int screenWidth = 500;

// pour la texcture
//-------------------
GLuint image ;
GLuint bufTexture,bufNormalMap;
GLuint locationTexture,locationNormalMap;
//-------------------------
void createTorus(float R, float r )
{
	float theta, phi;
	theta = ((float)radians(360.f))/((float)NB_R);
	phi = ((float)(radians(360.f)))/((float)NB_r);

	float pasU, pasV;
pasU= 1./NB_R;
pasV= 1./NB_r;
for (int i =0;i<=NB_R;i++ )
for (int j =0;j<=NB_r;j++ )
 {
float a,b,c;
    sommets[(i*(NB_r+1)*3)+ (j*3)] =   (R+r*cos((float)j*phi)) * cos((float)i*theta)    ;//x
    sommets[(i*(NB_r+1)*3)+ (j*3)+1] =  (R+r*cos((float)j*phi)) * sin((float)i*theta)  ;//y
    sommets[(i*(NB_r+1)*3)+ (j*3)+2] =  r*sin((float)j*phi)  ;

    normales[(i*(NB_r+1)*3)+ (j*3)] =   cos((float)j*phi)*cos((float)i*theta)    ;//x
    normales[(i*(NB_r+1)*3)+ (j*3)+1] = cos((float)j*phi)* sin((float)i*theta)  ;//y
    normales[(i*(NB_r+1)*3)+ (j*3)+2] =  sin((float)j*phi)  ;

    coordTexture[(i*(NB_r+1)*2)+ (j*2)]= ((float)i)*pasU;
    coordTexture[(i*(NB_r+1)*2)+ (j*2)+1]= ((float)j)*pasV;
     
    tangente[(i*(NB_r+1)*3)+ (j*3)] =   -sommets[(i*(NB_r+1)*3)+ (j*3)+1];//x
    tangente[(i*(NB_r+1)*3)+ (j*3)+1] = sommets[(i*(NB_r+1)*3)+ (j*3)];//Y
    tangente[(i*(NB_r+1)*3)+ (j*3)+2] =  0.0;
     
    bitangente[(i*(NB_r+1)*3)+ (j*3)] =  -cos((float)i*theta)*(-sin((float)j*phi))  ;//x
    bitangente[(i*(NB_r+1)*3)+ (j*3)+1] = -sin((float)i*theta) * cos((float)j*phi);//y
    bitangente[(i*(NB_r+1)*3)+ (j*3)+2] =  cos((float)j*phi);
}

int indiceMaxI =((NB_R+1)*(NB_r))-1;
int indiceMaxJ= (NB_r+1);

for (int i =0;i<NB_R;i++ )
for (int j =0;j<NB_r;j++ )
{ 	
int i0,i1,i2,i3,i4,i5;
 	indices[(i*NB_r*6)+ (j*6)]= (unsigned int)((i*(NB_r+1))+ j); 
   indices[(i*NB_r*6)+ (j*6)+1]=(unsigned int)((i+1)*(NB_r+1)+ (j));
   indices[(i*NB_r*6)+ (j*6)+2]=(unsigned int)(((i+1)*(NB_r+1))+ (j+1));
   indices[(i*NB_r*6)+ (j*6)+3]=(unsigned int)((i*(NB_r+1))+ j);
   indices[(i*NB_r*6)+ (j*6)+4]=(unsigned int)(((i+1)*(NB_r+1))+ (j+1));
   indices[(i*NB_r*6)+ (j*6)+5]=(unsigned int)(((i)*(NB_r+1))+ (j+1));
}

}

//----------------------------------------
GLubyte* glmReadPPM(char* filename, int* width, int* height)
//----------------------------------------
{
    FILE* fp;
    int i, w, h, d;
    unsigned char* image;
    char head[70];          /* max line <= 70 in PPM (per spec). */
    
    fp = fopen(filename, "rb");
    if (!fp) {
        perror(filename);
        return NULL;
    }
    
    /* grab first two chars of the file and make sure that it has the
       correct magic cookie for a raw PPM file. */
    fgets(head, 70, fp);
    if (strncmp(head, "P6", 2)) {
        fprintf(stderr, "%s: Not a raw PPM file\n", filename);
        return NULL;
    }
    
    /* grab the three elements in the header (width, height, maxval). */
    i = 0;
    while(i < 3) {
        fgets(head, 70, fp);
        if (head[0] == '#')     /* skip comments. */
            continue;
        if (i == 0)
            i += sscanf(head, "%d %d %d", &w, &h, &d);
        else if (i == 1)
            i += sscanf(head, "%d %d", &h, &d);
        else if (i == 2)
            i += sscanf(head, "%d", &d);
    }
    
    /* grab all the image data in one fell swoop. */
    image = new unsigned char[w*h*3];
    fread(image, sizeof(unsigned char), w*h*3, fp);
    fclose(fp);
    
    *width = w;
    *height = h;
    return image;
}

//----------------------------------------
void initTexture(void)
//-----------------------------------------
{
 int iwidth  , iheight;
    GLubyte *  image = NULL;
 
    image = glmReadPPM("../texture/Bricks01_COL_VAR1_1K.ppm", &iwidth, &iheight);
	  glGenTextures(1, &bufTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bufTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	  glTexImage2D(GL_TEXTURE_2D, 0, 3, iwidth,iheight, 0, GL_RGB,GL_UNSIGNED_BYTE,image);
    locationTexture = glGetUniformLocation(programID,"texture1");
    glUniform1i(locationTexture,0);
    image = glmReadPPM("../texture/Bricks01_NRM_1K.ppm", &iwidth, &iheight);
    glGenTextures(1, &bufNormalMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bufNormalMap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	  glTexImage2D(GL_TEXTURE_2D, 0, 3, iwidth,iheight, 0, GL_RGB,GL_UNSIGNED_BYTE,image);
    locationNormalMap = glGetUniformLocation(programID,"texture2");
    glUniform1i(locationNormalMap,1);
    //glActiveTexture(GL_TEXTURE0 + 1); // Texture unit 1
    //glBindTexture(GL_TEXTURE_2D, bufNormalMap);

    glBindAttribLocation(programID,indexUVTexture,"vertexUV");	// il y a les coord UV  

}
//----------------------------------------
void initOpenGL(void)
//----------------------------------------
{
  glCullFace (GL_BACK); // on spécifie queil faut éliminer les face arriere
  glEnable(GL_CULL_FACE); // on active l'élimination des faces qui par défaut n'est pas active
  glEnable(GL_DEPTH_TEST); 
// le shader
   programID = LoadShaders( "PhongShader.vert", "PhongShader.frag" );
 
 
   // Get  handles for our matrix transformations "MVP" VIEW  MODELuniform
  MatrixIDMVP = glGetUniformLocation(programID, "MVP");
  MatrixIDModel = glGetUniformLocation(programID, "Model");

  // Projection matrix : 65 Field of View, 1:1 ratio, display range : 1 unit <-> 1000 units
  // ATTENTIOn l'angle est donné en radians si f GLM_FORCE_RADIANS est défini sinon en degré
  Projection = glm::perspective( glm::radians(60.f), 1.0f, 1.0f, 1000.0f);

/* on recupere l'ID */
locCameraPosition = glGetUniformLocation(programID, "cameraPosition");
locAmbient = glGetUniformLocation(programID, "ambient");
locLightPos = glGetUniformLocation(programID, "lightPos");
locMatShininess = glGetUniformLocation(programID, "materialShininess");
locAttenuation = glGetUniformLocation(programID,"attenuation");

/*
ici ajouter autres handles pour la lumiere, les materiaux etc...
*/
 
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
  glutCreateWindow("TORE VBO SHADER au Départ");


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

   
   createTorus(1.,.3);

 // construction des VBO a partir des tableaux du cube deja construit
  genereVBO();
  initTexture();
  
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
 
    if(glIsBuffer(VBO_UVtext) == GL_TRUE) glDeleteBuffers(1, &VBO_UVtext);
    glGenBuffers(1, &VBO_UVtext);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_UVtext);
    glBufferData(GL_ARRAY_BUFFER, sizeof(coordTexture),coordTexture , GL_STATIC_DRAW);
    glVertexAttribPointer (indexUVTexture, 2, GL_FLOAT, GL_FALSE, 0,  (void*)0  );
    
    if(glIsBuffer(VBO_tangente) == GL_TRUE) glDeleteBuffers(1, &VBO_tangente);
    glGenBuffers(1, &VBO_tangente);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_tangente);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tangente),tangente , GL_STATIC_DRAW);
    glVertexAttribPointer ( indexTangente, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    
    
    if(glIsBuffer(VBO_bitangente) == GL_TRUE) glDeleteBuffers(1, &VBO_bitangente);
    glGenBuffers(1, &VBO_bitangente);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_bitangente);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bitangente),bitangente , GL_STATIC_DRAW);
    glVertexAttribPointer ( indexBitangente, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
 
    glEnableVertexAttribArray(indexVertex);
    glEnableVertexAttribArray(indexNormale );
    glEnableVertexAttribArray(indexUVTexture);
    glEnableVertexAttribArray(indexTangente);
    glEnableVertexAttribArray(indexBitangente);

 
   
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
    glDeleteBuffers(1, &VBO_UVtext);
    glDeleteBuffers(1, &VBO_tangente);
    glDeleteBuffers(1, &VBO_bitangente);
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
//Trace le tore 2 via le VAO
void traceObjet()
//-------------------------------------
{
 // Use  shader & MVP matrix   MVP = Projection * View * Model;
 glUseProgram(programID);

//on envoie les données necessaires aux shaders */
 glUniformMatrix4fv(MatrixIDModel, 1, GL_FALSE, &Model[0][0]);
 glUniformMatrix4fv(MatrixIDMVP, 1, GL_FALSE, &MVP[0][0]);
 glUniform3f(locCameraPosition,cameraPosition.x, cameraPosition.y, cameraPosition.z);
 glUniform1f(locAmbient,LightAmbientCoefficient);
 glUniform3f(locLightPos,LightPosition.x,LightPosition.y,LightPosition.z);
 glUniform1f(locMatShininess,materialShininess);
 glUniform3f(locAttenuation,attenuation.x, attenuation.y, attenuation.z);
 
/*
 Ici mettre les valeurs correspondant aux variables fournies par les handle vers le GPU
*/
 
//pour l'affichage
	glBindVertexArray(VAO); // on active le VAO
    glDrawElements(GL_TRIANGLES,  sizeof(indices), GL_UNSIGNED_INT, 0);// on appelle la fonction dessin 
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

void processMenuEvents(int option) {
    switch (option) {
        case 1: materialShininess+=.5;break;
        case 2: materialShininess-=.5;break;
        case 3: LightAmbientCoefficient+=.1; break;
        case 4: LightAmbientCoefficient-=.1; break;
        case 5: LightPosition.x+=.2; break;
        case 6: LightPosition.x-=.2; break;
        case 7: LightPosition.y+=.2; break;
        case 8: LightPosition.y-=.2; break;
        case 9: LightPosition.z+=.2; break;
        case 10: LightPosition.z-=.2; break;
        case 11: attenuation.x+=.1;break;
        case 12: if(attenuation.x >= 0.1 && (attenuation.x + attenuation.y + attenuation.z) != 0) attenuation.x-=.1;    break;
        case 13: LightPosition.y+=.05;break;
        case 14: if(attenuation.y >= 0.05 && (attenuation.x + attenuation.y + attenuation.z) != 0)attenuation.y-=.05;break;
        case 15: attenuation.z+=.05; break;
        case 16: if(attenuation.z >= 0.05 && (attenuation.x + attenuation.y + attenuation.z) != 0)attenuation.z-=.05; break; break;
        case 17: exit(0); break;
    }
    glutPostRedisplay();
}


void createGLUTMenus() {
    
    int menu, sub1, sub2, sub3;

    // create the menu and
    // tell glut that "processMenuEvents" will
    // handle the events

    sub2 = glutCreateMenu(processMenuEvents);
    glutAddMenuEntry("Augmenter X",5);
    glutAddMenuEntry("Diminuer X",6);
    glutAddMenuEntry("Augmenter Y",7);
    glutAddMenuEntry("Diminuer Y",8);
    glutAddMenuEntry("Augmenter Z",9);
    glutAddMenuEntry("Diminuer Z",10);

    sub3 = glutCreateMenu(processMenuEvents);
    glutAddMenuEntry("Augmenter Coefficient 1",11);
    glutAddMenuEntry("Diminuer Coefficient 1",12);
    glutAddMenuEntry("Augmenter Coefficient 2",13);
    glutAddMenuEntry("Diminuer Coefficient 2",14);
    glutAddMenuEntry("Augmenter Coefficient 3",15);
    glutAddMenuEntry("Diminuer Coefficient 3",16);


    menu = glutCreateMenu(processMenuEvents);

    //add entries to our menu
    glutAddMenuEntry("Augmenter Material Shininess",1);
    glutAddMenuEntry("Diminuer Material Shininess",2);
    glutAddMenuEntry("Augmenter Coefficient Ambient",3);
    glutAddMenuEntry("Diminuer Coefficient Ambient",4);
    glutAddSubMenu("Light Position", sub2);
    glutAddSubMenu("Coefficient Attenuation",sub3);
    glutAddMenuEntry("Quit", 17);
    // attach the menu to the right button
    glutAttachMenu(GLUT_MIDDLE_BUTTON);
}


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
    case 's' : /* Affichage en mode sommets seuls */
      materialShininess-=.5;
      glutPostRedisplay();
      break;
    case 'S' : /* Affichage en mode sommets seuls */
      materialShininess+=.5;
      glutPostRedisplay();
      break;
    case 'x' : /* Affichage en mode sommets seuls */
      LightPosition.x-=.2;
      glutPostRedisplay();
      break;
    case 'X' : /* Affichage en mode sommets seuls */
      LightPosition.x+=.2;
      glutPostRedisplay();
      break;
    case 'y' : /* Affichage en mode sommets seuls */
      LightPosition.y-=.2;
      glutPostRedisplay();
      break;
    case 'Y' : /* Affichage en mode sommets seuls */
      LightPosition.y+=.2;
      glutPostRedisplay();
      break;
    case 'z' : /* Affichage en mode sommets seuls */
      LightPosition.z-=.2;
      glutPostRedisplay();
      break;
    case 'Z' : /* Affichage en mode sommets seuls */
      LightPosition.z+=.2;
      glutPostRedisplay();
      break;
    case 'a' : /* Affichage en mode sommets seuls */
      LightAmbientCoefficient-=.1;
      glutPostRedisplay();
      break;
    case 'A' : /* Affichage en mode sommets seuls */
      LightAmbientCoefficient+=.1;
      glutPostRedisplay();
      break;
    case 'k' : /* Affichage en mode sommets seuls */
      if(attenuation.x >= 0.1 && (attenuation.x + attenuation.y + attenuation.z) != 0) attenuation.x-=.1;
      glutPostRedisplay();
      break;
    case 'K' : /* Affichage en mode sommets seuls */
      attenuation.x+=.1;
      glutPostRedisplay();
      break;  
    case 'l' : /* Affichage en mode sommets seuls */
       if(attenuation.y >= 0.05 && (attenuation.x + attenuation.y + attenuation.z) != 0)attenuation.y-=.05;
      glutPostRedisplay();
      break;
    case 'L' : /* Affichage en mode sommets seuls */
      attenuation.y+=.05;
      glutPostRedisplay();
      break;
    case 'm' : /* Affichage en mode sommets seuls */
      if(attenuation.z >= 0.05 && (attenuation.x + attenuation.y + attenuation.z) != 0)attenuation.z-=.05;
      glutPostRedisplay();
      break;
    case 'M' : /* Affichage en mode sommets seuls */
      attenuation.z+=.05;
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







