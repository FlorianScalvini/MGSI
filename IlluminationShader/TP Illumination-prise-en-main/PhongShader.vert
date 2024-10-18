

#version 450


uniform mat4 MVP;
uniform mat4 Model;
uniform vec3 lightPos;
uniform vec3 cameraPosition;

layout(location = 0) in vec3 position; // le location permet de dire de quel flux/canal on récupère les données (doit être en accord avec le location du code opengl)
layout(location = 3) in vec3 normales;
layout(location = 2) in vec2 vertexUV;
layout(location = 4) in vec3 tangente;
layout(location = 5) in vec3 bitangente;

out vec3 fragPosition;
out vec3 fragNormale;
out vec2 UV;
void main(){
	gl_Position = MVP*vec4(position, 1.0);
   	fragNormale = normales;
	fragPosition = position;
	UV = vertexUV;
}
