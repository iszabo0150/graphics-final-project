#version 330 core
in vec2 vUV;
out vec4 fragColor;

uniform sampler2D u_SceneTex;
uniform float u_Threshold;   // like 0.8
uniform float u_SoftKnee;    // like 0.2

float luminance(vec3 c) {
    return dot(c, vec3(0.2126, 0.7152, 0.0722));
}

void main() {
    vec3 col = texture(u_SceneTex, vUV).rgb;
    float l = luminance(col);

    // Soft threshold (prevents harsh cutoff)
    float knee = max(u_SoftKnee, 1e-6);
    float t0 = u_Threshold - knee;
    float t1 = u_Threshold + knee;
    float w = clamp((l - t0) / (t1 - t0), 0.0, 1.0);
    w = w * w * (3.0 - 2.0 * w);

    fragColor = vec4(col * w, 1.0);
}
