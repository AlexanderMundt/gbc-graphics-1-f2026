#version 430 core

in vec3 colour;
out vec4 frag_colour;

uniform vec3 u_colour;

void main()
{
    frag_colour = vec4(colour * u_colour, 1.0);
}