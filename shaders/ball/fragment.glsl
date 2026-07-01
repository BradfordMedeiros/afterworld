#include "./res/shaders/default/default_lighting.glsl"

uniform vec3 postColor;
uniform vec4 circleColor;

void main(){

  vec4 color = vec4(1, 1, 1, 1);
  vec3 normalVec = vec3(0, 0, 0);

  mainAlgorithm(color, normalVec);

  float direction = dot(normalVec, vec3(0, 1, 0));
  float xzDistance = length(FragPos.xz - postColor.xz);


  float intensity = 0.2;
  const float RING_RADIUS = 0.5;
  if ((direction > 0.1  && xzDistance < RING_RADIUS)){
    color.r = color.r + (circleColor.r  * intensity);
    color.g = color.g + (circleColor.g  * intensity);
    color.b = color.b + (circleColor.b  * intensity);

    if (xzDistance > (RING_RADIUS - 0.05)){
      color = circleColor;
    }
  }

  FragColor = color;
}
