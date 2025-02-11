#version 330 core

in vec2 TexCoord;
in vec2 FragPos;
uniform sampler2D textureData;
uniform bool forceTint;
uniform vec4 tint;
uniform vec4 encodedid;
uniform float time;

uniform vec2 resolution;
uniform vec2 shipPosition;


layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 FragColor2;

void main(){
  if (forceTint){
    FragColor = tint;
    return;
  }


  vec4 texColor = texture(textureData, vec2(TexCoord.x, TexCoord.y));


  vec2 lightLocation = vec2(0.5 * (shipPosition.x + 1), 0.5 * (shipPosition.y + 1));

  //vec2 lightLocation = vec2(0.5 * (1 + cos(time)), 0.5);

  vec2 locNdi = vec2(gl_FragCoord.x / resolution.x, gl_FragCoord.y / resolution.y);
  float distanceToLight = (1 / (3 * distance(locNdi, lightLocation)));
  vec4 lightColor = vec4(distanceToLight, distanceToLight, distanceToLight, 1);

  FragColor = texColor * tint * lightColor;
  FragColor2 = encodedid;
}
