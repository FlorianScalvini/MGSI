#version 450


in vec3 fragPosition;
in vec3 fragNormale;
in vec2 UV;
uniform mat4 Model;

uniform float ambient;
uniform float materialShininess;
uniform vec3 lightPos;
uniform vec3 cameraPosition;
uniform vec3 attenuation;
out vec4 finalColor;

layout(binding=0) uniform sampler2D texture1;  
layout(binding=1) uniform sampler2D texture2;



void main() {

    vec4 color = texture(texture1,UV);
    vec4 normal =  texture(texture2,UV);
    vec3 fragNormTrans = normalize(vec3(transpose(inverse(Model))*(normal + vec4(fragNormale,1.))));
    vec3 fragposTrans = vec3(Model*vec4(fragPosition,1.));
    vec3 lightDir = normalize(lightPos - fragposTrans);
 
    float diffuse = max(dot(fragNormTrans, lightDir), 0.0);
    vec3 dirCamera = normalize(cameraPosition - fragposTrans);
    vec3 reflexionDir = reflect(-lightDir, fragNormTrans);
    float speculaire = pow(max(dot(cameraPosition, reflexionDir),0.0),materialShininess);
    float distanceLight = length(lightPos - fragPosition);
    float fAttenuation = 1 / (attenuation.x + attenuation.y*distanceLight + attenuation.z*pow(distanceLight,2)); // Formule de l'attenuation
    finalColor = ambient+(diffuse + speculaire)*fAttenuation*color;
    // affichage de la position comme donnée de couleur peut être utilisé pour le debugage



}

