#version 330 core

uniform mat4 MVP;
uniform vec3 pen;
uniform vec3 fontColor;

in vec3 vertexPosition;
in vec3 vertexNormal;

out vec3 fragColor;

void main ()
{
    gl_Position = MVP * (vec4(vertexPosition, 1.0) + vec4(pen, 1.0));
    // fragColor = vec3((vertexNormal.x+1)/2,(vertexNormal.y+1)/2,(vertexNormal.z+1)/2);
    fragColor = fontColor;
}