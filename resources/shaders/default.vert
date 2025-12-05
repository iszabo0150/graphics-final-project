// default.vert
#version 330 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;

out vec3 worldPos;
out vec3 worldNormal;
out vec3 objPos; // object-space position for UV mapping

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;

void main()
{
    objPos = inPos;

    // World-space position
    vec4 world = u_Model * vec4(inPos, 1.0);
    worldPos = world.xyz;

    // World-space normal
    mat3 normalMat = mat3(transpose(inverse(u_Model)));
    worldNormal = normalize(normalMat * inNormal);

    gl_Position = u_Proj * u_View * world;
}
