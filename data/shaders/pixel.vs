#version 330 core

in vec3 a_vertex;
in vec3 a_normal;
in vec2 a_coord;

uniform mat4 u_model;
uniform mat4 u_viewprojection;

//This will store interpolated variables for the pixel shader
out vec3 v_normal;
out vec3 v_world_position;
out vec2 v_uv;

uniform float u_time;

void main()
{		
	//calcule the normal in camera space (the NormalMatrix is like ViewMatrix but without traslation)
	v_normal = (u_model * vec4( a_normal, 0.0) ).xyz;

	//calculate the vertex in object space
	vec3 position = a_vertex;
	v_world_position = (u_model * vec4( position, 1.0) ).xyz;

	//store the texture coordinates
	v_uv = a_coord;

	//calcule the position of the vertex using the matrices
	gl_Position = u_viewprojection * vec4( v_world_position, 1.0 );
}
