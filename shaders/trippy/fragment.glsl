#include "./res/shaders/default/default_lighting.glsl"

// This isn't accurate...but whatever...fix before using
void main(){
  vec4 color = vec4(1, 1, 1, 1);
  vec3 normalVec = vec3(0, 0, 0);


  vec2 distVec = TexCoord;
  float distance = length(distVec);
  float angle = atan(distVec.y, distVec.x) + cos(time * 0);

  float numSectors = 20;
  float anglePerSector = 2 * PI / numSectors;

  float angleInSector = mod(angle, anglePerSector);

  int sector = int(floor(angle / anglePerSector));

  vec4 tint = vec4(1, 1, 1, 1);
  if (sector % 2 == 0){
  		//tint = vec4(0, 5, 1, 1);
  		angleInSector = anglePerSector - angle;
  }
 

  float sampleX = distance * cos(angleInSector);
  float sampleY  = distance * sin(angleInSector);


  mainAlgorithm(color, normalVec, vec2(0, 0), vec2(sampleX, sampleY));
  if (angleInSector < 0.02 && angleInSector > 0){
  	tint = vec4(1, 5, 0, 1);
  }


  FragColor = tint * color;

}
