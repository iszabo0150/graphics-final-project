#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec4 aColor;
layout(location=2) in float aSize;

uniform mat4 u_View;
uniform mat4 u_Proj;

out vec4 vColor;

void main() {
    vColor = aColor;
    gl_Position = u_Proj * u_View * vec4(aPos, 1.0);
    gl_PointSize = max(aSize, 1.0);
}
