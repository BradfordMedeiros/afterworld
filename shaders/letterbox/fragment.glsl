#version 330 core

in vec2 TexCoord;
uniform sampler2D textureData;
uniform bool forceTint;
uniform vec4 tint;
uniform vec4 encodedid;
uniform float time;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 FragColor2;

float hash(vec2 p){
  return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

void main(){
  if (forceTint){
    FragColor = tint;
    FragColor2 = encodedid;
    return;
  }

  vec2 uv = TexCoord;
  uv.x = uv.x * 0.2 + sin(uv.y * 24.0 + time * 1.4) * 0.003;
  uv.y += time * 0.008;
  vec3 textureColor = texture(textureData, uv).rgb;

  float pulse = 0.5 + 0.5 * sin(TexCoord.x * 18.0 - time * 0.7);
  float scanline = 0.9 + 0.1 * sin(TexCoord.y * 180.0 + time * 2.0);
  float staticNoise = hash(floor(TexCoord * vec2(240.0, 30.0)) + floor(time * 10.0));
  float signal = clamp(dot(textureColor, vec3(0.2, 0.55, 0.25)) * 0.8 + pulse * 0.2, 0.0, 1.0);

  vec3 deepBlue = vec3(0.008, 0.025, 0.06);
  vec3 neonTeal = vec3(0.015, 0.52, 0.64);
  vec3 color = mix(deepBlue, neonTeal, signal) * scanline;
  color += vec3(0.01, 0.09, 0.12) * staticNoise;
  color *= tint.rgb;

  FragColor = vec4(color * 0, 0.5);
  FragColor2 = encodedid;
}
