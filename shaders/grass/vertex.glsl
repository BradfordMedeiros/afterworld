#include "./res/shaders/default/default_vertex.glsl"

uniform sampler2D opacityTexture; 
uniform bool hasOpacityTexture;

vec3 calcModelPositionOffset(){
  float distanceToTip = aTexCoords.y;
  const float amount = 0.3;
  vec3 offset = vec3(amount * distanceToTip * cos(1 * time) , 0, amount * distanceToTip * cos(1 * time));
  return offset;
}


void main(){
  coreVertex();
} 
