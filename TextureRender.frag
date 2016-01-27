#version 330 core

// Interpolated values from the vertex shaders
in vec2 fragTexCoord;

// output data
out vec3 color;

// Texture sample for the whole mesh
uniform sampler2D texSampler;

void main()
{
    // Output color = color from texture sample specified in the vertex shader,
    // interpolated between all 3 surrounding vertices of the triangle
    color = texture( texSampler, fragTexCoord ).rgb;
}