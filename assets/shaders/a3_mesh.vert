#version 430 core

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_tcoord;

out vec3 position;
out vec3 normal;
out vec2 tcoord;

uniform mat4 u_world;
uniform mat4 u_mvp;

void main()
{
    position = (u_world * vec4(vertex_position, 1.0)).xyz;
    tcoord = vertex_tcoord;
    normal = normalize(transpose(inverse(mat3(u_world))) * vertex_normal);

    gl_Position = u_mvp * vec4(vertex_position, 1.0);
}