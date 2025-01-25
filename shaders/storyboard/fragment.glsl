#version 330 core

in vec2 TexCoord;
uniform sampler2D textureData;
uniform bool forceTint;
uniform vec4 tint;
uniform vec4 encodedid;
uniform float time;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 FragColor2;

float noiseIntensity = 0;
float noise(in vec2 st){
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

void main(){
  if (forceTint){
    FragColor = tint;
    return;
  }

  float noiseValue = noise(TexCoord + time * 0.00001) * 0.3;

  vec4 texColor = texture(textureData, vec2(TexCoord.x + cos(0.05 * time), TexCoord.y + sin(0.15 * time)));
  texColor.b *= 2;

  float brightness = 0.2;
  texColor.r *= brightness;
  texColor.g *= brightness;
  texColor.b *= brightness;

  FragColor = texColor * tint + vec4(noiseValue, noiseValue, noiseValue, 0);
  
  FragColor2 = encodedid;
}
