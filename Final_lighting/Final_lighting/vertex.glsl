#version 330

uniform mat4 transform;
uniform mat4 model;

in vec3 pos;
in vec3 color;
in vec2 texcoord;
in vec3 light;

out vec3 v_out_color;
out vec2 v_out_texcoord;
out vec3 v_out_light;
out vec3 v_out_fragPos;

void main()
{
v_out_fragPos = vec3(model * vec4(pos, 1.0));
v_out_color = color;
		v_out_texcoord = texcoord;
	v_out_light = mat3(transpose(inverse(model))) * light;  
	gl_Position = transform * vec4(pos, 1.0);
}
