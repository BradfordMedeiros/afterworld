#include "./res/shaders/default/default_lighting.glsl"

uniform sampler2D customTexture;
uniform bool hasCustomTexture;
uniform vec2 _pupilLook;

float irisWarpInfluence(vec2 uv, vec2 irisCenter, vec2 pupilOffset){
  float wrappedU = fract(uv.x - irisCenter.x + 0.5) - 0.5;
  vec2 fromPupil = vec2(wrappedU, uv.y - irisCenter.y) - pupilOffset;
  return 1.0 - smoothstep(0.025, 0.13, length(fromPupil));
}

float eyelidCoverage(vec2 uv, vec2 irisCenter, float blink){
  float wrappedU = fract(uv.x - irisCenter.x + 0.5) - 0.5;
  float lidRadius = 0.15;
  float horizontal = clamp(wrappedU / lidRadius, -1.0, 1.0);
  float eyeHalfHeight = lidRadius * sqrt(max(0.0, 1.0 - horizontal * horizontal));
  float vertical = abs(uv.y - irisCenter.y);
  float insideWidth = 1.0 - smoothstep(lidRadius - 0.003, lidRadius + 0.003, abs(wrappedU));
  float eyeArea = insideWidth * (1.0 - smoothstep(eyeHalfHeight - 0.003, eyeHalfHeight + 0.003, vertical));
  float opening = eyeHalfHeight * (1.0 - blink) - 0.003 * blink;
  return eyeArea * blink * smoothstep(opening - 0.002, opening + 0.002, vertical);
}

float eyelidSeam(vec2 uv, vec2 irisCenter, float blink){
  float wrappedU = fract(uv.x - irisCenter.x + 0.5) - 0.5;
  float lidRadius = 0.15;
  float horizontal = clamp(wrappedU / lidRadius, -1.0, 1.0);
  float eyeHalfHeight = lidRadius * sqrt(max(0.0, 1.0 - horizontal * horizontal));
  float vertical = abs(uv.y - irisCenter.y);
  float opening = eyeHalfHeight * (1.0 - blink) - 0.003 * blink;
  float insideWidth = 1.0 - smoothstep(lidRadius - 0.003, lidRadius + 0.003, abs(wrappedU));
  float withinEye = insideWidth * (1.0 - smoothstep(eyeHalfHeight - 0.003, eyeHalfHeight + 0.003, vertical));
  return blink * withinEye * (1.0 - smoothstep(0.001, 0.005, abs(vertical - opening)));
}

void main(){
  vec4 color = vec4(1.0);
  vec3 normalVec = vec3(0.0);
  vec2 pupilOffset = _pupilLook * 0.1;
  float warp = max(
    irisWarpInfluence(TexCoord, vec2(0.706, 0.298), pupilOffset),
    irisWarpInfluence(TexCoord, vec2(0.292, 0.705), pupilOffset)
  );
  vec2 baseUv = TexCoord - pupilOffset * warp;
  mainAlgorithm(color, normalVec, vec2(0.0), baseUv);

  if (hasCustomTexture){
    vec2 pupilUv = TexCoord - pupilOffset;
    vec4 pupil = texture(customTexture, pupilUv);
    color.rgb = mix(color.rgb * 0.5, pupil.rgb, pupil.a * 0.8);
  }

  float blinkPhase = mod(time, 1);
  float blink = smoothstep(0.015, 0.065, blinkPhase) * (1.0 - smoothstep(0.10, 0.22, blinkPhase));
  vec2 upperIrisCenter = vec2(0, 0);
  vec2 lowerIrisCenter = vec2(0, 0);
  float lidCoverage = max(
    eyelidCoverage(TexCoord, upperIrisCenter, blink),
    eyelidCoverage(TexCoord, lowerIrisCenter, blink)
  );
  float lidSeam = max(
    eyelidSeam(TexCoord, upperIrisCenter, blink),
    eyelidSeam(TexCoord, lowerIrisCenter, blink)
  );
  color.rgb = mix(color.rgb, vec3(0.012, 0.018, 0.022), lidCoverage);
  color.rgb = mix(color.rgb, vec3(0.002, 0.003, 0.004), lidSeam * 0.8);

  FragColor = color;
  BloomColor = vec4(0.0);
}
