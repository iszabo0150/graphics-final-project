#version 330 core

// Task 4: declare a vec3 object-space position variable, using
//         the `layout` and `in` keywords.

layout(location = 0) in vec3 posObjSpace;
layout(location = 1) in vec3 normalObjSpace;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tangentObjSpace;
layout(location = 4) in vec3 bitangentObjSpace;

// model matrix
layout(location = 5) in vec4 model0;
layout(location = 6) in vec4 model1;
layout(location = 7) in vec4 model2;
layout(location = 8) in vec4 model3;

// material properties
layout(location = 9) in vec3 ambient;
layout(location = 10) in vec3 diffuse;
layout(location = 11) in vec3 specular;
layout(location = 12) in float shininess;

// Task 5: declare `out` variables for the world-space position and normal,
//         to be passed to the fragment shader

out vec3 posWorldSpace;
out vec3 normalWorldSpace; //in world space as well
out vec2 fragUV;

out vec3 tangentWorldSpace;
out vec3 bitangentWorldSpace;

out vec4 lightSpacePosition;

out vec3 materialAmbient;
out vec3 materialDiffuse;
out vec3 materialSpecular;
out float materialShininess;

// no londer needed because of instance rendering.
// uniform mat4 modelMatrix;
// uniform mat4 modelInverseTrans;

uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 lightMatrix;


void main() {
    
    // reconstructing matricies from instance attribs 
    mat4 modelMatrix = mat4(model0, model1, model2, model3);
    mat4 normalMatrix = transpose(inverse(modelMatrix));

    vec4 p = modelMatrix * vec4(posObjSpace, 1.0);
    posWorldSpace = p.xyz;

    // fixed ????
    normalWorldSpace = normalize(mat3(normalMatrix) * normalize(normalObjSpace));
    tangentWorldSpace = normalize(mat3(normalMatrix) * tangentObjSpace);
    bitangentWorldSpace = normalize(mat3(normalMatrix) * bitangentObjSpace);

    fragUV = uv;

    materialAmbient = ambient;
    materialDiffuse = diffuse;
    materialSpecular = specular;
    materialShininess = shininess;

    // set gl_Position to the object space position transformed to clip space
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(posObjSpace, 1.0);

    // compute light-space position using light matrix (for shadow mapping)
    lightSpacePosition = lightMatrix * p;

}
