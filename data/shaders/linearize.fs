#version 330 core

//Varying variables
in vec2 v_uv;

//Uniforms
uniform vec2 u_camera_nearfar;
uniform sampler2D u_texture; //depth map
uniform float u_shadow_index;
uniform float u_num_shadows;

//Output
out vec4 FragColor;

void main()
{
	//Shadow atlas coordinates
	vec2 shadow_uv = v_uv;
	shadow_uv.x = (shadow_uv.x + u_shadow_index)/u_num_shadows;

	float n = u_camera_nearfar.x;
	float f = u_camera_nearfar.y;
	float z = texture2D(u_texture,shadow_uv).x;

	if( n == 0.0 && f == 1.0 )
		FragColor = vec4(z);
	else
		FragColor = vec4( n * (z + 1.0) / (f + n - z * (f - n)) );
}