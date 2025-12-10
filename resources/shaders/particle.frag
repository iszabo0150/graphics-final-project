#version 330 core

in vec4 vColor;
out vec4 fragColor;

// How soft the circle edge is (0.0 = hard, ~0.05-0.15 = nice)
uniform float u_Softness;

void main() {
    // gl_PointCoord is 0..1 inside the point
    vec2 d = gl_PointCoord - vec2(0.5);
    float r = length(d);

    // Circle with soft edge
    float edge0 = 0.5 - u_Softness;
    float edge1 = 0.5;
    float mask = 1.0 - smoothstep(edge0, edge1, r);

    if (mask <= 0.0) discard;

    fragColor = vec4(vColor.rgb, vColor.a * mask);
}
