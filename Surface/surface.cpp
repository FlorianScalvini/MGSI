/********************************************************/
/*                     Surface.c                           */
/********************************************************/
/********************************************************/

/* inclusion des fichiers d'en-tete Glut */

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <iostream>
#include "ifs.h"
#include <stdlib.h>
#include <vector>

char presse;
int anglex,angley,x,y,xold,yold;
int window,width,height;
int selected [] = {0,0};

int tailleVec = 10;
const arma::vec vecT = linspace(0,1,tailleVec);
const arma::mat matT = arma::join_horiz(pow(vecT,3),pow(vecT,2),vecT,arma::ones(arma::size(vecT)[0],size(vecT)[1]));
const arma::mat matBezierCub = {{-1.0, 3.0, -3.0, 1.0}, {3.0, -6.0, 3.0, 0.0}, {-3.0, 3.0, 0.0, 0.0},{ 1.0, 0.0, 0.0, 0.0}};
const arma::mat matCatmullRomCub = {{-0.5, 1.5, -1.5, 0.5},{1.0, -2.5, 2.0, -0.5},{-0.5, 0.0, 0.5, 0.0}, {0.0, 1.0, 0.0, 0.0}};
const arma:: mat matBSplineUni = {{-1.0, 3.0, -3.0, 1.0}, {3.0, -6.0, 3.0, 0.0}, {-3.0, 0.0, 3.0, 0.0}, {1.0, 4.0, 1.0, 0.0}};
const arma::mat matBezierMultVecT = matT*matBezierCub;
const arma::mat matCatVecT = matT*matCatmullRomCub;
const arma::mat matBSplineUniVecT = matT*1/6.0*matBSplineUni;
const int m =15; const int n = 13; char ordre = 4;
char selectedKnot; char check = 0; bool pointOrNodal = false; bool drawSurface[4] = {0};
arma::vec vecteurNodal [2];
arma::vec Point[m][n];
vector <rowvec> savePoint;
/* Prototype des fonctions */
void affichage();
void clavier(unsigned char touche,int x,int y);
void reshape(int x,int y);
void idle();
void mouse(int bouton,int etat,int x,int y);
void mousemotion(int x,int y);
void clavierSpecial(int key, int x, int y);


/*
     GESTION DES VECTEURS NODAUX
 */
void upValueVector(int i)
{
   int N = (check == 0) ? m : n;
     
   if ( (i < N + ordre - 1) )
   {
      if (vecteurNodal[check](i) < vecteurNodal[check](i+1)) vecteurNodal[check][i] += 5.0;
	  else 
	  {
	     upValueVector(i+1);
		 if (vecteurNodal[check](i) < vecteurNodal[check](i+1)) vecteurNodal[check][i] += 5.0;
	  }
   }
   else if ( vecteurNodal[check](i) < 100) vecteurNodal[check][i] += 5.0;
}

void downValueVector(int i)
{
   if ( (i > 0) )
   {
      if (vecteurNodal[check](i) > vecteurNodal[check](i-1)) vecteurNodal[check][i] -= 5.0;
	  else 
	  {
	     downValueVector(i-1);
		 if (vecteurNodal[check](i) > vecteurNodal[check](i-1)) vecteurNodal[check][i] -= 5.0;
	  }
   }
   else if (vecteurNodal[check](i) > 0) vecteurNodal[check][i] -= 5.0;
}
void drawPoint(int i, int size,char caseVector){
    glBegin(GL_POINTS);
      if(check == caseVector && i==selectedKnot) glColor3f(0.0,1.0,0.0);
      else glColor3f(0.0,0.0,1.0);    
      glVertex2d(1.6*vecteurNodal[caseVector](i)/(100*(ordre + size -1)) - 0.8,-0.8 -caseVector*0.1);
    glEnd();
}

void drawVecteurNodal(){
	glColor3f(1.0,1.0,0.0);
		glPointSize(6.0);
		for(int i = 0; i < ordre + m; i++){drawPoint(i,m,0);}
    for(int i = 0; i < ordre + n; i++){drawPoint(i,n,1);}    
		glColor3f(1.0,0.0,0.0);
		glBegin(GL_LINES);
			glVertex2d(-0.8, -0.8);
			glVertex2d(0.8, -0.8);
		glEnd();
    
        glBegin(GL_LINES);
            glVertex2d(-0.8, -0.9);
            glVertex2d(0.8, -0.9);
        glEnd();
}

void resetVecteurNodal(){
	vecteurNodal[0].resize(ordre + m);
  vecteurNodal[1].resize(ordre + n);
	vecteurNodal[0] = linspace(0,100*(ordre + m -1), ordre + m);
  vecteurNodal[1] = linspace(0,100*(ordre + n -1), ordre + n);
	selectedKnot = 0;
}

void GrillePoint(){
  int i,j;
  for(i = 0; i < m; i++){
      for(j = 0; j < n; j++){
        Point[i][j] = arma::vec(3);
        Point[i][j](0) = i/10.0 - m/20.0;
        Point[i][j](1) = 0.0;
        Point[i][j](2) = j/10.0 - m/20.0;
      }
  }
}
// Draw morceau de surface
void drawMatrix(arma::cube result){ 
    for(int u=0; u<tailleVec-1;u++){
      for(int v=0; v<tailleVec-1;v++){
        glBegin(GL_POLYGON);
          glVertex3d(result(u,v,0),result(u,v,1),result(u,v,2));
          glVertex3d(result(u,v+1,0),result(u,v+1,1),result(u,v+1,2));
          glVertex3d(result(u+1,v+1,0),result(u+1,v+1,1),result(u+1,v+1,2));
          glVertex3d(result(u+1,v,0),result(u+1,v,1),result(u+1,v,2));
        glEnd();
      }
    }
}

// Surface Catmull + BSplineUni
void surface(){
  arma::cube result = cube(tailleVec,tailleVec,3);
  arma::cube matrixCalcul = cube(4,4,3);
  for(int a = 3; a < m; a++){
    for(int b = 3; b < n; b++){
        for(int i=3; i>=0;i--){
          for(int j=3; j>=0;j--){
            matrixCalcul.tube(i,j) = Point[a-i][b-j]; // point de controle
          } 
        }
        if(drawSurface[1]){ // Draw Catmull
            result = (matCatVecT*matrixCalcul.each_slice()).each_slice()*trans(matCatVecT);
            glColor3f(1.0,0.0,1.0);
            drawMatrix(result);
        }
        if(drawSurface[2]){ // Draw B-Spline Uniforme
            result = (matBSplineUniVecT*matrixCalcul.each_slice()).each_slice()*trans(matBSplineUniVecT);
            glColor3f(1.0,1.0,0.0);
            drawMatrix(result);
        }
    }
  }
}
// Surface de Bezier bi-Cubique
void bezier(){
    arma::cube result = cube(tailleVec,tailleVec,3);
    arma::cube matrixCalcul = cube(4,4,3);
    for(int a = 3; a < m; a+=3){
        for(int b = 3; b < n; b+=3){
            for(int i=3; i>=0;i--){
                for(int j=3; j>=0;j--){
                    matrixCalcul.tube(i,j) = Point[a-i][b-j]; // point de controle
                }
            }
            
        result = (matBezierMultVecT*matrixCalcul.each_slice()).each_slice()*trans(matBezierMultVecT); // C(t) = T*M*T'
        drawMatrix(result);
        }
    }
}




/*

        Surface B-Spline non-uniforme et Fonction Recursive

*/


float splineBrecurvise(int index, int ordre, float t, int indexvec){
    float part1, part2, denom1, denom2;
    if(ordre == 1){
        if(vecteurNodal[indexvec](index) <= t && t < vecteurNodal[indexvec](index+1)) return 1.0;
        else return 0.0;
    }
    else{
        denom1 = vecteurNodal[indexvec](index + ordre - 1) - vecteurNodal[indexvec](index);
        denom2 = vecteurNodal[indexvec](index + ordre) - vecteurNodal[indexvec](index + 1);
        if(denom1 == 0) part1 = 0;
        else part1 = (t-vecteurNodal[indexvec](index)) / denom1;
        if(denom2 == 0) part2 = 0;
        else part2 = (vecteurNodal[indexvec](index + ordre) - t) / denom2;
        return part1 * splineBrecurvise(index, ordre - 1, t, indexvec) + part2 * splineBrecurvise(index + 1, ordre - 1,t, indexvec);
    }
}



void curveSplineBNonUni(){
    arma::rowvec valueBiku = rowvec(ordre); // Bi(u)
    arma::rowvec valueBikv = rowvec(ordre); //Bi(v)
    arma::cube matrixCalcul = cube(ordre ,ordre ,3);
    arma::rowvec resuv = rowvec(3);
   
    savePoint.clear();
    for(int a = ordre - 1; a < m; a++){
    for(int b = ordre - 1; b < n; b++){
        for(int i=ordre - 1; i>=0;i--){
          for(int j=ordre - 1; j>=0;j--){
            matrixCalcul.tube(i,j) = Point[a-i][b-j]; // point de controle
          }
        }
        
        glColor3f(1.0,0.0,1.0);
        for (float t = vecteurNodal[0](a); t < vecteurNodal[0](a + 1) ; t += 30){
            for(int i = ordre - 1; i >= 0;i--){
                valueBiku.col(ordre - 1 -i) = splineBrecurvise(a - i , ordre, t,0);
            }
            for (float t1 = vecteurNodal[1](b); t1 < vecteurNodal[1](b + 1)-20; t1 += 30){
              for(int ii = ordre - 1; ii >= 0;ii--){
                  valueBikv.col(ordre - 1 -ii) = splineBrecurvise(b - ii , ordre, t1,1);
              }
             resuv = (valueBiku*matrixCalcul.each_slice()).each_slice()*trans(valueBikv);
            glBegin(GL_POINTS);
            glVertex3f(resuv(0),resuv(1),resuv(2));
            glEnd();

            }

        }  
 
      }
    }
}

void affichage()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    drawVecteurNodal();
    glPopMatrix();
    glPushMatrix();
    glRotatef(-angley,1.0,0.0,0.0);
    glRotatef(-anglex,0.0,1.0,0.0);

    glColor3f(0.0,1.0,0.0);
    glPointSize(3.0);
    glInitNames();
    glPushName(1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if(drawSurface[2] || drawSurface[1])surface();
    glColor3f(0.0,0.0,1.0);
    if(drawSurface[0]) bezier();
    if(drawSurface[3]) curveSplineBNonUni();
    glPointSize(5.0);
    glBegin(GL_POINTS);
    for(int i = 0; i < m; i++){
    for(int j = 0; j < n; j++){
      if((i == selected[0] && j == selected[1]) ||
         (-1 == selected[0] && j == selected[1]) ||
         (i == selected[0] && -1 == selected[1])) glColor3f(0.0,1.0,0.0);
      else glColor3f(1.0, 0.0,0.0);
      glVertex3f(Point[i][j](0), Point[i][j](1),Point[i][j](2));
    }
    }
    glEnd();
    glPopMatrix();
    glutPostRedisplay();
    glutSwapBuffers();
}

void movePoint(float x, float y, float z){

  if(selected[0] != -1 && selected[1] != -1){
    Point[selected[0]][selected[1]](0) += x;
    Point[selected[0]][selected[1]](1) += y;
    Point[selected[0]][selected[1]](2) += z;
  }
  else if (selected[0] == -1)
  {
    for(int i = 0; i<m;i++){
      Point[i][selected[1]](0) += x;
      Point[i][selected[1]](1) += y;
      Point[i][selected[1]](2) += z;
    }
  }
  else
  {
    for(int j = 0; j<n;j++){
      Point[selected[0]][j](0) += x;
      Point[selected[0]][j](1) += y;
      Point[selected[0]][j](2) += z;
    }
  }
}
/* GESTION DES EVENEMENTS, Menu, ... */

void clavierSpecial(int key, int x, int y){
  int N = (check == 0) ? m : n;
  switch (key)
  {
    case GLUT_KEY_UP:
      if(selected[1] != -1 && !pointOrNodal) selected[1]++;
      else if(pointOrNodal) upValueVector(selectedKnot);
      break;
    case GLUT_KEY_DOWN:
      if(selected[1] != -1 && !pointOrNodal) selected[1]--;
      else if(pointOrNodal) downValueVector(selectedKnot);
      break;
    case GLUT_KEY_RIGHT:
      if(selected[0] != -1 && !pointOrNodal) selected[0]++;
      else if(pointOrNodal) {
         if(selectedKnot < ordre + N) selectedKnot++;
         else selectedKnot = 0;
      }
      break;
    case GLUT_KEY_LEFT:
      if(selected[0] != -1 && !pointOrNodal) selected[0]--;
      else if(pointOrNodal){
          if(selectedKnot > 0) selectedKnot--;
          else selectedKnot = ordre + N;
      }
      break;
    }
    glutPostRedisplay();
}


void clavier(unsigned char touche,int x,int y)
{
  int N = (check == 0) ? m : n;
  switch (touche)
    {
    case 'r':
      movePoint(0.,.01,0.); // Deplacement point de controle en Y
      break;
    case 'f':
      movePoint(0.,-.01,0.); // Deplacement point de controle en Y
      break;
    case 'e' :
      movePoint(0.,0.,.01); // Deplacement point de controle en Z
      break;
    case 'd' :
      movePoint(0.,0.,-.01); // Deplacement point de controle en Z
      break;
    case 's' :
      movePoint(-.01,0.,0.); // Deplacement point de controle en X
        break;
    case 'z' :
    movePoint(.01,0.,0.); // Deplacement point de controle en X
      break;
    case 'q' : /*la touche 'q' permet de quitter le programme */
      exit(0);
    }
    glutPostRedisplay();
}

void reshape(int x,int y)
{
  if (x<y)
    glViewport(0,(y-x)/2,x,x);
  else
    glViewport((x-y)/2,0,y,y);
}

void mouse(int button, int state,int x,int y)
{
  /* si on appuie sur le bouton gauche */
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
  {
    presse = 1; /* le booleen presse passe a 1 (vrai) */
    xold = x; /* on sauvegarde la position de la souris */
    yold=y;
  }
  /* si on relache le bouton gauche */
  if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
    presse=0; /* le booleen presse passe a 0 (faux) */
}

void mousemotion(int x,int y)
  {
    if (presse) /* si le bouton gauche est presse */
    {
      /* on modifie les angles de rotation de l'objet
     en fonction de la position actuelle de la souris et de la derniere
     position sauvegardee */
      anglex=anglex+(x-xold);
      angley=angley+(y-yold);
      glutPostRedisplay(); /* on demande un rafraichissement de l'affichage */
    }
    
    xold=x; /* sauvegarde des valeurs courante de le position de la souris */
    yold=y;
  }

void processMenuEvents(int option) {
    switch (option) {
        case 1 :
            check = !check;
            selectedKnot = 0;
            break;
        case 2:
            pointOrNodal = !pointOrNodal;
            break;
        case 3 :
            if(selected[0] == -1){selected[0]=0; selected[1] = -1;}
            else if(selected[1] == -1){selected[0]=0; selected[1] = 0;}
            else selected[0] = -1;
            break;
        case 4:
            drawSurface[0] = !drawSurface[0]; // Affichage Bezier
            break;
        case 5:
            drawSurface[1] = !drawSurface[1]; // Affichage Catmull
            break;
        case 6:
            drawSurface[2] = !drawSurface[2]; // Affichage BSpline Uni
            break;
        case 7:
            drawSurface[3] = !drawSurface[3]; // Affichage BSpline Non-Uniforme
            break;
        case 8:
            if(ordre < 6) ordre++;
            else ordre = 2;
            resetVecteurNodal();
              break;
        case 9: movePoint(.01,0.,0.);break;
        case 10: movePoint(-.01,0.,0.);break;
        case 11: movePoint(0.,.01,0.);break;
        case 12: movePoint(0.,-.01,0.);break;
        case 13: movePoint(0.,0.,.01);break;
        case 14: movePoint(0.,0.,-.01);break;
        case 15 : exit(0); break;
    }
}

void createGLUTMenus() {

    int menu,sub1;

    // create the menu and
    // tell glut that "processMenuEvents" will
    // handle the events
    sub1 = glutCreateMenu(processMenuEvents);
    glutAddMenuEntry("Augmenter X",9);
    glutAddMenuEntry("Diminuer X",10);
    glutAddMenuEntry("Augmenter Y",11);
    glutAddMenuEntry("Diminuer Y",12);
    glutAddMenuEntry("Augmenter Z",13);
    glutAddMenuEntry("Diminuer Z",14);

    menu = glutCreateMenu(processMenuEvents);

    //add entries to our menu
    glutAddMenuEntry("Change vecteur Nodal",1);
    glutAddMenuEntry("Select Point ou Nodal",2);
    glutAddMenuEntry("Change outil selection",3);
    glutAddMenuEntry("Draw Bezier",4);
    glutAddMenuEntry("Draw Calmull-Rom",5);
    glutAddMenuEntry("Draw B-Spline Uniforme",6);
    glutAddMenuEntry("Draw B-Spline Non Uniforme",7);
    glutAddMenuEntry("Changement Ordre",8);
    glutAddSubMenu("Mouvement point controle", sub1);
    glutAddMenuEntry("Quit", 15);
    // attach the menu to the right button
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main (int argc, char** argv)
{
    GrillePoint();
    resetVecteurNodal();
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(200, 200);
    glutInit(&argc, argv);
    window = glutCreateWindow("Select");
    glEnable(GL_DEPTH_TEST);
    glutReshapeFunc(reshape);
    glutDisplayFunc(affichage);
    glutMouseFunc(mouse);
    glutMotionFunc(mousemotion);
    glutKeyboardFunc(clavier);
    glutSpecialFunc(clavierSpecial);
    createGLUTMenus();
    glutPostRedisplay();
    glutMainLoop();
    
    return 0;
}

