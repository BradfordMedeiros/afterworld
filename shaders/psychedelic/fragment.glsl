#include "./res/shaders/default/default_lighting.glsl"

vec3 psychedelicFractal(vec2 uv) {
  const float TAU = 6.28318530718;

  vec2 p = (uv - 0.5) * 4.0;
  float radius = length(p);
  float sector = TAU / 7.0;
  float angle = atan(p.y, p.x);
  angle = abs(mod(angle + sector * 0.5, sector) - sector * 0.5);
  p = radius * vec2(cos(angle), sin(angle));

  float twist = 0.35 * sin(radius * 2.0 - time * 0.35);
  p = mat2(cos(twist), -sin(twist), sin(twist), cos(twist)) * p;
  p += 0.08 * vec2(sin(p.y * 3.0 + time * 0.25), sin(p.x * 3.0 - time * 0.2));

  vec2 z = p * 0.78;
  vec2 c = vec2(-0.38, 0.58) + 0.16 * vec2(cos(time * 0.12), sin(time * 0.17));
  float orbitTrap = 4.0;
  float iterations = 18.0;
  float escaped = 0.0;

  for (int i = 0; i < 18; i++) {
    z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;
    orbitTrap = min(orbitTrap, abs(length(z) - 0.72));

    if (dot(z, z) > 16.0) {
      iterations = float(i);
      escaped = 1.0;
      break;
    }
  }

  float smoothIterations = iterations;
  if (escaped > 0.5) {
    smoothIterations -= log2(max(1.0, log2(length(z))));
  }

  float bands = smoothIterations * 0.18 - log(orbitTrap + 0.015) * 0.55 + radius * 1.8 - time * 0.12;
  vec3 color = 0.5 + 0.5 * cos(TAU * (bands + vec3(0.0, 0.33, 0.67)));
  float glow = exp(-orbitTrap * 4.5);
  color = mix(color * 0.3, color + vec3(0.2, 0.65, 1.0) * glow, 0.75);
  return color;
}

void main() {
  vec4 baseColor = vec4(1.0);
  vec3 normalVec = vec3(0.0);
  mainAlgorithm(baseColor, normalVec, vec2(0.0), TexCoord);

  vec3 fractalColor = psychedelicFractal(TexCoord);
  FragColor.rgb = fractalColor * baseColor.rgb;
  FragColor.a = baseColor.a;

  if (FragColor.r + FragColor.g + FragColor.b > bloomThreshold) {
    BloomColor = vec4(FragColor.rgb, 1.0);
  } else {
    BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
  }
}
