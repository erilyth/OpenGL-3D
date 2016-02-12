#version 330

uniform mat4 MVP;
uniform vec3 objectPosition;
uniform vec3 playerPosition;
uniform float playerAngleY;
uniform float playerAngleXZ;
uniform float isPlayer;
uniform float isNight;

layout (location = 0) in vec3 inPosition; //In position is vertex position
layout (location = 1) in vec3 vertexColor;

out vec3 fragColor;
out vec3 vertexPosition;
out vec3 playerPosition1;
out float playerYAngle;
out float playerXZAngle;
out float playercheck;
out float nightcheck;

void main()
{
	gl_Position = MVP*vec4(inPosition, 1.0);
	vertexPosition = inPosition+objectPosition;
	playercheck=isPlayer;
	nightcheck=isNight;
	playerYAngle=playerAngleY;
	playerXZAngle=playerAngleXZ;
	playerPosition1 = playerPosition;
	fragColor = vertexColor;
}
