#include "./res/shaders/default/default_vertex.glsl"

uniform sampler2D opacityTexture; 
uniform bool hasOpacityTexture;
uniform vec3 _postColor;
uniform float _windStrength;

const float FLATTEN_START_DISTANCE_SQUARED = 0.25;
const float FLATTEN_END_DISTANCE_SQUARED = 6.25;
const float FLATTEN_HEIGHT = 1.35;

vec3 calcModelPositionOffset(){
  float heightWeight = clamp(aTexCoords.y, 0.0, 1.0);
  vec3 worldPosition = vec3(model * vec4(aPos, 1.0));
  vec2 awayFromBall = worldPosition.xz - _postColor.xz;
  float verticalDistance = worldPosition.y - _postColor.y;
  float distanceSquared = dot(awayFromBall, awayFromBall) + verticalDistance * verticalDistance;
  float flattening = 1.0 - smoothstep(FLATTEN_START_DISTANCE_SQUARED, FLATTEN_END_DISTANCE_SQUARED, distanceSquared);
  float horizontalDistanceSquared = dot(awayFromBall, awayFromBall);
  vec2 bendDirection = horizontalDistanceSquared > 0.0001
    ? awayFromBall * inversesqrt(horizontalDistanceSquared)
    : vec2(0.0);

  float gust = cos(time * 0.8 + worldPosition.x * 0.35 + worldPosition.z * 0.2);
  float sway = 0.3 * _windStrength * heightWeight * (1.0 - flattening) * gust;
  vec3 swayOffset = vec3(sway, 0.0, sway);
  vec3 flattenOffset = vec3(
    bendDirection.x,
    -FLATTEN_HEIGHT,
    bendDirection.y
  ) * (heightWeight * flattening);
  return swayOffset + flattenOffset;
}


void main(){
  coreVertex();
} 
