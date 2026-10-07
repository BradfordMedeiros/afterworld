#include "./res/shaders/default/default_lighting.glsl"

vec2 kaleidoscopeUv(vec2 uv) {
  const float TAU = 6.28318530718;
  const float SECTORS = 10.0;

  vec2 p = (uv - 0.5) * 2.0;
  float radius = length(p);
  float angle = atan(p.y, p.x);
  angle += time * 0.025 + 0.48 * sin(radius * 9.0 - time * 0.11);

  float sectorAngle = TAU / SECTORS;
  float foldedAngle = abs(mod(angle + sectorAngle * 0.5, sectorAngle) - sectorAngle * 0.5);
  float warpedRadius = radius * (1.0 + 0.045 * sin(radius * 25.0 - time * 0.16));
  vec2 folded = warpedRadius * vec2(cos(foldedAngle), sin(foldedAngle));

  folded += 0.025 * vec2(
    sin(folded.y * 19.0 + time * 0.08),
    sin(folded.x * 19.0 - time * 0.07)
  );
  return 0.5 + folded * 0.68;
}

void main() {
  vec4 baseColor = vec4(1.0);
  vec3 normalVec = vec3(0.0);
  mainAlgorithm(baseColor, normalVec, vec2(0.0), kaleidoscopeUv(TexCoord));

  float pulse = 0.5 + 0.5 * sin(length(TexCoord - 0.5) * 22.0 - time * 0.12);
  vec3 prismTint = 0.88 + 0.12 * cos(vec3(0.0, 2.1, 4.2) + time * 0.1 + pulse * 2.0);
  FragColor.rgb = baseColor.rgb * prismTint;
  FragColor.a = baseColor.a;

  if (FragColor.r + FragColor.g + FragColor.b > bloomThreshold) {
    BloomColor = vec4(FragColor.rgb, 1.0);
  } else {
    BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
  }
}
