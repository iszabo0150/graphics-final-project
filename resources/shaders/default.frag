// default.frag
#version 330 core

// Interpolated from vertex shader (world space + object space)
in vec3 worldPos;
in vec3 worldNormal;
in vec3 objPos;

// Output color
out vec4 fragColor;

// Global Phong coefficients (from SceneGlobalData)
uniform float k_a;
uniform float k_d;
uniform float k_s;

// Camera
uniform vec4  u_CamPos;
uniform float shininess;

// Per object material colors (from SceneMaterial)
uniform vec3 u_MatAmbient;
uniform vec3 u_MatDiffuse;
uniform vec3 u_MatSpecular;

// Texture mapping
uniform sampler2D u_Texture;
uniform int   u_UseTexture;   // 0 = no texture, 1 = use texture
uniform vec2  u_TexRepeat;    // repeatU, repeatV
uniform float u_TexBlend;     // material blend factor [0,1]

// Shape type (must match PrimitiveType enum order)
uniform int u_ShapeType;

// Light type codes
#define LIGHT_TYPE_POINT        0
#define LIGHT_TYPE_DIRECTIONAL  1
#define LIGHT_TYPE_SPOT         2

// PrimitiveType codes (must match scenedata.h)
#define SHAPE_CUBE      0
#define SHAPE_CONE      1
#define SHAPE_CYLINDER  2
#define SHAPE_SPHERE    3
#define SHAPE_MESH      4

// Maximum number of lights
#define MAX_LIGHTS 8

// Multi light uniforms
uniform int   u_NumLights;
uniform vec3  u_LightPos[MAX_LIGHTS];
uniform vec3  u_LightDir[MAX_LIGHTS];
uniform vec3  u_LightColor[MAX_LIGHTS];
uniform vec3  u_LightFunc[MAX_LIGHTS];     // (c, l, q)
uniform int   u_LightType[MAX_LIGHTS];     // 0 point, 1 directional, 2 spot
uniform float u_LightAngle[MAX_LIGHTS];    // outer angle (radians)
uniform float u_LightPenumbra[MAX_LIGHTS]; // penumbra (radians)

float distanceAttenuation(vec3 func, float r) {
    float c = func.x;
    float l = func.y;
    float q = func.z;
    float denom = c + l * r + q * r * r;
    if (denom <= 0.0) {
        return 1.0;
    }
    return 1.0 / denom;
}

// Matches the smooth spotFalloff style you used in the raytracer
float spotAttenuation(int type, vec3 lightDir, vec3 LdirPW,
                      float angle, float penumbra) {
    if (type != LIGHT_TYPE_SPOT) {
        return 1.0;
    }

    vec3 Sd = normalize(lightDir);
    vec3 lightToPointDir = -normalize(LdirPW);

    float cosTheta = dot(Sd, lightToPointDir);

    float outer = max(0.0, angle);
    float inner = max(0.0, outer - max(0.0, penumbra));

    float cosOuter = cos(outer);
    float cosInner = cos(inner);

    float t = (cosTheta - cosOuter) / (cosInner - cosOuter + 1e-6);
    t = clamp(t, 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

// === UV mapping helpers ===
const float PI = 3.14159265;

float wrap01(float u) {
    u = u + 0.5;
    u = u - floor(u);
    return u;
}

// Sphere
vec2 uvSphereRT(vec3 pO) {
    vec3 n = normalize(pO);
    float u = atan(-n.z, n.x) / (2.0 * PI) + 0.5;
    u = wrap01(u);
    float v = 1.0 - acos(clamp(n.y, -1.0, 1.0)) / PI;
    return vec2(u, clamp(v, 0.0, 1.0));
}

// Cylinder side
vec2 uvCylinderSideRT(vec3 pO) {
    float u = atan(-pO.z, pO.x) / (2.0 * PI) + 0.5;
    u = wrap01(u);
    float v = clamp(pO.y + 0.5, 0.0, 1.0);
    return vec2(u, v);
}

// Cylinder cap
vec2 uvCylinderCapRT(vec3 pO, vec3 nO) {
    float u = clamp(pO.x + 0.5, 0.0, 1.0);
    float v = clamp((nO.y > 0.0 ? -pO.z : pO.z) + 0.5, 0.0, 1.0);
    return vec2(u, v);
}

// Cone side
vec2 uvConeSideRT(vec3 pO) {
    float u = atan(-pO.z, pO.x) / (2.0 * PI) + 0.5;
    u = wrap01(u);
    float v = clamp(pO.y + 0.5, 0.0, 1.0);
    return vec2(u, v);
}

// Cone cap
vec2 uvConeCapRT(vec3 pO) {
    float u = clamp(pO.x + 0.5, 0.0, 1.0);
    float v = clamp(pO.z + 0.5, 0.0, 1.0);
    return vec2(u, v);
}

// Cube (face-based, using normal)
vec2 uvCubeRT(vec3 pO, vec3 nO) {
    vec3 a = abs(nO);
    vec2 uv;
    if (a.x >= a.y && a.x >= a.z) {
        if (nO.x > 0.0) uv = vec2(-pO.z + 0.5,  pO.y + 0.5);
        else            uv = vec2( pO.z + 0.5,  pO.y + 0.5);
    } else if (a.y >= a.x && a.y >= a.z) {
        if (nO.y > 0.0) uv = vec2( pO.x + 0.5, -pO.z + 0.5);
        else            uv = vec2( pO.x + 0.5,  pO.z + 0.5);
    } else {
        if (nO.z > 0.0) uv = vec2( pO.x + 0.5,  pO.y + 0.5);
        else            uv = vec2(-pO.x + 0.5,  pO.y + 0.5);
    }
    return clamp(uv, vec2(0.0), vec2(1.0));
}

// Planar mapping by dominant normal (for meshes / fallback)
vec2 uvPlanarByNormalRT(vec3 pO, vec3 nO) {
    vec3 a = abs(nO);
    if (a.x >= a.y && a.x >= a.z) {
        return clamp(vec2(pO.z + 0.5, pO.y + 0.5), vec2(0.0), vec2(1.0));
    }
    if (a.y >= a.x && a.y >= a.z) {
        return clamp(vec2(pO.x + 0.5, pO.z + 0.5), vec2(0.0), vec2(1.0));
    }
    return clamp(vec2(pO.x + 0.5, pO.y + 0.5), vec2(0.0), vec2(1.0));
}

// Decide which mapping to use based on shape + normal
vec2 computeUV(vec3 pObj, vec3 nWorld) {
    vec3 nO = normalize(nWorld);

    if (u_ShapeType == SHAPE_SPHERE) {
        return uvSphereRT(pObj);

    } else if (u_ShapeType == SHAPE_CYLINDER) {
        // Use normal to distinguish side vs cap
        vec3 a = abs(nO);
        if (a.y >= a.x && a.y >= a.z) {
            return uvCylinderCapRT(pObj, nO);
        } else {
            return uvCylinderSideRT(pObj);
        }

    } else if (u_ShapeType == SHAPE_CONE) {
        vec3 a = abs(nO);
        if (a.y >= a.x && a.y >= a.z) {
            // bottom cap
            return uvConeCapRT(pObj);
        } else {
            // slanted side
            return uvConeSideRT(pObj);
        }

    } else if (u_ShapeType == SHAPE_CUBE) {
        return uvCubeRT(pObj, nO);

    } else { // SHAPE_MESH: planar by normal
        return uvPlanarByNormalRT(pObj, nO);
    }
}

void main()
{
    // Base normal and view direction
    vec3 N = normalize(worldNormal);
    vec3 V = normalize(u_CamPos.xyz - worldPos);

    // Flip normal to face the viewer if needed (same as shadePhongAll)
    if (dot(N, V) < 0.0) {
        N = -N;
    }

    float shin = max(1.0, shininess);

    // Match raytracer variable naming
    vec3 ka = u_MatAmbient;
    vec3 ks = u_MatSpecular;

    // === Diffuse base and texture mixing (mirror shadePhongAll) ===
    vec3 diffuseBase  = k_d * u_MatDiffuse;   // g.kd * mat.cDiffuse
    vec3 diffuseColor = diffuseBase;

    if (u_UseTexture == 1) {
        vec2 uvBase = computeUV(objPos, worldNormal);
        vec2 uv     = uvBase * u_TexRepeat;

        // Flip vertically to match image origin
        uv.y = 1.0 - uv.y;

        vec3 texRGB = texture(u_Texture, uv).rgb;
        float alpha = clamp(u_TexBlend, 0.0, 1.0);
        diffuseColor = mix(diffuseBase, texRGB, alpha);
    }

    // Start with ambient term: C = g.ka * ka
    vec3 color = k_a * ka;

    // Sum all lights
    for (int i = 0; i < u_NumLights; ++i) {
        int  type     = u_LightType[i];
        vec3 LiColor  = u_LightColor[i];
        vec3 LdirPW   = vec3(0.0);
        float distAtten = 1.0;
        float coneAtten = 1.0;

        if (type == LIGHT_TYPE_DIRECTIONAL) {
            // Directional light: direction only, no distance attenuation
            LdirPW = normalize(-u_LightDir[i]);
        } else {
            // Point or spot
            vec3 toL = u_LightPos[i] - worldPos;
            float r = length(toL);
            if (r <= 0.0) {
                continue;
            }
            LdirPW = toL / r;

            distAtten = distanceAttenuation(u_LightFunc[i], r);
            coneAtten = spotAttenuation(type, u_LightDir[i], LdirPW,
                                        u_LightAngle[i], u_LightPenumbra[i]);
            if (coneAtten <= 0.0) {
                continue;
            }
        }

        float NdotL = max(dot(N, LdirPW), 0.0);
        if (NdotL <= 0.0) {
            continue;
        }

        // Diffuse: uses diffuseColor directly (already has k_d / texture mix baked in)
        vec3 diffuse = diffuseColor * NdotL;

        // Phong specular: g.ks * ks * pow(R · V, shin)
        vec3 R = normalize(reflect(-LdirPW, N));
        float RdotV = max(dot(R, V), 0.0);
        vec3 specular = k_s * ks * pow(RdotV, shin);

        float weight = distAtten * coneAtten;
        color += (diffuse + specular) * LiColor * weight;
    }

    color = clamp(color, 0.0, 1.0);
    fragColor = vec4(color, 1.0);
}
