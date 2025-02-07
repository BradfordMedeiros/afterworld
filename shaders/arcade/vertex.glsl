#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 projection;

out vec2 TexCoord;
out vec2 FragPos;

void main(){
  FragPos = gl_Position.xy;
  gl_Position = projection * model * vec4(aPos.xy, 0.0, 1.0);
  TexCoord = aTexCoords;
} 
