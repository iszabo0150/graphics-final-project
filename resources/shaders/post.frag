#version 330 core

in vec2 vUV;
out vec4 fragColor;

uniform sampler2D u_ScreenTex;
uniform int u_Mode; // 0 passthrough, 1 invert, 2 grayscale

void main() {
    vec3 col = texture(u_ScreenTex, vUV).rgb;

    if (u_Mode == 1) {
        col = vec3(1.0) - col;
    } else if (u_Mode == 2) {
        float g = dot(col, vec3(0.299, 0.587, 0.114));
        col = vec3(g);
    }

    fragColor = vec4(col, 1.0);
}
