#version 450

uniform vec3 cameraPosition;

uniform sampler2D myTextureSampler;
uniform mat4 MODEL;

in vec3 fragPosition;

out vec4 finalColor;


void main() {

   
    finalColor = vec4(fragPosition,1.);
    //finalColor = vec4(1.0,0.0,0.0,1.);


}
