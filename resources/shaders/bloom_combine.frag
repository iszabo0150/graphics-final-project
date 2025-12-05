#version 330 core
in vec2 vUV;
out vec4 fragColor;

uniform sampler2D u_SceneTex;
uniform sampler2D u_BloomTex;
uniform float u_Strength;   // like 0.6

void main() {
    vec3 scene = texture(u_SceneTex, vUV).rgb;
    vec3 bloom = texture(u_BloomTex, vUV).rgb;

    vec3 outCol = scene + bloom * u_Strength;
    outCol = clamp(outCol, 0.0, 1.0);

    fragColor = vec4(outCol, 1.0);
}
