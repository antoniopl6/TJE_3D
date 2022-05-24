#version 330 core

in vec3 a_vertex;

uniform mat4 u_model;
uniform mat4 u_viewprojection;

void main()
{	
	//calcule the screen position of the vertex using the matrices
	vec3 world_position = (u_model * vec4( a_vertex, 1.0) ).xyz;
	gl_Position = u_viewprojection * vec4( world_position, 1.0 );
}