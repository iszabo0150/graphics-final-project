#version 330 core

in vec3 textureDir;
//in vec2 texture_coords;

// uniform sampler2D myTexture;
// uniform float near;
// uniform float far;

out vec4 fragColor;

uniform samplerCube cubeMap;


// float linearizeDepth(float depth) {
//     float z = depth;
//     return (2.0 * near * far) / (far + near - z * (far - near));
// }

void main() {
    // float depthValue = texture(myTexture, texture_coords).r;
    // float newDepth = linearizeDepth(depthValue) / far;
    // newDepth = (newDepth) / (far - near) + near;

    // below linearization did not work
    //fragColor = vec4(newDepth, newDepth, newDepth, 1.0f);
   // fragColor = vec4(depthValue, depthValue, depthValue, 1.0f);
    fragColor = texture(cubeMap, textureDir);
   // fragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
