#include "./res/shaders/default/default_lighting.glsl"

uniform vec3 _postColor;
uniform vec4 _circleColor;
uniform sampler2D _overlayTexture;

uniform vec3  _pulsePositions[3];
uniform float _pulseTimes[3];

vec3 textureWithFalloff(vec3 impulsePos, float scale, float falloffDistance){
  float dist = length(impulsePos - FragPos);
  float value = length(dist + time) * 10;
  int valueInt = int(value);
  if (valueInt % 2 == 0){
  //  return vec3(0, 0, 0);
  }

  float percentage = (falloffDistance - dist) / falloffDistance;
  if (percentage < 0){
    return vec3(0, 0, 0);
  }
  return 0.4 * texture(_overlayTexture, vec2(TexCoord.x * 10 * scale , TexCoord.y * 10  * scale)).rgb * percentage * 2;

}
void main(){

  vec4 color = vec4(1, 1, 1, 1);
  vec3 normalVec = vec3(0, 0, 0);

  mainAlgorithm(color, normalVec);

  float direction = dot(normalVec, vec3(0, 1, 0));
  float xzDistance = length(FragPos.xz - _postColor.xz);


  float intensity = 0.2;
  const float RING_RADIUS = 0.5;
  if ((direction > 0.1  && xzDistance < RING_RADIUS)){
    color.r = color.r + (_circleColor.r  * intensity);
    color.g = color.g + (_circleColor.g  * intensity);
    color.b = color.b + (_circleColor.b  * intensity);

    if (xzDistance > (RING_RADIUS - 0.05)){
      color = _circleColor;
    }
  }


  //if (color.r > 0.5 && color.g < 0.8){
  float timeOffset = 0.5 * (time);
  vec4 newColor = texture(_overlayTexture, normalize(vec2(TexCoord.x + timeOffset, TexCoord.y)));
  //   color.rgb = newColor.rgb;
  //}

  //if (dot(normalVec, vec3(0, 1, 0)) < 0.2){
  //  //discard;
  //}

  
  vec3 effectColor = textureWithFalloff(_postColor, 1.0, 2);

  if (color.r < 0.3 && color.g < 0.3 && color.b < 0.3){
    color = color + vec4(effectColor.rgb, 0);
  }


  vec4 extraColor = vec4(0, 0, 0, 0);

  if (color.r < 0.3 && color.g < 0.3 && color.b < 0.3){
    const float PULSE_DURATION = 0.5;
    const float PULSE_DISTANCE = 2;

    for (int i = 0; i < 3; i++){
      float distance = length(FragPos - _pulsePositions[i]);
      float distancePercentage = max(0, (PULSE_DISTANCE - 1) / PULSE_DISTANCE);

      float elapsedTime = time - _pulseTimes[i];
      float percentage = max(0, (PULSE_DURATION - elapsedTime) / PULSE_DURATION);

      vec3 pulseColor = textureWithFalloff(_pulsePositions[i], percentage, 5);
      if (distance < PULSE_DISTANCE || distance < 10000000){
        extraColor = extraColor + (distancePercentage * percentage) * vec4(pulseColor.rgb, 0);
      }
    }
  }
  

  FragColor = color + vec4(0.2 * newColor.x, 0.2 * newColor.y, 0.2 * newColor.z, 0) + extraColor;
}
