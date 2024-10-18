#version 450

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

uniform mat4 MVP;
uniform float angle; // Angle hyperboloique 
uniform float deformation; // Coefficient de déformation D = 1.0 : Non defom
uniform int MAXMERID; // Nombre de point de chaque cercle du cylindre
uniform int status; // Choix Transfo
uniform float thetaMax; // Angle rotation theta
uniform vec2 deforInter; // Intervalle de deformation
uniform int rotationChoix; // Choix rotation X - Y - Z
//uniform mat4 MScale;

layout(location = 0) in vec3 position; // le location permet de dire de quel flux/canal on récupère les données (doit être en accord avec le location du code opengl)
layout(location = 3) in vec3 normale;
out vec3 fragPosition;


mat3 rotationX( in float ang ) {
	return mat3(1,  0,	      0,
			 	0,	cos(ang), -sin(ang),
				0, 	sin(ang), cos(ang));
}


mat3 rotationY( in float ang ) {
	return mat3(cos(ang),  0, sin(ang),
			 	0,		   1, 0,
				-sin(ang), 0, cos(ang));
}


mat3 rotationZ( in float ang ) {
	return mat3(cos(ang), -sin(ang),	0,
			 	sin(ang), cos(ang),     0,
                0,        0,            1);
}


float sTaper(float x){
    if(x < deforInter.x) return 1;
    else if(deforInter.x <= x && x <= deforInter.y) return 1-0.5*(x-deforInter.x)/(deforInter.y-deforInter.x);
    else return 0.5;
}

mat3 taperTrans(vec4 position){
    return mat3(sTaper(position.x), 0, 0,
                0, sTaper(position.y),0,
                0, 0, sTaper(position.z));
}

// Fonction de calcul de l'angle pour Vortex - Twister
float rVorTwi(float z){
    if(z < deforInter.x) return 0;
    else if(deforInter.x <= z && z <= deforInter.y) return (z-deforInter.x)/(deforInter.y-deforInter.x) * thetaMax;
    else return thetaMax;
}
// Twist Transformation en fonction du choix de l'axe de rotation
mat3 twistTrans(vec4 position, int choix){
    float rValue = rVorTwi(position.z);
    return (choix == 1) ? rotationX(rValue) : (choix == 2) ? rotationY(rValue) : rotationZ(rValue);
}

// Vortex Transformation en fonction du choix de l'axe de rotation
mat3 vortexTrans(vec4 position, int choix){
    float alpha;
    if(choix == 1){
        alpha = rVorTwi(position.x)*exp(-(pow(position.y,2)+pow(position.z,2)));
        return rotationX(alpha);
    }
    else if(choix == 2){
        alpha = rVorTwi(position.y)*exp(-(pow(position.x,2)+pow(position.z,2)));
        return rotationY(alpha);
    }
    else{
        alpha = rVorTwi(position.z)*exp(-(pow(position.x,2)+pow(position.y,2)));
        return rotationZ(alpha);
    }
}




void main(){
	vec4 pos = vec4(position,1.); // Recupere la position d'origine du cylindre
    // Hyperboloide
	if(pos.y == 1){
		pos.xyz = pos.xyz * rotationY(angle*3.1415926535/180);
	}
    // Herison Deformation en xyz
    if(mod(round(acos(pos.z)*MAXMERID/(2*M_PI)),2) == 0){
       pos.xyz =  pos.xyz * deformation;
    }
    
    // Deformation Vortex - Taper - Twister - None
    switch(status){
        case 0 : break;
        case 1 : pos.xyz = pos.xyz*taperTrans(pos); break;
        case 2 : pos.xyz = pos.xyz*twistTrans(pos,rotationChoix); break;
        case 4 : pos.xyz = pos.xyz*vortexTrans(pos,rotationChoix); break;
    }     
	gl_Position = MVP*pos; // Mise a jour de la position
	fragPosition = position;
}


