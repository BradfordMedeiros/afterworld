#version 430 

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in mat3 TangentToWorld;
in vec4 sshadowCoord;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BloomColor;
layout(location = 2) out vec4 EncodeId;
layout(location = 3) out vec4 UVCoords;

uniform sampler2D maintexture;
uniform sampler2D emissionTexture;
uniform sampler2D opacityTexture;
uniform sampler2D lightDepthTexture;
uniform samplerCube cubemapTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D normalTexture;

uniform vec4 tint;
uniform vec3 cameraPosition;

uniform bool enableLighting;
uniform bool enableShadows;
uniform bool enableDiffuse;
uniform bool enableSpecular;
uniform bool hasDiffuseTexture;
uniform bool hasEmissionTexture;
uniform bool hasOpacityTexture;
uniform bool hasCubemapTexture;
uniform bool hasRoughnessTexture;
uniform bool hasNormalTexture;
uniform float shadowIntensity;

uniform vec2 textureOffset;
uniform vec2 textureTiling;
uniform vec2 textureSize;

#define MAX_LIGHTS 32
uniform int numlights;
uniform vec3 lights[MAX_LIGHTS];
uniform vec3 lightscolor[MAX_LIGHTS];
uniform vec4 lightscoord[MAX_LIGHTS];
uniform vec3 lightsdir[MAX_LIGHTS];
uniform mat4 lightsdirmat[ $LIGHT_BUFFER_SIZE ];
uniform vec3 lightsatten[MAX_LIGHTS];
uniform float lightsmaxangle[MAX_LIGHTS];
uniform float lightsangledelta[MAX_LIGHTS];
uniform bool lightsisdir[MAX_LIGHTS];
uniform int voxelcellwidth;
uniform vec4 lightstexindex[ $LIGHT_BUFFER_SIZE ];

//uniform mat4 lightsprojview[MAX_LIGHTS];

uniform vec3 ambientAmount;
uniform float emissionAmount;
uniform float discardTexAmount;
uniform float realtime;
uniform float bloomThreshold;
//uniform vec3 mouseCoordVal;
uniform bool enableAttenutation;

//uniform int textureid;
//uniform vec4 encodedid;



int getNumLights(){
  return min(numlights, MAX_LIGHTS);
}

float calcAttenutation(int lightNumber){
  vec3 lightPos = lights[lightNumber];
  float distanceToLight = length(lightPos - FragPos);
  vec3 attenuationTerms = lightsatten[lightNumber];
  float constant = attenuationTerms.x;
  float linear = attenuationTerms.y;
  float quadratic = attenuationTerms.z;
  float attenuation = enableAttenutation ? (1.0 / (constant + (linear * distanceToLight) + (quadratic * (distanceToLight * distanceToLight)))) : 1;
  return attenuation;
}


int numCellsDim = 2;

float convertBase(float value, float fromBaseLow, float fromBaseHigh, float toBaseLow, float toBaseHigh){
  return ((value - fromBaseLow) * ((toBaseHigh - toBaseLow) / (fromBaseHigh - fromBaseLow))) + toBaseLow;
}


int xyzToIndex(int x, int y, int z){
  return x + (numCellsDim * y) + (numCellsDim * numCellsDim * z);
}
vec3 lookupAmbientLight(){
  int numCellsPerRow = 2;
  int totalCells = numCellsPerRow * numCellsPerRow * numCellsPerRow;

  int cellXInt = int(round(FragPos.x)) / voxelcellwidth;                 // cellYInt is 24
  int remainingCellX = cellXInt % numCellsDim;  // 0 

  int cellYInt = int(round(FragPos.y)) / voxelcellwidth;                 // cellYInt is 24
  int remainingCellY = cellYInt % numCellsDim;  // 0 

  int cellZInt = int(round(FragPos.z)) / voxelcellwidth;                 // cellYInt is 24
  int remainingCellZ = cellZInt % numCellsDim;  // 0 

  int finalIndex = xyzToIndex(remainingCellX, remainingCellY, remainingCellZ);

  return vec3(0, 0, 0);
}

vec3 calculatePhongLight(vec3 normal){
  if (lightscoord[0].x > 23434){
    vec4 newNormal = vec4(normal.xyz, 1);
    newNormal =  lightsdirmat[0] * newNormal;
    if (newNormal.x > 234234 || lightstexindex[0].x == 112302){
      discard;
    }
  }
  vec3 ambient =  lookupAmbientLight();   
  vec3 totalDiffuse  = vec3(0, 0, 0);     
  vec3 totalSpecular = vec3(0, 0, 0);     

  for (int i = 0; i < getNumLights(); i++){
    vec3 lightPos = lights[i];
    vec3 lightDir = lightsisdir[i] ?  lightsdir[i] : normalize(lightPos - FragPos);

    float angle = dot(lightDir, normalize(-lightsdir[i]));

    float angleFactor = 1;
    float minAngle = lightsmaxangle[i];
    float maxAngle = minAngle + lightsangledelta[i];
    float angleAmount = mix(minAngle, maxAngle, angle);
    if (angle < maxAngle){
      if (angle < minAngle){
        continue;
      }
      angleFactor = (angle - minAngle) / (maxAngle - minAngle);
    }

    vec3 diffuse = max(dot(normal, lightDir), 0.0) * lightscolor[i];
    vec3 viewDir = normalize(cameraPosition - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);  
    vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), 32) * vec3(1.0, 1.0, 1.0);  
    float attenuation = calcAttenutation(i);

    totalDiffuse = totalDiffuse + angleFactor * (attenuation * diffuse * lightscolor[i]);
    totalSpecular = totalSpecular + angleFactor * (attenuation * specular * lightscolor[i]);
  }

  vec3 diffuseValue = enableDiffuse ? totalDiffuse : vec3(0, 0, 0);
  vec3 specularValue = enableSpecular ? totalSpecular : vec3(0, 0, 0);
  vec3 color = ambient + diffuseValue + specularValue;
  return color;
}


//////////
// Background: https://www.scitepress.org/Papers/2021/102527/102527.pdf
 //  https://www.scitepress.org/Papers/2021/102527/102527.pdf

const float PI = 3.14159265359;

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

float noiseIntensity = 0;
float noise(in vec2 st){
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

void main(){
    //EncodeId = vec4(encodedid.x, encodedid.y, encodedid.z, encodedid.w);
    //UVCoords = vec4(TexCoord.x, TexCoord.y, textureid, 0);
    
    if (hasCubemapTexture){
      FragColor = tint * texture(cubemapTexture, FragPos);
      return;
    }

    if (voxelcellwidth < -1){
      discard;
    }

    vec3 shadowCoord = sshadowCoord.xyz * 0.5 + 0.5;
    vec2 offsetTexCoord = vec2(TexCoord.x, TexCoord.y); 
  
    vec2 adjustedTexCoord = mod(offsetTexCoord * textureTiling, 1) * textureSize + textureOffset;

    float movementX = -0.8 + sin(realtime * 0.1) * wiggleAmount.x;
    float movementY = -0.8 + sin(realtime * 0.1) * wiggleAmount.y;

    vec2 totalRipple = vec2(0.0, 0.0);
    for (int i = 0; i < 4; i++){
      vec2 rippleCoord = vec2(movementX + i * 0.4, movementY  + i * 0.4);
      vec2 rippleAmount = calcRipple(rippleCoord, 0.02);
      totalRipple += rippleAmount;
    }
    if (enableMouseRipple){
      //totalRipple += calcRipple(vec2(mouseCoordVal.x, mouseCoordVal.y), 0.0005);
    }

    adjustedTexCoord += totalRipple;


    vec4 diffuseColor = hasDiffuseTexture ? texture(maintexture, adjustedTexCoord) : vec4(1, 1, 1, 1);
    float closestDepth = texture(lightDepthTexture, shadowCoord.xy).r;

    vec4 emissionColor = texture(emissionTexture, adjustedTexCoord);
    vec4 opacityColor = texture(opacityTexture, adjustedTexCoord);

    bool discardTexture = hasOpacityTexture && opacityColor.r < discardTexAmount;   
    vec4 texColor;
    if (discardTexture){
        discard;
    }else{
        texColor = diffuseColor + emissionAmount * (hasEmissionTexture ? vec4(emissionColor.rgb, 0) : vec4(0, 0, 0, 0));
    }
    if (texColor.a < 0.1){
      discard;
    }

    vec3 normal = normalize(Normal);


    if (hasNormalTexture){
      vec3 normalTexColor = texture(normalTexture, adjustedTexCoord).rgb ;
      normal = TangentToWorld * normalize(vec3(normalTexColor.r * 2 - 1, normalTexColor.g * 2 - 1, normalTexColor.b * 2 - 1));
    }

    /*if (hasNormalTexture){
      vec3 normalTexColor = texture(normalTexture, adjustedTexCoord).rgb ;
      normal = normalize(vec3(normalTexColor.r * 2 - 1, normalTexColor.g * 2 - 1, normalTexColor.b * 2 - 1));
    }*/
//    texColor = texture(normalTexture, adjustedTexCoord).rgba;
    
 
    vec4 color  = vec4(calculatePhongLight(normal), 1.0) * texColor;
    

    bool inShadow = (shadowCoord.z - 0.00001) > closestDepth;
    float shadowDelta = (enableShadows && inShadow) ? shadowIntensity : 1.0;

    float noiseValue = noise(TexCoord + realtime * 0.00001) * noiseIntensity;
    if (enableLighting){
      FragColor = tint *  vec4(color.xyz * shadowDelta, color.w) * 2 + vec4(noiseValue, noiseValue, noiseValue, 0);
    }else{
      FragColor = tint * texColor * vec4(0.2, 0.2, 0.2, 1) + vec4(noiseValue, noiseValue, noiseValue, 0);
    }

    // TODO -> what would be a better thesholding function? 
    float brightness = FragColor.r + FragColor.g + FragColor.b;
    if(brightness > bloomThreshold){
      BloomColor = vec4(FragColor.rgb, 1.0);
    }else{
      BloomColor = vec4(0.0, 0.0, 0.0, 0.0);    
    }       
}
