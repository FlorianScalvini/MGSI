#include <iostream>
#include <math.h>


#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif




float Ri = 100; // Rayon d'influence
float seuil = 0.5;



// variables globales pour OpenGL
int window,width,height;
const int NMAX = 100;
int N = 0;
int mp=-1,droite=0,gauche=0;
// taille grille
int m = 50;
int n = 50;

char selectionMode = 0; // Selection = 0 : Compo, = 1 : Intersection, = 2 : Union, = 3 : Difference
char selectionFctPotentiel = 0; // Murakani = 0, Nishimura = 1, Wyvill = 2

struct Point {
    float x,y;
    Point(float a=0, float b=0) {set(a,b);}
    void set(float a, float b) {x=a;y=b;}
};

Point P[NMAX];




/*

    Fonctions de potentiel implémentées

*/

float murakani(float r){
    if(r < Ri) return pow(1-pow(r/Ri,2),2);
    else return 0.0;
}

float nishimura(float r){
    if(r >= 0 && r <= Ri/3.0) return 1-3*pow(r/Ri,2);
    else if(r >= Ri/3.0 && r <= Ri) return (3.0/2.0)*pow(1.0-(r/Ri),2);
    else return 0.0;
}

float wyvill(float r){
    if(r < Ri) return (-4.0/9.0 * pow(r/Ri,6) + 17.0/9.0 * pow(r/Ri,4) - 22.0/9.0 * pow(r/Ri,2) + 1);
    else return 0.0;
}


float fctPotentiel(float r){
    float value = 0.0;
    switch(selectionFctPotentiel){
        case 0:
            value = murakani(r);
            break;
        case 1:
            value = nishimura(r);
            break;
        case 2:
            value = wyvill(r);
            break;
    }
    return value;
}


bool calculPotentiel(float x, float y){
    float value = 0.0;
    float distance;
    bool output;
    for(int k=0;k<N;k++){
        distance = sqrt(pow(y-P[k].y,2)+pow(x-P[k].x,2));
        if(selectionMode == 0) value += fctPotentiel(distance);
        else if (selectionMode == 1){
            if(k==0) value = distance;
            else if (distance > value) value = distance;
        }
        else if(selectionMode == 2){
            if(k==0) value = distance;
            else if (distance < value) value = distance;
        }
        else{
            if(k==0) value = fctPotentiel(distance);
            else value -= fctPotentiel(distance);
        }
    }
    if(selectionMode != 0 && selectionMode != 3) value = fctPotentiel(value);
    if(value > seuil) output = true;
    else output = false;
    return output;
}

// Matching Square en fonction de la valeur de 4 points du carrée si In or Out de la surface implicite
void marchingSquare(float i,float j,bool p1,bool p2,bool p3, bool p4){
    float pasX = (float)glutGet(GLUT_WINDOW_WIDTH)/m;
    float pasY = (float)glutGet(GLUT_WINDOW_HEIGHT)/n;
    glColor3f(255,255,255);
    glBegin(GL_POLYGON);
    if(p1 && !p2 && !p3 && !p4){
        glVertex2f(i-pasX,j-pasY);
        glVertex2f(i-pasX,j-pasY/2.0);
        glVertex2f(i-pasX/2.0,j-pasY);
    }
    else if(!p1 && !p2 && p3 && !p4){
        glVertex2f(i-pasX/2.0,j-pasY);
        glVertex2f(i,j-pasY/2.0);
        glVertex2f(i,j-pasY);
    }
     
    
    else if(p1 && !p2 && p3 && !p4){
        glVertex2f(i-pasX,j-pasY);
        glVertex2f(i-pasX,j-pasY/2.0);
        glVertex2f(i,j-pasY/2.0);
        glVertex2f(i,j-pasY);
    }

    else if(!p1 && !p2 && !p3 && p4){
        glVertex2f(i-pasX/2.0,j);
        glVertex2f(i,j);
        glVertex2f(i,j-pasY/2.0);
    }
    else if(p1 && !p2 && !p3 && p4){
        glVertex2f(i-pasX,j-pasY);
        glVertex2f(i-pasX,j-pasY/2.0);
        glVertex2f(i-pasX/2.0,j-pasY);
        glEnd();
        glBegin(GL_POLYGON);
        glVertex2f(i-pasX/2.0,j);
        glVertex2f(i,j);
        glVertex2f(i,j-pasY/2.0);
    }
     
   else if(!p1 && !p2 && p3 && p4){
        glVertex2f(i-pasX/2.0,j-pasY);
        glVertex2f(i-pasX/2.0,j);
        glVertex2f(i,j);
        glVertex2f(i,j-pasY);
    }
    
    else if(p1 && !p2 && p3 && p4){
        glVertex2f(i-pasX,j-pasY);
        glVertex2f(i-pasX,j-pasY/2.0);
        glVertex2f(i-pasX/2.0,j);
        glVertex2f(i,j);
        glVertex2f(i,j-pasY);
    }
    
    else if(!p1 && p2 && !p3 && !p4){
        glVertex2f(i-pasX,j-pasY/2.0);
        glVertex2f(i-pasX,j);
        glVertex2f(i-pasX/2.0,j);
    }
    
    else if(p1 && p2 && !p3 && !p4){
        glVertex2f(i-pasX,j-pasY);
        glVertex2f(i-pasX,j);
        glVertex2f(i-pasX/2.0,j);
        glVertex2f(i-pasX/2.0,j-pasY);
    }
    
    else if(!p1 && p2 && p3 && !p4){
        glVertex2f(i-pasX,j-pasY/2.0);
        glVertex2f(i-pasX,j);
        glVertex2f(i-pasX/2.0,j);
        glEnd();
        glBegin(GL_POLYGON);
        glVertex2f(i-pasX/2.0,j-pasY);
        glVertex2f(i,j-pasY/2.0);
        glVertex2f(i,j-pasY);
    }
    else if(p1 && p2 && p3 && !p4){
        glVertex2f(i-pasX,j-pasY);
        glVertex2f(i-pasX,j);
        glVertex2f(i-pasX/2.0,j);
        glVertex2f(i,j-pasY/2.0);
        glVertex2f(i,j-pasY);
    }
     
   else if(!p1 && p2 && !p3 && p4){
        glVertex2f(i-pasX,j-pasY/2.0);
        glVertex2f(i-pasX,j);
        glVertex2f(i,j);
        glVertex2f(i,j-pasY/2.0);
    }
    
    else if(p1 && p2 && !p3 && p4){
        glVertex2f(i-pasX,j-pasY);
        glVertex2f(i-pasX,j);
        glVertex2f(i,j);
        glVertex2f(i,j-pasY/2.0);
        glVertex2f(i-pasX/2.0,j-pasY);
    }
    
    else if(!p1 && p2 && p3 && p4){
        glVertex2f(i-pasX,j-pasY/2.0);
        glVertex2f(i-pasX,j);
        glVertex2f(i,j);
        glVertex2f(i,j-pasY);
        glVertex2f(i-pasX/2.0,j-pasY);
    }
    else if(p1 && p2 && p3 && p4){
        glVertex2f(i-pasX,j-pasY);
        glVertex2f(i-pasX,j);
        glVertex2f(i,j);
        glVertex2f(i,j-pasY);
    }
    else if(!p1 && !p2 && !p3 && !p4){
        glColor3f(0,0,0);
        glVertex2f(i-pasX,j-pasY);
        glVertex2f(i-pasX,j);
        glVertex2f(i,j);
        glVertex2f(i,j-pasY);
    }
    glEnd();
}

void traceGrille(){
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    float pasX = (float)glutGet(GLUT_WINDOW_WIDTH)/m;
    float pasY = (float)glutGet(GLUT_WINDOW_HEIGHT)/n;
    float result;
    bool value;
    bool values[n];
    glColor3f(255,255,255);
    if(N > 0){
        for(float i=0 ; i <= glutGet(GLUT_WINDOW_WIDTH); i+=pasX){ // Calcul de la valeur en tout point de l'affichage
            int l = 0;
            for(float j=0;j <= glutGet(GLUT_WINDOW_HEIGHT);j+=pasY){
                result = calculPotentiel(i,j);
                if(l>0){
                    if(i!=0){ // Pour tte lignes (sauf la premiere), on calcul le matching Square
                        marchingSquare(i,j,values[l-1],values[l],value,result);
                    }
                    values[l-1] = value;
                }
                value = result;
                l++;
            }
            values[l-1] = result;
        }
    }
}


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
    traceGrille();
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

// Variation des paramètres (Seuil, Rayon, ... ) changeValue<Type>(nomVar, Pas incrémentation, valeur maximale ou minimale si pas < 0)
template <typename T>
void changeValue(T& var,const T& incre,const T& max)
{
    if(incre > 0 && var + incre <= max) var += incre;
    else if(incre < 0 && var + incre >= max) var +=incre;
}


void clavier(unsigned char touche,int x,int y)
{
  switch (touche)
    {
    case 'q' : /*la touche 'q' permet de quitter le programme */
        exit(0);
    case 'a' :
        changeValue<char>(selectionMode,1,3);
        break;
    case 'z' :
        changeValue<char>(selectionMode,-1,0);
        break;
    case 'e' :
        changeValue<char>(selectionFctPotentiel,1,2);
        break;
    case 'r' :
        changeValue<char>(selectionFctPotentiel,-1,0);
        break;
    case 't' :
        changeValue<int>(m,10,glutGet(GLUT_WINDOW_WIDTH));
        break;
    case 'y' :
        changeValue<int>(m,-10,0);
        break;
    case 'g' :
        changeValue<int>(n,10,glutGet(GLUT_WINDOW_HEIGHT));
        break;
    case 'h' :
        changeValue<int>(n,-10,0);
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

void processMenuEvents(int option) {
    switch (option) {
        case 1 :
            selectionFctPotentiel = 0; break; // Murakami
        case 2 :
            selectionFctPotentiel = 1; break; // Nishimura
        case 3 :
            selectionFctPotentiel = 2; break; // Wyvill
        case 4 :
            selectionMode = 0; break; // Compositio
        case 5 :
            selectionMode = 1; break; // Intersection
        case 6 :
            selectionMode = 2; break; // Union
        case 7 :
            selectionMode = 3; break; // Différence
        case 8 :
            changeValue<int>(m,10,500); // Augmentation résolution largeur
            break;
        case 9 :
            changeValue<int>(m,-10,0); // Diminution résolution largeur
            break;
        case 10 :
            changeValue<int>(n,10,500); // Augmentation résolution hauteur
            break;
        case 11 :
            changeValue<int>(n,-10,0); // Diminution résolution hauteur
            break;
        case 12: changeValue<float>(Ri,10,200);break; // Gestion du rayon d'influence
        case 13: changeValue<float>(Ri,-1,10);break;
        case 14: changeValue<float>(seuil,0.1,1);break; // Gestion du seuil
        case 15: changeValue<float>(seuil,-0.1,0);break;
        case 16: exit(0);break; // Quitter application
    }
}

void createGLUTMenus() {

    int menu;

    // create the menu and
    // tell glut that "processMenuEvents" will
    // handle the events
    menu = glutCreateMenu(processMenuEvents);

    //add entries to our menu
    glutAddMenuEntry("Murakani",1);
    glutAddMenuEntry("Nishimura",2);
    glutAddMenuEntry("Wyvill",3);
    glutAddMenuEntry("Composition",4);
    glutAddMenuEntry("Intersection",5);
    glutAddMenuEntry("Union",6);
    glutAddMenuEntry("Difference", 7);
    glutAddMenuEntry("Augmenter M",8);
    glutAddMenuEntry("Diminuer M", 9);
    glutAddMenuEntry("Augmenter N", 10);
    glutAddMenuEntry("Diminuer N", 11);
    glutAddMenuEntry("Augmenter Rayon",12);
    glutAddMenuEntry("Diminuer Rayon",13);
    glutAddMenuEntry("Augmenter seuil",14);
    glutAddMenuEntry("Diminuer seuil",15);
    glutAddMenuEntry("Quitter",16);
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
    glutMotionFunc(Motion);
    glutKeyboardFunc(clavier);
    createGLUTMenus();
    glutPostRedisplay();
    glutMainLoop();
    
    return 0;
}
