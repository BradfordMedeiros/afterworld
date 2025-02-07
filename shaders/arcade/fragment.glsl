#version 330 core

in vec2 TexCoord;
in vec2 FragPos;
uniform sampler2D textureData;
uniform bool forceTint;
uniform vec4 tint;
uniform vec4 encodedid;
uniform float time;
uniform vec2 resolution;


layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 FragColor2;

void main(){
  if (forceTint){
    FragColor = tint;
    return;
  }


  vec4 texColor = texture(textureData, vec2(TexCoord.x, TexCoord.y));



  vec2 lightLocation = vec2(500 + 100 * (1 + cos(time * 0.1)), 1000 * 0.5);
  float distanceToLight = 200 * (1 / distance(gl_FragCoord.xy, lightLocation));
  vec4 lightColor = vec4(0, 0, distanceToLight, 1);

  FragColor = texColor * tint * (vec4(lightColor.xyz, 1) + vec4(0.8, 0.8, 0.8, 0));
  FragColor2 = encodedid;
}
