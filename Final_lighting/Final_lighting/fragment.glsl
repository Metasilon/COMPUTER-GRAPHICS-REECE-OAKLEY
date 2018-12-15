#version 330

in vec3 v_out_color;
in vec2 v_out_texcoord;
in vec3 v_out_light;
in vec3 v_out_fragPos;

out vec4 color;

uniform sampler2D color_texture;
uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
	
	//color = texture(color_texture,v_out_texcoord) * vec4(v_out_color, 1.0) ;
	// ambient
    float ambientStrength = 0.1;
   vec3 ambient = ambientStrength * texture(color_texture,v_out_texcoord).rgb;
  	
    // diffuse 
   vec3 norm = normalize(v_out_light);
   vec3 lightDir = normalize(lightPos - v_out_fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * texture(color_texture,v_out_texcoord).rgb;
    
    // specular
    float specularStrength = 0.5;
  vec3 viewDir = normalize(viewPos - v_out_fragPos);
   vec3 reflectDir = reflect(-lightDir, norm);  
   float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
   vec3 specular = specularStrength * spec * lightColor;  
        
   vec3 result = (ambient + diffuse + specular);
  color = vec4(result, 1.0);
  
}