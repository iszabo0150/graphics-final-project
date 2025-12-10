#version 330 core
in vec2 vUV;
out vec4 fragColor;

uniform sampler2D u_InputTex;
uniform vec2 u_TexelStep; // (1/width,0) for horizontal, (0,1/height) for vertical

void main() {
    // 9-tap Gaussian weights (common set)
    float w0 = 0.227027;
    float w1 = 0.1945946;
    float w2 = 0.1216216;
    float w3 = 0.054054;
    float w4 = 0.016216;

    vec3 sum = texture(u_InputTex, vUV).rgb * w0;
    sum += texture(u_InputTex, vUV + u_TexelStep * 1.0).rgb * w1;
    sum += texture(u_InputTex, vUV - u_TexelStep * 1.0).rgb * w1;
    sum += texture(u_InputTex, vUV + u_TexelStep * 2.0).rgb * w2;
    sum += texture(u_InputTex, vUV - u_TexelStep * 2.0).rgb * w2;
    sum += texture(u_InputTex, vUV + u_TexelStep * 3.0).rgb * w3;
    sum += texture(u_InputTex, vUV - u_TexelStep * 3.0).rgb * w3;
    sum += texture(u_InputTex, vUV + u_TexelStep * 4.0).rgb * w4;
    sum += texture(u_InputTex, vUV - u_TexelStep * 4.0).rgb * w4;

    fragColor = vec4(sum, 1.0);
}
