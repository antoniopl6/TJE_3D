#version 330 core

in vec2 v_uv;

uniform vec4 u_color;
uniform sampler2D u_texture;
uniform float u_time;
uniform vec4 u_tex_range;

void main()
{
	vec2 uv = v_uv;
	uv.x = (u_tex_range.z) * uv.x + u_tex_range.x;
	uv.y = 1.0 - ((-u_tex_range.w) * uv.y + u_tex_range.y + u_tex_range.w);
	gl_FragColor = u_color * texture2D( u_texture, uv );
}
