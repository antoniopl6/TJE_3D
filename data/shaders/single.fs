#version 330 core

//Interpolated variables
in vec3 v_world_position;
in vec3 v_normal;
in vec2 v_uv;

//Material factors
uniform vec3 u_albedo_factor;
uniform vec3 u_specular_factor;
uniform vec3 u_occlusion_factor;
uniform vec3 u_emissive_factor;

//Textures
uniform sampler2D u_albedo_texture;
uniform sampler2D u_specular_texture;
uniform sampler2D u_normal_texture;
uniform sampler2D u_occlusion_texture;
uniform sampler2D u_metalness_texture;
uniform sampler2D u_roughness_texture;
uniform sampler2D u_omr_texture;
uniform sampler2D u_emissive_texture;
uniform sampler2D u_shadow_atlas;
uniform int u_textures[8];

//Scene uniforms
uniform vec3 u_camera_position;
uniform float u_time;
uniform vec3 u_ambient_light;
uniform bool u_last_iteration;

//Prefab uniforms
uniform vec4 u_color;
uniform float u_alpha_cutoff;

//Single pass maximum number of lights to render
const int MAX_LIGHTS = 5;

//Global lights uniforms
uniform vec3 u_lights_position[MAX_LIGHTS];
uniform vec3 u_lights_color[MAX_LIGHTS];
uniform float u_lights_intensity[MAX_LIGHTS];
uniform float u_lights_max_distance[MAX_LIGHTS];
uniform int u_lights_type[MAX_LIGHTS];
uniform int u_num_lights;

//Spot lights uniforms
uniform vec3 u_spots_direction[MAX_LIGHTS];
uniform vec2 u_spots_cone[MAX_LIGHTS];

//Directional lights uniforms
uniform vec3 u_directionals_front[MAX_LIGHTS];

//Shadows
uniform bool u_cast_shadows[MAX_LIGHTS];
uniform float u_shadows_index[MAX_LIGHTS];
uniform float u_shadows_bias[MAX_LIGHTS];
uniform mat4 u_shadows_vp[MAX_LIGHTS];
uniform float u_num_shadows;

//Output
out vec4 FragColor;

mat3 cotangent_frame(in vec3 N, in vec3 p, in vec2 uv)
{
	// get edge vectors of the pixel triangle
	vec3 dp1 = dFdx( p );
	vec3 dp2 = dFdy( p );
	vec2 duv1 = dFdx( uv );
	vec2 duv2 = dFdy( uv );
	
	// solve the linear system
	vec3 dp2perp = cross( dp2, N );
	vec3 dp1perp = cross( N, dp1 );
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
	// construct a scale-invariant frame 
	float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
	return mat3( T * invmax, B * invmax, N );
}

vec3 perturbNormal(in vec3 N, in vec3 WP, in vec2 uv, in vec3 normal_pixel)
{
	normal_pixel = normal_pixel * 255./127. - 128./127.;
	mat3 TBN = cotangent_frame(N, WP, uv);
	return normalize(TBN * normal_pixel);
}

float testShadowMap(in float shadow_index, in float num_shadows,in float shadows_bias, in vec3 world_position, in mat4 shadow_vp, in sampler2D shadow_atlas){
	//project our 3D position to the shadowmap
	vec4 proj_pos = shadow_vp * vec4(world_position,1.0);

	//from homogeneus space to clip space
	vec2 shadow_uv = proj_pos.xy / proj_pos.w;

	//from clip space to uv space
	shadow_uv = shadow_uv * 0.5 + vec2(0.5);

	//In case the point we are painting is out of the shadowm frustrum, it doesn't cast a shadow
	if( shadow_uv.x < 0.0 || shadow_uv.x > 1.0 || shadow_uv.y < 0.0 || shadow_uv.y > 1.0 ) return 1.0;

	//Shadow atlas coordinates
	shadow_uv.x = (shadow_uv.x + shadow_index)/num_shadows;

	//read depth from depth buffer in [0..+1] non-linear
	float shadow_depth = texture2D( shadow_atlas, shadow_uv).x;

	//get point depth [-1 .. +1] in non-linear space
	float real_depth = (proj_pos.z - shadows_bias) / proj_pos.w;

	//normalize from [-1..+1] to [0..+1] still non-linear
	real_depth = real_depth * 0.5 + 0.5;

	//In case the point we are painting is before the near or behind the far plane of the light camera, it doesn't cast a shadow
	if(real_depth < 0.0 || real_depth > 1.0) return 1.0;

	//compute final shadow factor by comparing
	float shadow_factor = 1.0;

	//we can compare them, even if they are not linear
	if( shadow_depth < real_depth ) shadow_factor = 0.0;

	//Return shadow factor
	return shadow_factor;
}

vec3 PhongEquation(in int index, in vec3 light_vector, in float light_intensity, in float light_distance, in vec3 normal_vector, in bool light_attenuation)
{
	//Compute vectors
	vec3 L = light_vector;
	vec3 N = normal_vector;
	vec3 R = reflect(-L, N);
	vec3 V = u_camera_position - v_world_position;
    R = normalize(R);
    V = normalize(V);

    //Compute dot products
    float NdotL = clamp(dot(N,L), 0.0, 1.0);
    float RdotV = clamp(dot(R,V), 0.0, 1.0);

	//Shadow factor
	float shadow_factor = 1.0;
	if(u_cast_shadows[index]) shadow_factor = testShadowMap(u_shadows_index[index], u_num_shadows,u_shadows_bias[index], v_world_position, u_shadows_vp[index], u_shadow_atlas);

    //Compute attenuation factor
    float attenuation_factor = 1.0;
    if(light_attenuation)
    {
	    attenuation_factor =  u_lights_max_distance[index] - light_distance;
		attenuation_factor /= u_lights_max_distance[index];
		attenuation_factor = pow(max( attenuation_factor, 0.0 ),2.0);
	}

	//Diffuse factor
	vec3 diffuse_factor = vec3(NdotL);

	//Specular factor
	vec3 specular_factor;
	if(u_textures[1] == 1)
	{
		specular_factor = texture2D(u_specular_texture,v_uv).xyz;
	}
	else if(u_textures[6] == 1)
	{
		vec3 omr = texture2D(u_omr_texture,v_uv).xyz;
	    specular_factor = vec3(omr.z * pow(RdotV, omr.y * 20.0)); //Multiply metalness by a float to reduce specular inaccuracy
	}
	else if(u_textures[4] == 1 && u_textures[5] == 1)
	{
		vec3 metalness_factor = texture2D(u_metalness_texture,v_uv).xyz;
		vec3 roughness_factor = texture2D(u_roughness_texture,v_uv).xyz;
	    specular_factor = roughness_factor * pow(vec3(RdotV), metalness_factor * 20.0); //Multiply metalness by a float to reduce specular inaccuracy
	}
	else
	{
		specular_factor = u_specular_factor;
	}

    //Phong equation
	vec3 light = attenuation_factor * (diffuse_factor + specular_factor) * u_lights_color[index] * light_intensity * shadow_factor;

	//Return light
	return light;
}

void main()
{
	//Alpha mask
	if(u_color.a < u_alpha_cutoff)
		discard;

	//Albedo value
	vec4 color;
	if(u_textures[0] == 1) color = texture2D(u_albedo_texture, v_uv );
	else color = vec4(u_albedo_factor,1.0);

	//Normal value
	vec3 tangent_space_normal;
	if(u_textures[2] == 1) tangent_space_normal = texture2D( u_normal_texture, v_uv ).xyz;

	//Occlusion value
	vec3 ambient_factor;
	if(u_textures[3] == 1) ambient_factor = texture2D( u_occlusion_texture, v_uv ).xyz;
	else if(u_textures[6] == 1) ambient_factor = vec3(texture2D( u_omr_texture, v_uv ).x);
	else ambient_factor = u_occlusion_factor;

	//Interpolated normal
	vec3 interpolated_normal = normalize(v_normal);

	//Normal mapping
	vec3 normal_vector;
	if(u_textures[2] == 1) normal_vector = perturbNormal(interpolated_normal, v_world_position, v_uv, tangent_space_normal);//Normal map
	else normal_vector = interpolated_normal;//Interpolated Normal

	//Set ambient light to phong light
	vec3 phong_light = ambient_factor * u_ambient_light;

	//Single pass for loop
	for( int i = 0; i < MAX_LIGHTS; ++i )
	{
		if(i < u_num_lights)
		{
			//Light intensisty
			float light_intensity = u_lights_intensity[i];

			if(u_lights_type[i] == 0) //point light
			{
				//Light vector
				vec3 light_vector = u_lights_position[i] - v_world_position;

				//Light distance
				float light_distance = length(light_vector);

				//Normalize light vector
				light_vector /= light_distance;

				//Phong Equation
				phong_light += PhongEquation(i, light_vector, light_intensity, light_distance, normal_vector, true);
			}
			else if(u_lights_type[i] == 1)//spot light
			{
				//Light vector
				vec3 light_vector = u_lights_position[i] - v_world_position;

				//Light distance
				float light_distance = length(light_vector);

				//Normalize light vector
				light_vector /= light_distance;

				//Orient spot vector
				vec3 spot_vector = -u_spots_direction[i];

				//Compute the cosine of the angle between previous vectors
				float spot_cosine = dot(light_vector,spot_vector);

				//Check if the pixel is within the cone
				if(spot_cosine >= u_spots_cone[i].y)
				{
					//Light intesity
					light_intensity *= pow(spot_cosine,max(u_spots_cone[i].x,0.0));

					//Phong Equation
					phong_light += PhongEquation(i, light_vector, light_intensity, light_distance, normal_vector, true);
				}

			}
			else if(u_lights_type[i] == 2) //directional light
			{
				//Light vector
				vec3 light_vector = u_directionals_front[i];

				//Light distance
				float light_distance = length(light_vector);

				//Normalize light vector
				light_vector /= light_distance;

				//Phong Equation
				phong_light += PhongEquation(i, light_vector, light_intensity, light_distance, normal_vector, false);
			}			
		}
	}

	//Final color
	color.rgb *= phong_light;

	if(u_last_iteration)
	{
		if(u_textures[7] == 1)
		{
			vec3 emissive_light = u_emissive_factor * texture2D(u_emissive_texture,v_uv).xyz;
			color.rgb += emissive_light;
		}
	}

	//Output
	FragColor = color;
}