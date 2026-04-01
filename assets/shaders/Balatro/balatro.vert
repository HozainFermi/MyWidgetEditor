#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 fragCoord;
uniform vec2 iResolution;

uniform mat4 ortho_model;
uniform mat4 ortho_view;
uniform mat4 ortho_projection;

void main()
{
    fragCoord = aTexCoords * iResolution; 
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    //fragCoord = aPos.xy;    
    //gl_Position = ortho_projection * ortho_view * ortho_model *vec4(aPos, 1.0);
}