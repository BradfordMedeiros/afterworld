#include "./res/shaders/default/default_lighting.glsl"

void main(){
  vec4 color = vec4(1, 1, 1, 1);
  mainAlgorithm(color);

  color = vec4(color.r * 0.2, color.g * 0.2, color.b, 1);


  FragColor = color;
}
