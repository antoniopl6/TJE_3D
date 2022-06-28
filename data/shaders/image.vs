#version 330 core

in vec3 a_vertex;
in vec2 a_coord;

uniform mat4 u_model;
uniform mat4 u_viewprojection;

out vec2 v_uv;

void main()
{	
	//Calculate the vertex in object space
	vec3 world_position = (u_model * vec4( a_vertex, 1.0) ).xyz;

	//Store the texture coordinates
	v_uv = a_coord;
	
	//Calculate the position of the vertex using the matrices
	gl_Position = u_viewprojection * vec4( world_position, 1.0 );
}