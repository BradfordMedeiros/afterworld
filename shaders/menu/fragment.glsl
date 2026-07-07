#include "./res/shaders/default/default_lighting.glsl"

uniform float realtime;

bool enableMouseRipple = true;
vec2 wiggleAmount = vec2(0, -1);

vec2 calcRipple(vec2 rippleCoord, float rippleMagnitude){
  float dist = distance(vec2(FragPos.x, FragPos.y), rippleCoord);
  dist = clamp(dist, 0.0, rippleMagnitude);
  vec2 direction = normalize(vec2(rippleCoord.x, rippleCoord.y) - vec2(FragPos.x, FragPos.y));
  float xWeight = direction.x * dist;
  float yWeight = direction.y * dist;
  return vec2(xWeight, yWeight);
}

float noiseIntensity = 0.00;
float noise(in vec2 st){
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

void main(){
  vec4 color = vec4(1, 1, 1, 1);
  vec3 normalVec = vec3(0, 0, 0);

  float movementX = -0.8 + sin(realtime * 0.1) * wiggleAmount.x;
  float movementY = -0.8 + sin(realtime * 0.1) * wiggleAmount.y;

  vec2 totalRipple = vec2(0.0, 0.0);
  for (int i = 0; i < 4; i++){
    vec2 rippleCoord = vec2(movementX + i * 0.4, movementY  + i * 0.4);
    vec2 rippleAmount = calcRipple(rippleCoord, 0.02);
    totalRipple += rippleAmount;
  }
  if (enableMouseRipple){
    //totalRipple += calcRipple(vec2(mouseCoordVal.x, mouseCoordVal.y), 5);
  }

  float noiseValue = noise(TexCoord + realtime * 0.00001) * noiseIntensity;

  totalRipple.y += 0.05 * cos(time * 0.1);

  mainAlgorithm(color, normalVec, totalRipple + vec2(noiseValue, noiseValue));

  vec4 newColor = color;
  newColor.a  = 0.7;
  if (newColor.b < 0.04){
    // newColor.r = 1;
    //newColor.a = 0.2;
   
      vec3 dir = normalize(FragPos - vec3(0, 0.00001, 0.001)) ; 
      //dir = normalize(dir + normalize(lightRot));

      float azimuth = atan(dir.z, dir.x);         // [-π, π]
      float elevation = acos(clamp(dir.y, -1.0, 1.0)); // [0, π]

      // Convert to [0,1] range
      float u = (azimuth + 3.1416) / (2.0 * 3.1416);
      float v = elevation / PI;
      // u goes between 0 an d 1


      vec2 baseUV =  vec2(u + time * 0.02, v);  // These are your texture coordinates


    newColor = color * 7.0 * texture(maintexture, baseUV);
    //newColor.g = 1;
  }else{
    newColor = newColor;
  }

  FragColor = newColor;

}
