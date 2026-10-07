#define FragPos ShellFragPos
#define Normal ShellNormal
#define TexCoord ShellTexCoord
#define TangentToWorld ShellTangentToWorld
#define sshadowCoord ShellSshadowCoord
#define vertColor ShellVertColor
#define instanceId ShellInstanceId
#include "./res/shaders/default/default_lighting.glsl"

in float shellHeight;
uniform float _furDensity;
uniform float _furStrandWidth;

float strandHash(vec2 cell) {
  return fract(sin(dot(cell, vec2(127.1, 311.7))) * 43758.5453);
}

void main() {
  vec4 baseColor = vec4(1.0);
  vec3 normalVec = vec3(0.0);
  mainAlgorithm(baseColor, normalVec, vec2(0.0), ShellTexCoord);

  if (shellHeight > 0.0) {
    vec2 strandGrid = ShellTexCoord * vec2(_furDensity * 2.4444, _furDensity);
    vec2 strandCell = floor(strandGrid);
    vec2 localUv = fract(strandGrid);
    float seed = strandHash(strandCell);
    float strandCenter = 0.5 + (strandHash(strandCell + 13.7) - 0.5) * 0.55;
    float strandWidth = mix(_furStrandWidth * 0.5, _furStrandWidth * 1.5, strandHash(strandCell + 27.1));
    float strandLength = mix(0.22, 1.0, strandHash(strandCell + 41.3));
    strandCenter += 0.08 * sin(localUv.y * 3.14 + seed * 6.28);

    if (abs(localUv.x - strandCenter) > strandWidth || shellHeight > strandLength) {
      discard;
    }

    float tip = smoothstep(strandLength - 0.18, strandLength, shellHeight);
    vec3 furTint = mix(vec3(0.62, 0.36, 0.2), vec3(1.15, 0.83, 0.58), seed * 0.55 + tip * 0.25);
    baseColor.rgb *= furTint;
  }

  FragColor = baseColor;
  if (FragColor.r + FragColor.g + FragColor.b > bloomThreshold) {
    BloomColor = vec4(FragColor.rgb, 1.0);
  } else {
    BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
  }
}
