#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in int aBoneIndex[4];
layout (location = 8) in float aBoneWeight[4];

uniform mat4 model;
uniform mat4 projview;
uniform mat4 bones[100];
uniform bool hasBones;

uniform mat4 lightsprojview;

out vec2 TexCoord;
out vec3 Normal;
out mat3 TangentToWorld;
out vec3 FragPos;
out vec4 FragLight;
out vec4 sshadowCoord;

out vec4 glFragPos;
flat out vec4 overcolor;
uniform bool showBoneWeight;
uniform bool useBoneTransform;
uniform vec3 mouseCoordVal;

void main(){
  vec4 modelPosition = model * vec4(aPos.xyz, 1.0);
  gl_Position = ( projview * modelPosition );
  TexCoord = aTexCoords;
  Normal = mat3(transpose(inverse(model))) * aNormal;  
  FragPos = modelPosition.xyz;
  sshadowCoord = lightsprojview * vec4(FragPos, 1.0);
  glFragPos = gl_Position ;
} 
