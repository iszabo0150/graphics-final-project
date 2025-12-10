#version 330 core

layout (location = 0) in vec3 position;
//layout (location = 1) in vec2 uv_coords;
uniform mat4 projection;
uniform mat4 view;

//out vec2 texture_coords;
out vec3 textureDir;


void main() {
    //gl_Position = vec4(position, 1.0f);
    //texture_coords = uv_coords;
    textureDir = vec3(-position.x, -position.y, position.z);
    vec4 pos = projection * view * vec4(position, 1.0);
    gl_Position = pos.xyww;

}
