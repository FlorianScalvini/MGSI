#include <iostream>
#include <math.h>
#include <vector>
#include "ifs.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif


void curveBezier();
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
int m = 2,l = 2;
struct Point {
	float x,y;
	Point(float a=0, float b=0) {set(a,b);}
	void set(float a, float b) {x=a;y=b;}
};
bool selection = false;
float minX,maxX,minY,maxY;
Point P[NMAX];
Point Pmemo[NMAX]; // Sauvegarde la valeur d'origine des points
Point PdcDeforma[30][30];



/*
 
  Calcul des coefficients de Bernstein avec le triangle de Pascal
 
*/
int pas0[] = {1};
int pas1[] = {1,1};
int pas2[] = {1,2,1};
int pas3[] = {1,3,3,1};
int pas4[] = {1,4,6,4,1};
int pas5[] = {1,5,10,10,5,1};
int pas6[] = {1,6,15,20,15,6,1};
int pas7[] = {1,7,21,35,21,7,1};

// Tableau des coefficients de Pascal
int *pascal[]
{
    pas0,
    pas1,
    pas2,
    pas3,
    pas4,
    pas5,
    pas6,
    pas7
};

//Formule des polynomes de Benstein
double Bernstein(int i, int n, double u){
    return pascal[n][i]*pow(u,i)*pow(1-u,m-i);
}



// Forme à déformer: Courbe de Bezier Cubique
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
		result = matT*matBezierCub*pointControle; // C(t) = T * M * P
		glBegin(GL_LINE_STRIP);
		for (int j = 0; j < 20; j++)
			glVertex2f(result(j,0),result(j,1));
		glEnd();
	}
}


/*

	Trace les N points

*/


void ffd(){
    float t,s;
    float bs, bt;
    float mx;
    float my;
    for(int i = 0; i < N; i++){
        s = (Pmemo[i].x - minX) / (maxX - minX);
        t = (Pmemo[i].y - minY) / (maxY - minY);
        mx = 0;
        my = 0;
        for(int index = 0; index <= m; index++){
            bs = Bernstein(index,m,s); // Calcul coeff de
            for(int j = 0; j <= l; j++){
                bt = Bernstein(j,l,t);
                mx = mx + bs*bt*((PdcDeforma[index][j].x - minX) / (maxX - minX)); // Nouvelle position X
                my = my + bs*bt*((PdcDeforma[index][j].y - minY) / (maxY - minY)); // Nouvelle position Y
            }
        }
        P[i].x = mx*(maxX - minX) + minX;
        P[i].y= my*(maxY - minY) + minY;
    }
    
}



void Trace()
{
	glBegin(GL_POINTS);
	for (int i=0;i<N;i++){
		glVertex2f(P[i].x,P[i].y);
	}
    glEnd();
    if(N > 1){ // Si nombre de point > 1
        for (int i=0;i<=m;i++){
            for (int j=0;j<=l;j++){
                glPointSize(8.0);
                glColor3f(1,0,1);
                glBegin(GL_POINTS);
                glVertex2f(PdcDeforma[i][j].x,PdcDeforma[i][j].y); // Draw point de la boite emglobante
                glEnd();
                glColor3f(1,1,1);
                glPointSize(4.0);
                if (j<l) { // Draw arete de la boite emglobante
                    glBegin(GL_LINES);
                    glVertex2f(PdcDeforma[i][j].x,PdcDeforma[i][j].y);
                    glVertex2f(PdcDeforma[i][j+1].x,PdcDeforma[i][j+1].y);
                    glEnd();
                }
                if (i<m) { // Draw arete de la boite emglobante
                    glBegin(GL_LINES);
                    glVertex2f(PdcDeforma[i][j].x,PdcDeforma[i][j].y);
                    glVertex2f(PdcDeforma[i+1][j].x,PdcDeforma[i+1][j].y);
                    glEnd();
                }
            }
        }
    }
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
// Calcul de la boite emglobante
void valueBox(){ 
	int i;
    minX = P[0].x; maxX = P[0].x;minY = P[0].y;maxY = P[0].y;
	for(i = 1; i < N ; i++){
		if(P[i].x < minX) minX = P[i].x;
		if(P[i].x > maxX) maxX = P[i].x;
		if(P[i].y < minY) minY = P[i].y;
		if(P[i].y > maxY) maxY = P[i].y;
	}
}
void reset_pdc(){
    valueBox();
    if(N>0){
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        float diffX = (maxX-minX)/ m;
        float diffY = (maxY-minY)/ l;
        for (int i=0;i<=m;i++){
            for (int j=0;j<=l;j++){
                PdcDeforma[i][j].x = diffX*i+minX;
                PdcDeforma[i][j].y = diffY*j+minY;
            }
        }
    }
    Trace();
}




void main_display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glPointSize(3.0);
    glInitNames();
    glPushName(1);
    glColor3f(0.0,1.0,0.0);
    Trace();
    curveBezier();
    if(N > 1){
        glColor3f(1,1,1);
    }
    
    glRasterPos2f(glutGet(GLUT_WINDOW_WIDTH) - 40, glutGet(GLUT_WINDOW_HEIGHT) - 40);
    glutPostRedisplay();
    glutSwapBuffers();
}

void Mouse(int button, int state, int x, int y) {
	GLint viewport[4];

	glutSetCursor(GLUT_CURSOR_CROSSHAIR);
	glGetIntegerv(GL_VIEWPORT, viewport);

	if(button == GLUT_LEFT_BUTTON && !selection) {
		droite = 0; gauche = 1;
			glColor3f(0.0,1.0,0.0);
			glPointSize(3.0);
			glInitNames();
			glPushName(1);

			P[N].x = x;
			P[N].y = viewport[3]-y;

            Pmemo[N].x =P[N].x;
            Pmemo[N].y =P[N].y;
			glLoadName(N);
			glBegin(GL_POINTS);
				glVertex2f(P[N].x,P[N].y);
			glEnd();
			if(N==0){ 
				minX = P[0].x; minY = P[0].y; maxX = P[0].x;maxY = P[0].y;
			}
			if(state == GLUT_UP) {
				N++;
                reset_pdc();
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
            glInitNames();
            glPushName(1);
            if(!selection){
                glColor3f(0.0,1.0,0.0);
                glPointSize(3.0);
                for (int i = 0;i<N;i++) {
                    glLoadName(i);
                    glBegin(GL_POINTS);
                    glVertex2f(P[i].x,P[i].y);
                    glEnd();
                }
                reset_pdc();
            }
            else{
                glPointSize(6.0);
                glColor3f(1,0,1);
                for (int i = 0;i<=m;i++) {
                    for (int j = 0;j<=l;j++) {
                        glLoadName(NMAX+i*(l+1)+j);
                        glBegin(GL_POINTS);
                        glVertex2f(PdcDeforma[i][j].x,PdcDeforma[i][j].y);
                        glEnd();
                    }
                }
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

		if(state == GLUT_UP){
			mp = -1;
		}
		main_reshape(viewport[2], viewport[3]);
		glutPostRedisplay();
	}
}


void clavier(unsigned char touche,int x,int y)
{
  switch (touche)
    {
    case 'q' : /*la touche 'q' permet de quitter le programme */
    	exit(0);
    case ' ':
        selection = !selection;
        break;
    }

}

void Motion(int x, int y) {
	GLint viewport[4];

	glGetIntegerv(GL_VIEWPORT, viewport);
	
	if ((droite) &&(mp != -1)) {
        int i= mp ;
        if(!selection){
            P[i].x = x;
            P[i].y = viewport[3]-y;
            reset_pdc();
        }
        else{
            i = (mp - NMAX)/(l+1);
            int j = (mp - NMAX)%(l+1);
            PdcDeforma[i][j].x = x;
            PdcDeforma[i][j].y = viewport[3]-y;
            ffd();
        }
	}
    Trace();
    
    glutPostRedisplay();
}

void processMenuEvents(int option) {
    switch (option) {
        case 1 :
            selection = false; break;
        case 2 :
            selection = true; break;
        case 3 :
            m++;
            reset_pdc();
            break;
        case 4 :
            if(m>1)m--;
            reset_pdc();
            break;
        case 5 :
            l++;
            reset_pdc();
            break;
        case 6 :
            if(l>1)l--;
            reset_pdc();
            break;
    }
}

void createGLUTMenus() {

    int menu;

    // create the menu and
    // tell glut that "processMenuEvents" will
    // handle the events
    menu = glutCreateMenu(processMenuEvents);

    //add entries to our menu
    glutAddMenuEntry("Selection PdC Bezier",1);
    glutAddMenuEntry("Selection PdC box",2);
    glutAddMenuEntry("Augmenter M",3);
    glutAddMenuEntry("Diminuer M",4);
    glutAddMenuEntry("Augmenter L",5);
    glutAddMenuEntry("Diminuer L",6);
    // attach the menu to the right button
    glutAttachMenu(GLUT_MIDDLE_BUTTON);
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
    glutKeyboardFunc(clavier);
	glutMotionFunc(Motion);
    createGLUTMenus();
	glutPostRedisplay();  
    glutMainLoop();
    
    return 0;
}
