#version 330 core

// Interpolated values from the vertex shaders
in vec3 fragColor;

// output data
out vec4 outputColor;

void main()
{
    // Output color = color specified in the vertex shader,
    // interpolated between all 3 surrounding vertices of the triangle
    outputColor.a=1;
    outputColor.x=fragColor.x;
    outputColor.y=fragColor.y;
    outputColor.z=fragColor.z;	
}
