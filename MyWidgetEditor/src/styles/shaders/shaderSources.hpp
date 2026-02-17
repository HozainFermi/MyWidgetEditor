#ifndef SHADER_SOURCES_H
#define SHADER_SOURCES_H

namespace ShaderSources {

	inline	const char* Test_vertex= R"row(
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


out vec2 TexCoord;

void main(){
		
	gl_Position = projection * view * model * vec4(aPos, 1.0f);	
	TexCoord = aTexCoord;
}	
)row";

	

	inline	const char* Test_fragment = R"row(
#version 330 core

out vec4 FragColor;

//in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float u_time;

void main(){
	FragColor = vec4(100.0/255, 20.0/255, 150.0/255 ,1.0);	
	
}
)row";
	//mix(texture(texture1, TexCoord), texture(texture2, vec2(1-TexCoord.x, TexCoord.y)), 0.2);
	//texture(texture1, vec2(TexCoord.x+desturbTex.r*sin(u_time), TexCoord.y) )

	inline	const char* LightShader_vertex = R"row(
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;

void main(){
		
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
	TexCoord = aTexCoord;
}	
)row";



	inline	const char* LightShader_fragment = R"row(
#version 330 core
out vec4 FragColor;
  
uniform vec3 objectColor;
uniform vec3 lightColor;

void main()
{
    FragColor = vec4(lightColor * objectColor, 1.0);
}
)row";


	inline	const char* Test_fragmentYellow = R"row(
#version 330 core

out vec4 FragColor;

void main(){
    FragColor = vec4(226.0/255, 200.0/255, 24.0/255, 1.0);
}

)row";
}

#endif