#version 330

in vec3 fragColor;
in vec3 vertexPosition;
in float playercheck;
in vec3 playerPosition1;

out vec4 outputColor;

uniform vec4 vColor;

void main()
{	
	vec4 vTexColor = vec4(fragColor,1.0);
	float fDiffuseIntensity = max(0.0, dot(normalize(vec3(0,1,0)), vec3(-1,-1,-1)));
	outputColor = vTexColor*vec4(vec3(2,2,2)*(0.5+fDiffuseIntensity-(1.0-playercheck)*(distance(vertexPosition,playerPosition1)/1000.0)*0.5), 1.0);
}

