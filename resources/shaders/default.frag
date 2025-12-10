#version 330 core

struct Light {
    int type; //0 is a pointlight, 1 is a directional, 2 is a spotlight
    vec3 color;
    vec3 function;// attenuation functoin
    vec3 pos; // position in world space
    vec3 dir; // Direction with CTM applied (Not applicable to point lights)
    float penumbra; // Only applicable to spot lights, in RADIANS
    float angle;    // Only applicable to spot lights, in RADIANS
};

struct Material {
    vec3 cAmbient;  // Ambient term
    vec3 cDiffuse;  // Diffuse term
    vec3 cSpecular; // Specular term
    float shininess;      // Specular exponent
};

struct TextureInfo {
    bool isUsed;
    float repeatU;
    float repeatV;
    float blend;
};

struct NormalMapInfo {
    bool isUsed;
    float repeatU;
    float repeatV;
    float strength;
};

uniform TextureInfo textureInfo;
uniform NormalMapInfo bumpMapInfo;
uniform NormalMapInfo normalMapInfo;

uniform sampler2D textureSampler;
uniform sampler2D bumpTextureSampler;
uniform sampler2D normTextureSampler;


in vec3 posWorldSpace;
in vec3 normalWorldSpace;
in vec4 lightSpacePosition;
in vec2 fragUV;

in vec3 tangentWorldSpace;
in vec3 bitangentWorldSpace;

in vec3 materialAmbient;
in vec3 materialDiffuse;
in vec3 materialSpecular;
in float materialShininess;

out vec4 fragColor;

uniform Light lights[8];
uniform int lightCount;
// uniform Material material; no longer needed, passed from instance rendering
uniform vec3 cameraPos;

uniform float kd;
uniform float ks;
uniform float ka;

uniform sampler2D shadowTexture;

float calculateShadow(vec4 lightPosition, vec3 lightDir) {
    // [-w, w] coords to [-1, 1] coords
    vec3 projectionCoords = lightPosition.xyz / lightPosition.w;
    // depth map range is [0, 1], projectionCoords are [-1, 1]
    projectionCoords = projectionCoords * 0.5 + 0.5;

    float xOffset = 1.0/1024.0f;
    float yOffset = 1.0/1024.0f;


    float dot = dot(normalize(normalWorldSpace), lightDir);
    dot = clamp(dot, 0.0f, 1.0f);
    float bias = max(0.05 * (1.0 - dot), 0.005);

    float sum = 0.0f;
    // samples 25 texels
    for (int y = -2; y <= 2; ++y) {
        for (int x = -2; x <=2; ++x) {
            vec2 offsets = vec2(x * xOffset, y * yOffset);
            // add 0.001 to avoid z-fighting
            vec2 coords = projectionCoords.xy + offsets;


            float closestDepth = texture(shadowTexture, coords).r;
            float currDepth = projectionCoords.z - bias;

            if (currDepth > closestDepth) {
                sum += 1.0f;
            }
        }
    }

    return sum / 25.0f;
}


vec3 getNormal() {
    if (bumpMapInfo.isUsed || normalMapInfo.isUsed) {
        // build TBN matrix (world space to tangent space)
        mat3 TBN = transpose(mat3(
            normalize(tangentWorldSpace),
            normalize(bitangentWorldSpace),
            normalize(normalWorldSpace)
        ));

        vec3 combinedNormal = vec3(0.0, 0.0, 1.0);

        if (bumpMapInfo.isUsed) {
            vec2 bumpUV = vec2(fragUV.x * bumpMapInfo.repeatU, fragUV.y * bumpMapInfo.repeatV);
            vec3 bumpNormal = texture(bumpTextureSampler, bumpUV).rgb * 2.0 - 1.0;
            combinedNormal = normalize(mix(combinedNormal, bumpNormal, bumpMapInfo.strength));
        }

        if (normalMapInfo.isUsed) {
            vec2 normUV = vec2(fragUV.x * normalMapInfo.repeatU, fragUV.y * normalMapInfo.repeatV);
            vec3 normalMapNormal = texture(normTextureSampler, normUV).rgb * 2.0 - 1.0;
            combinedNormal = normalize(mix(combinedNormal, normalMapNormal, normalMapInfo.strength));
        }

        // transform from tangent space to world space
        vec3 worldSpaceNormal = transpose(TBN) * combinedNormal;

        return normalize(worldSpaceNormal);
    } else {
        // just use the normalized original normal
        return normalize(normalWorldSpace);
    }
}



// calculates the phong model for directional lights!
vec3 phongDirectional(vec3 matDiff, vec3 lightColor, float NdotL, vec3 lightDirNormalized, vec3 normNormalized, vec3 camDirNormalized, vec3 lightDir){
    // calculate shadow
    float shadow = calculateShadow(lightSpacePosition, normalize(lightDir));

    // diffusion !!
    vec3 diffuse = (1.0f - shadow) * lightColor * matDiff * NdotL;

    // specular !!
    vec3 reflectDir = normalize(reflect(-lightDirNormalized, normNormalized));
    float RdotV = max(0.0f, dot(reflectDir, camDirNormalized));

    vec3 specular = vec3(0, 0, 0);

    // if (RdotV > 0.0f){
    if (materialShininess != 0){
        float specFactor = pow(RdotV, materialShininess);
        specular = (1.0f - shadow) * ks * lightColor * vec3(materialSpecular) * specFactor;
    } else {
        specular = (1.0f - shadow) * ks * lightColor * vec3(materialSpecular);
    }

    return diffuse + specular;
}

// calculates the phong model for point lights!
vec3 phongPoint(vec3 matDiff, vec3 lightColor, float NdotL, vec3 lightDirNormalized, vec3 lightPos, vec3 normNormalized, vec3 camDirNormalized, vec3 att_coeffs){

    float distanceFromLight = distance(lightPos, posWorldSpace);

    float attenuation = min(1.0f, 1.0f / (att_coeffs[0] + (distanceFromLight * att_coeffs[1]) +
                                               (distanceFromLight * distanceFromLight) * att_coeffs[2]));

    // diffusion !!
    vec3 diffuse = attenuation * lightColor * matDiff * NdotL;

    // specular !!
    vec3 reflectDir = reflect(-lightDirNormalized, normNormalized);
    float RdotV = max(0.0f, dot(reflectDir, camDirNormalized));

    vec3 specular = vec3(0, 0, 0);

    // if (RdotV > 0.0f){
    //     float specFactor = pow(RdotV, materialShininess);
    //     specular = attenuation * ks * lightColor * vec3(materialSpecular) * specFactor;
    // }


    if (materialShininess != 0){
        float specFactor = pow(RdotV, materialShininess);
        specular = attenuation * ks * lightColor * vec3(materialSpecular) * specFactor;
    } else {
        specular = attenuation * ks * lightColor * vec3(materialSpecular);
    }

    return diffuse + specular;
}

// helper method for calculting phong model for spot lights-- used fo rhte diffuse term
vec3 calcLightIntensity(float x, float outer_angle, float penumbra, vec3 lightColor) {

    float theta_inner = outer_angle - penumbra;

    if (x <= theta_inner) {
        return lightColor; // full intensity

    } else if (x > outer_angle) {
        return vec3(0.0f); // completely outside of cone

    } else { //within the outer cone
        float t = (x - theta_inner) / (outer_angle - theta_inner);
        float falloff = -2.0f * pow(t, 3.0f) + 3.0f * pow(t, 2.0f);
        return lightColor * (1.0f - falloff);
    }
}

//calculates the phong model for spot lights !
vec3 phongSpot(vec3 matDiff, vec3 lightColor, float NdotL, vec3 lightDirNormalized, vec3 lightPos, vec3 normNormalized, vec3 camDirNormalized, vec3 att_coeffs,
               float outer_angle, float penumbra) {

    float distanceFromLight = distance(lightPos, posWorldSpace);
    vec3 dirFomLightToObject = normalize(posWorldSpace - lightPos);
    vec3 dirToLight = normalize(lightPos - posWorldSpace);

    float shadow = calculateShadow(lightSpacePosition, lightDirNormalized);

    //the angle between the current direction from the the hit point to the light and the direction of the spotlight itself
    float x = acos(dot(lightDirNormalized, dirFomLightToObject));
    //calculating the intensity of the light depending on where in the cone we are !!

    vec3 lightIntensity = calcLightIntensity(x, outer_angle, penumbra, lightColor);

    float attenuation = min(1.0f, 1.0f / (att_coeffs[0] + (distanceFromLight * att_coeffs[1]) +
                                               (distanceFromLight * distanceFromLight) * att_coeffs[2]));

    // diffusion !!
    vec3 diffuse = (1.0f - shadow) * lightIntensity * attenuation * matDiff * NdotL;

    // specular !!
    vec3 reflectDir = reflect(-dirToLight, normNormalized);
    float RdotV = max(0.0f, dot(reflectDir, camDirNormalized));


    vec3 specular = vec3(0, 0, 0);

    // if (RdotV > 0.0f){

    //     float specFactor = pow(RdotV, materialShininess);
    //     specular = attenuation * lightIntensity * ks * vec3(materialSpecular) * specFactor;
    // }


    if (materialShininess != 0){
        float specFactor = pow(RdotV, materialShininess);
        specular = (1.0f - shadow) * attenuation * ks * lightColor * vec3(materialSpecular) * specFactor;
    } else {
        specular = (1.0f - shadow) * attenuation * ks * lightColor * vec3(materialSpecular);
    }
    return diffuse + specular;
}





void main() {

    vec3 color = vec3(0.0); //all black

    color += ka * materialAmbient;

    vec3 matDiff = kd * vec3(materialDiffuse);

    if (textureInfo.isUsed) {
        vec2 repeatedUV = vec2(fragUV.x * textureInfo.repeatU, fragUV.y * textureInfo.repeatV);
        vec3 texColor = texture(textureSampler, repeatedUV).rgb;
        matDiff = mix(matDiff, texColor, textureInfo.blend);
    }

    vec3 surfToCam = normalize(cameraPos - posWorldSpace);

    vec3 normNormalized  = getNormal();
    float NdotL;
    vec3 surfToLight;

    for (int i=0; i< lightCount; i++){
        vec3 lightDirNormalized = normalize(-lights[i].dir);

        switch(lights[i].type){
            case 0: // point light
                surfToLight = normalize(lights[i].pos - posWorldSpace);
                NdotL = clamp(dot(normNormalized, surfToLight), 0, 1);

                color += phongPoint(matDiff, lights[i].color, NdotL, surfToLight, lights[i].pos,  normNormalized, surfToCam, lights[i].function);
                break;

            case 1: // direction light
                NdotL = clamp(dot(normNormalized, lightDirNormalized), 0, 1);
                color += phongDirectional(matDiff, lights[i].color, NdotL, lightDirNormalized, normNormalized, surfToCam, lights[i].dir);
                break;

            case 2: // spotlight
                surfToLight = normalize(lights[i].pos - posWorldSpace);
                NdotL = max(0.0f, dot(normNormalized, surfToLight));
                color += phongSpot(matDiff, lights[i].color, NdotL, normalize(lights[i].dir), lights[i].pos, normNormalized, surfToCam, lights[i].function,
                                   lights[i].angle, lights[i].penumbra);
                break;
            default:
                break;
        }
    }

    fragColor = clamp(vec4(color, 1), 0, 1);

}
