#version 330

in vec3 fragColor;
in vec3 vertexPosition;
in float playercheck;
in vec3 playerPosition1;
in float playerYAngle;
in float playerXZAngle;
in float nightcheck;

out vec4 outputColor;

uniform vec4 vColor;

void main()
{	
	vec4 vTexColor = vec4(fragColor,1.0);
	vec3 playerDirection = vec3(10*sin(3.14/180*playerYAngle),-playerXZAngle,10*cos(3.14/180*playerYAngle));
	vec3 vertexDirection = vertexPosition-playerPosition1-vec3(0,+20,0);
	float angle = acos(dot(playerDirection,vertexDirection)/(length(playerDirection)*length(vertexDirection)))*180;
	float fDiffuseIntensity = max(0.0, dot(normalize(vec3(0,1,0)), vec3(-1,-1,-1)));
	float intensity = 0;
	if(angle>=0 && angle<=110){
		intensity=110-angle;
	}
	else if(angle<=0 && angle>=-110){
		intensity=angle+110;		
	}
    float remLight=1;
    if(intensity==0)
        remLight=0;
	if(playercheck!=1.0){
		outputColor = vTexColor*vec4(vec3(2,2,2)*((0.5+fDiffuseIntensity)*intensity/90-remLight*(distance(vertexPosition,playerPosition1)/1000.0)*0.2), 1.0);
	}
	else{
		outputColor = vTexColor;
	}
	if(nightcheck==0)
		outputColor = vec4(fragColor,1.0);
}

