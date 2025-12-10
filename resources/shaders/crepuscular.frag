#version 330 core

in vec2 uv;
out vec4 fragColor;

uniform sampler2D sceneTexture;
uniform sampler2D depthTexture;

uniform vec2 lightScreenPosition;

uniform float exposure;
uniform float decay;
uniform float density;
uniform float weight;
uniform int samples;

void main() {

    vec3 sceneColor = texture(sceneTexture, uv).rgb;

    vec2 deltaTexCoord = uv - lightScreenPosition;
    deltaTexCoord *= 1.0 / float(samples) * density;

    vec2 texCoord = uv;
    float illuminationDecay = 1.0;
    vec3 rays = vec3(0.0);

    for (int i = 0; i < samples; i++) {

        texCoord -= deltaTexCoord;

        float depth = texture(depthTexture, texCoord).r;
        float occlusion = (depth > 0.999) ? 1.0 : 0.0;

        vec3 f_sample = texture(sceneTexture, texCoord).rgb * occlusion;
        f_sample *= illuminationDecay * weight;
        rays += f_sample;

        illuminationDecay *= decay;

    }

    rays *= exposure;
    fragColor = vec4(sceneColor + rays, 1.0);

}
