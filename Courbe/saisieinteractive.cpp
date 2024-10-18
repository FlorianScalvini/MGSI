#include <iostream>
#include <math.h>
#include "ifs.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif



void curveBezier();
void curveCatmull();
void curveSplineBNonUni();
void curveSplineBUni();
void drawVecteurNodal();
void resetVecteurNodal();

arma::vec vecT = linspace(0,1,20);
arma::mat matT = arma::join_horiz(pow(vecT,3),pow(vecT,2),vecT,arma::ones(arma::size(vecT)[0],size(vecT)[1]));
arma::mat result = mat(size(vecT)[0],2);
arma::vec vecteurNodal = vec();
const float PI = 3.1415926535;

// variables globales pour OpenGL
int window,width,height;
const int NMAX = 100;
int N = 0;
int mp=-1,droite=0,gauche=0;
int degreeNonUniforme = 3;
char ordre = 4;
int selectedKnot = 0;

struct Point {
	float x,y;
	Point(float a=0, float b=0) {set(a,b);}
	void set(float a, float b) {x=a;y=b;}
};

Point P[NMAX];


/*                     

		Fonction de gestion des valeurs du vecteur nodal. 

*/
void upValueVector(int i)
{
   if ( (i < N + ordre - 1) )
   {
      if (vecteurNodal(i) < vecteurNodal(i+1)) vecteurNodal[i] += 1.0;
	  else 
	  {
	     upValueVector(i+1);
		 if (vecteurNodal(i) < vecteurNodal(i+1)) vecteurNodal[i] += 1.0;
	  }
   }
   else if ( vecteurNodal(i) < 100) vecteurNodal[i] += 1.0;
}

void downValueVector(int i)
{
   if ( (i > 0) )
   {
      if (vecteurNodal(i) > vecteurNodal(i-1)) vecteurNodal[i] -= 1.0;
	  else 
	  {
	     downValueVector(i-1);
		 if (vecteurNodal(i) > vecteurNodal(i-1)) vecteurNodal[i] -= 1.0;
	  }
   }
   else if (vecteurNodal(i) > 0) vecteurNodal[i] -= 1.0;
}

void resetVecteurNodal(){
	vecteurNodal.resize(ordre + N);
	vecteurNodal = round(linspace(0,100, ordre + N));
	selectedKnot = 0;
}
/*                     

		Fonction d'affichage du vecteur nodal'

*/

void drawVecteurNodal(){
	glColor3f(1.0,1.0,0.0);
	if (N > 0){
		glPointSize(6.0);
		for(int i = 0; i < ordre + N; i++){
			glBegin(GL_POINTS);
				if(i==selectedKnot) glColor3f(0.0,1.0,0.0);
				else glColor3f(0.0,0.0,1.0);
				glVertex2f((glutGet(GLUT_WINDOW_WIDTH) - 40.0)*vecteurNodal(i)/100.0 + 20, 40.0);
			glEnd();
		}
		glColor3f(1.0,0.0,0.0);
		glBegin(GL_LINES);
			glVertex2f(20.0, 40.0);
			glVertex2f( glutGet(GLUT_WINDOW_WIDTH) - 20.0, 40.0);
		glEnd();
	}
}

/*                     

		Courbe de Bezier Cubique. 

*/

void curveBezier(){
	glColor3f(0.0,0.0,1.0);
	arma::mat matBezierCub = {{-1.0, 3.0, -3.0, 1.0}, 
							{3.0, -6.0, 3.0, 0.0}, 
							{-3.0, 3.0, 0.0, 0.0},
							{ 1.0, 0.0, 0.0, 0.0}};
	arma::mat pointControle = mat(4,2);
	for(int i = 3; i < N;i+=3){
		pointControle = {{P[i-3].x, P[i-3].y},
							   {P[i-2].x, P[i-2].y},
							   {P[i-1].x, P[i-1].y},
							   {P[i].x, P[i].y}};
		result = matT*matBezierCub*pointControle;
		glLoadName(i/3);
		glBegin(GL_LINE_STRIP);
		for (int j = 0; j < 20; j++)
			glVertex2f(result(j,0),result(j,1));
		glEnd();
	}
}

/*                     

		Courbe de Catmull Rom Cubique. 

*/

void curveCatmull(){
	glColor3f(1.0,0.0,0.0);
	arma::mat pointControle = mat(4,2);
	arma::mat matCatmullRomCub = {{-0.5, 1.5, -1.5, 0.5},
							  	{1.0, -2.5, 2.0, -0.5},
							  	{-0.5, 0.0, 0.5, 0.0}, 
							  	{0.0, 1.0, 0.0, 0.0}};
	for(int i = 3; i < N;i++){
		pointControle = {{P[i-3].x, P[i-3].y},
							   {P[i-2].x, P[i-2].y},
							   {P[i-1].x, P[i-1].y},
							   {P[i].x, P[i].y}};
		result = matT*matCatmullRomCub*pointControle;
		glLoadName(i-2);
		glBegin(GL_LINE_STRIP);
		for (int j = 0; j < 20; j++)
			glVertex2f(result(j,0),result(j,1));
		glEnd();
	}
}

/*                     

		Courbe B-Spline uniforme Cubique. 

	*/

	void curveSplineBUni(){
		glColor3f(0.0,1.0,0.0);
		arma::mat pointControle = mat(4,2);
		arma:: mat matBSplineUni = {{-1.0, 3.0, -3.0, 1.0}, 
									{3.0, -6.0, 3.0, 0.0}, 
									{-3.0, 0.0, 3.0, 0.0}, 
									{1.0, 4.0, 1.0, 0.0}};
		for(int i = 3; i < N;i++){
			pointControle = {{P[i-3].x, P[i-3].y},
								   {P[i-2].x, P[i-2].y},
								   {P[i-1].x, P[i-1].y},
								   {P[i].x, P[i].y}};
			result =matT*1/6.0*matBSplineUni*pointControle;	
			glBegin(GL_LINE_STRIP);			   
			for (int j = 0; j < 20; j++)
				glVertex2f(result(j,0),result(j,1));
			glEnd();
		}
	}

/*                     

		Courbe B-Spline non-uniforme et Fonction Recursive 

*/


float splineBrecurvise(int index, int ordre, float t){
	float part1, part2, denom1, denom2;
	if(ordre == 1){
		if(vecteurNodal(index) <= t && t < vecteurNodal(index+1)) return 1.0;
		else return 0.0;
	}
	else{
		denom1 = vecteurNodal(index + ordre - 1) - vecteurNodal(index);
        denom2 = vecteurNodal(index + ordre) - vecteurNodal(index + 1);
		if(denom1 == 0) part1 = 0;
		else part1 = (t-vecteurNodal(index)) / denom1;
		if(denom2 == 0) part2 = 0;
		else part2 = (vecteurNodal(index + ordre) - t) / denom2;
		return part1 * splineBrecurvise(index, ordre - 1, t) + part2 * splineBrecurvise(index + 1, ordre - 1,t);
	}
}



void curveSplineBNonUni(){
	glColor3f(1.0,1.0,0.0);
	arma::mat pointControle = mat(ordre,2);
	arma::rowvec valueBik = rowvec(ordre);
	arma::rowvec res = rowvec(2);
	for(int i = ordre - 1; i < N;i++){
		for(int j=i - ordre + 1; j<=i;j++)
			pointControle.row(j-i+ordre - 1) = rowvec({P[j].x, P[j].y});
		glBegin(GL_LINE_STRIP);
		for (float t = vecteurNodal(i); t < vecteurNodal(i + 1); t += 0.005){
			for(int j = ordre - 1; j >= 0;j--){
				valueBik.col(ordre - 1 -j) = splineBrecurvise(i - j , ordre, t);
			}
			res = valueBik * pointControle;
			glVertex2f(res(0),res(1));
		}
		glEnd();
	}
}

/*

	Trace les N points

*/

void Trace()
{
	glBegin(GL_POINTS);
	for (int i=0;i<N;i++){
		glVertex2f(P[i].x,P[i].y);
	}
	glEnd();
}

/*

	Fonction de base d'OpenGL

*/

void main_reshape(int width,  int height) 
{
	GLint viewport[4];
    glViewport(0, 0, width, height);
    glGetIntegerv(GL_VIEWPORT, viewport);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	glOrtho(0.0, viewport[2], 0.0, viewport[3], -50.0, 50.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void main_display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
	glColor3f(0.0,1.0,0.0);
    glPointSize(3.0);
	glInitNames();
	glPushName(1);
	glColor3f(0.0,1.0,0.0);
	for (int i=0;i<N;i++){
		glLoadName(i);
		glBegin(GL_POINTS);
		glVertex2f(P[i].x,P[i].y);
		glEnd();
	}
	glPushName(2);
	curveBezier();
	glPushName(3);
	curveCatmull();
	glPushName(4);
	curveSplineBUni();
	drawVecteurNodal();
	curveSplineBNonUni();
	glRasterPos2f(glutGet(GLUT_WINDOW_WIDTH) - 40, glutGet(GLUT_WINDOW_HEIGHT) - 40);
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, '0' + ordre);
	glutPostRedisplay();
    glutSwapBuffers();
}

void Mouse(int button, int state, int x, int y) {
	GLint viewport[4];

	glutSetCursor(GLUT_CURSOR_CROSSHAIR);
	glGetIntegerv(GL_VIEWPORT, viewport);

	if(button == GLUT_LEFT_BUTTON) {
		droite = 0; gauche = 1;
			glColor3f(0.0,1.0,0.0);
			glPointSize(3.0);
			glInitNames();
			glPushName(1);

			P[N].x = x;
			P[N].y = viewport[3]-y;

			glLoadName(N);
			glBegin(GL_POINTS);
				glVertex2f(P[N].x,P[N].y);
			glEnd();
			if(state == GLUT_UP) {
				N++;
				resetVecteurNodal();
			}
			glutPostRedisplay();

		}

	if(button == GLUT_RIGHT_BUTTON) {
		gauche = 0; droite = 1;
		if(state == GLUT_DOWN) {
			GLuint *selectBuf = new GLuint[200];
			GLuint *ptr;
			GLint hits;

			glSelectBuffer(200, selectBuf);
			glRenderMode(GL_SELECT);

			glPushMatrix();
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPickMatrix(x, viewport[3]-y, 5.0, 5.0, viewport);
			glOrtho(0.0, viewport[2], 0.0, viewport[3], -50.0, 50.0);

			glColor3f(0.0,1.0,0.0);
			glPointSize(3.0);
			glInitNames();
			glPushName(1);

			for (int i = 0;i<N;i++) {
				glLoadName(i);
				glBegin(GL_POINTS);
				glVertex2f(P[i].x,P[i].y);
				glEnd();			
			}
				
			glPopMatrix();
			glFlush();

			hits = glRenderMode(GL_RENDER);
			if(hits) {
				ptr = (GLuint *)selectBuf;
				ptr += 3;
				mp = *ptr;
			}
		}

		if(state == GLUT_UP)
			mp = -1;

		main_reshape(viewport[2], viewport[3]);
		glutPostRedisplay();
	}
}

void clavierSpecial(int key, int x, int y){
  switch (key)
    {
    case GLUT_KEY_UP : 
		if(selectedKnot < ordre + N) selectedKnot++;
		else selectedKnot = 0;
	  	break;
	case GLUT_KEY_DOWN : 
		if(selectedKnot > 0) selectedKnot--;
		else selectedKnot = ordre + N;
	  	break;
	case GLUT_KEY_LEFT : 
		downValueVector(selectedKnot);	  	
		break;
	case GLUT_KEY_RIGHT : /*la touche 'q' permet de quitter le programme */
      	upValueVector(selectedKnot);	
	  	break;
	}
	glutPostRedisplay();			
}

void clavier(unsigned char touche,int x,int y)
{
  switch (touche)
    {
    case 'q' : /*la touche 'q' permet de quitter le programme */
    	exit(0);
	case ' ':
		if(ordre < 6) ordre++;
		else ordre = 2;
		resetVecteurNodal();
		break;
	}

}

void Motion(int x, int y) {
	GLint viewport[4];

	glGetIntegerv(GL_VIEWPORT, viewport);
	
	if ((droite) &&(mp != -1)) {
		int i= mp;
		P[i].x = x;
		P[i].y = viewport[3]-y;
		Trace();
		glutPostRedisplay();
	}
}


/*                     

		Main 

*/

int main (int argc, char** argv)
{
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(0, 0);
    glutInit(&argc, argv);
    window = glutCreateWindow("Select");
    glEnable(GL_DEPTH_TEST);
	glutReshapeFunc(main_reshape);
    glutDisplayFunc(main_display);
  	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);	
	glutKeyboardFunc(clavier);
	glutSpecialFunc(clavierSpecial);

	glutPostRedisplay();  
    glutMainLoop();
    
    return 0;
}
