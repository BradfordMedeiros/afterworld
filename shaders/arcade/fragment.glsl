#version 330 core

in vec2 TexCoord;
in vec2 FragPos;
uniform sampler2D textureData;
uniform sampler2D _overlayTexture;

uniform bool forceTint;
uniform vec4 tint;
uniform vec4 encodedid;
uniform float time;

uniform vec2 _resolution;
uniform vec2 _shipPosition;


layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 FragColor2;

void main(){
  if (forceTint){
    FragColor = tint;
    return;
  }


  vec4 texColor = texture(textureData, vec2(TexCoord.x, TexCoord.y));

  float slowTime = time * 0.1;
  int timeInt = int(slowTime);
  float remainder = slowTime - timeInt;

  float rotX = remainder + TexCoord.x;
  float rotY = TexCoord.y;
  vec4 overlayColor = texture(_overlayTexture, vec2(rotX, rotY)) * 0.1;

  vec2 lightLocation = vec2(0.5 * (_shipPosition.x + 1), 0.5 * (_shipPosition.y + 1));

  //vec2 lightLocation = vec2(0.5 * (1 + cos(time)), 0.5);

  vec2 locNdi = vec2(gl_FragCoord.x / _resolution.x, gl_FragCoord.y / _resolution.y);
  float distanceToLight = (1 / (3 * distance(locNdi, lightLocation)));
  vec4 lightColor = vec4(distanceToLight, distanceToLight, distanceToLight, 1);

  FragColor = (texColor + overlayColor) * tint * lightColor;
  FragColor2 = encodedid;
}
