#version 430 

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in mat3 TangentToWorld;
in vec4 sshadowCoord;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BloomColor;

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
uniform bool enablePBR;
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
uniform vec3 lightsdir[MAX_LIGHTS];
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
uniform float time;
uniform float bloomThreshold;
//uniform vec3 mouseCoordVal;
uniform bool enableAttenutation;
uniform bool enableVoxelLighting;


const float PI = 3.14159265359;


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
  float attenuation = enableVoxelLighting && enableAttenutation ? (1.0 / (constant + (linear * distanceToLight) + (quadratic * (distanceToLight * distanceToLight)))) : 1;
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

  return vec3(1, 1, 1);
}

vec3 calculatePhongLight(vec3 normal){
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

// approximation for fraction of light that gets reflected (not refracted)
vec3 fresnelSchlick(float angle, vec3 F0){
    return F0 + (1.0 - F0) * pow(clamp(1.0 - angle, 0.0, 1.0), 5);
} 


// Approximation for the fraction the microfacets aligned to halfway vector 
float trowbridgereitzGGX(vec3 N, vec3 H, float roughness){
    float a = roughness * roughness;
    float asquared     = a * a;
    float nOnH  = max(dot(N, H), 0.0);
    return asquared / (PI * pow(((nOnH * nOnH) * (asquared - 1.0) + 1.0), 2));   //  "Practically in applications 1.5<γ<3"
}

//  Approximation for fraction of not occluded geometry (do to one microfacet overlapping the ray of another)
float schlickGGX(float nOnV, float roughness){
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return nOnV / (nOnV * (1.0 - k) + k);
}
float smith(vec3 N, vec3 V, vec3 L, float roughness){
    float nOnV = max(dot(N, V), 0.0);
    float nOnL = max(dot(N, L), 0.0);
    float ggx2  = schlickGGX(nOnV, roughness);
    float ggx1  = schlickGGX(nOnL, roughness);
    return ggx1 * ggx2;
}

float ao = 3.2;
vec4 calcCookTorrence(
  vec3 N,
  vec3 albedo, 
  float metallic, 
  float roughness // 1 is rough, disperes light, 0 is metallic, darker with highlights. perfectly smooth is just dark so pick something small
){ 
    vec3 V = normalize(cameraPosition - FragPos);  
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 Lo = vec3(0.0);
    for(int i = 0; i < getNumLights(); ++i){
        vec3 L = normalize(lights[i] - FragPos);
        vec3 H = normalize(V + L);
        float attenuation = calcAttenutation(i);
        vec3 radiance     = lightscolor[i] * attenuation;        
        
        float D = trowbridgereitzGGX(N, H, roughness);        
        float G = smith(N, V, L, roughness);      
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
        vec3 kS = F;
        vec3 kD = (1.0 - metallic) * (vec3(1.0) - kS);
        
        vec3 specular = (D * G * F) / (4 * max(dot(N, V), 0) * max(dot(N, L), 0) + 0.0001);  
            
        float nOnL = max(dot(N, L), 0.0);                
        Lo += ((kD * albedo / PI) + specular) * radiance * nOnL; 
    }

    vec3 ambient = vec3(0.53) * albedo * ao;
    vec4 color = vec4(ambient + Lo, 1);
    return color;
}
////////////////////

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


vec2 kaleido(vec2 uv){
	float th = atan(uv.y, uv.x);
	float r = pow(length(uv), 1.);

  float p1 = sin(2. * PI * (time * 0.1) / 10.);
  float q = 2. * PI / ( 5. + 4. * p1);
  th = abs(mod(th, q) - 0.5 * q);
	return vec2(cos(th), sin(th)) * pow(r, 1.3 + 1.3 / (1.3 + sin(2. * PI * (time * 0.1) / 3.))) * .1;
}


vec2 transform(vec2 at){
	vec2 v;
	float th = .1 * (time * 0.1);
	v.x = at.x * cos(th) - at.y * sin(th) - .3 * sin(th);
	v.y = at.x * sin(th) + at.y * cos(th) + .3 * cos(th);

 	return mod(at + 0.1, 1.) * 2.0;
}


void main(){
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

    float movementX = -0.8 + sin(time * 0.1) * wiggleAmount.x;
    float movementY = -0.8 + sin(time * 0.1) * wiggleAmount.y;

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


    vec4 diffuseColor = hasDiffuseTexture ? texture(maintexture, transform(kaleido(adjustedTexCoord)) * 100) : vec4(1, 1, 1, 1);
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
    
 
    vec4 color  = enablePBR ? calcCookTorrence(normal, texColor.rgb, 0.2, 0.5) : vec4(calculatePhongLight(normal), 1.0) * texColor;
    

    bool inShadow = (shadowCoord.z - 0.00001) > closestDepth;
    float shadowDelta = (enableShadows && inShadow) ? shadowIntensity : 1.0;


    if (enableLighting){
      FragColor = tint *  vec4(color.xyz * shadowDelta, color.w);
    }else{
      FragColor = tint * texColor;
    }

    // TODO -> what would be a better thesholding function? 
    float brightness = FragColor.r + FragColor.g + FragColor.b;
    if(brightness > bloomThreshold){
      BloomColor = vec4(FragColor.rgb, 1.0);
    }else{
      BloomColor = vec4(0.0, 0.0, 0.0, 0.0);    
    }       
}
