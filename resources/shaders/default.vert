#version 330 core

// Task 4: declare a vec3 object-space position variable, using
//         the `layout` and `in` keywords.

layout(location = 0) in vec3 posObjSpace;
layout(location = 1) in vec3 normalObjSpace;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tangentObjSpace;
layout(location = 4) in vec3 bitangentObjSpace;



// Task 5: declare `out` variables for the world-space position and normal,
//         to be passed to the fragment shader

out vec3 posWorldSpace;
out vec3 normalWorldSpace; //in world space as well
out vec2 fragUV;

out vec3 tangentWorldSpace;
out vec3 bitangentWorldSpace;

// // Task 6: declare a uniform mat4 to store model matrix

uniform mat4 modelMatrix;
uniform mat4 modelInverseTrans;

// // Task 7: declare uniform mat4's for the view and projection matrix

uniform mat4 viewMatrix;
uniform mat4 projMatrix;


void main() {
    // Task 8: compute the world-space position and normal, then pass them to
    //         the fragment shader using the variables created in task 5

    vec4 p = modelMatrix * vec4(posObjSpace, 1.0);
    posWorldSpace = p.xyz;


    //world space normal
    vec3 objNormal = normalize(normalObjSpace);

    fragUV = uv;


    //FIX !!!!!! was doing matrices weird :P
    normalWorldSpace = normalize(mat3(modelInverseTrans) * objNormal);



    // Recall that transforming normals requires obtaining the inverse-transpose of the model matrix!
    // In projects 5 and 6, consider the performance implications of performing this here.

    // Task 9: set gl_Position to the object space position transformed to clip space

    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(posObjSpace, 1.0);

    tangentWorldSpace = normalize(mat3(modelInverseTrans) * tangentObjSpace);
    bitangentWorldSpace = normalize(mat3(modelInverseTrans) * bitangentObjSpace);
}
