//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//        __  __        _        __  ___ ____   __  ___
//       / / / /____ _ (_)_____ /  |/  // __ \ /  |/  /
//      / /_/ // __ `// // ___// /|_/ // /_/ // /|_/ /
//     / __  // /_/ // // /   / /  / // ____// /  / /
//    /_/ /_/ \__,_//_//_/   /_/  /_//_/    /_/  /_/
//
//    This file is part of HairMPM - Material Point Method for Hair Simulation.
//    Created: 2018. All rights reserved.
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// fragment shader, particle render
#version 410 core

#define COLOR_MODE_UNIFORM_DIFFUSE_COLOR 0
#define COLOR_MODE_RANDOM                1
#define COLOR_MODE_RAMP                  2
#define COLOR_MODE_OBJ_INDEX             3

#define NUM_TOTAL_LIGHTS 4

struct PointLight
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 position;
};

layout(std140) uniform Lights
{
    PointLight lights[NUM_TOTAL_LIGHTS];
    int        u_NumLights;
};

layout(std140) uniform CameraData
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 invViewMatrix;
    mat4 invProjectionMatrix;
    mat4 shadowMatrix;
    vec4 camPosition;
    vec4 camFocus;
};

layout(std140) uniform Material
{
    vec4  ambient;
    vec4  diffuse;
    vec4  specular;
    float shininess;
} material;

uniform float u_PointRadius;
uniform int   u_ColorMode;
uniform int   u_ParticleType;
uniform vec4  u_ParticleTypeColors[3]; // standard, vertex, quad

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
flat in vec3 f_ViewCenter;
flat in vec3 f_Color;

out vec4 outColor;

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
vec3 shadeLight(int lightID, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir     = normalize(vec3(viewMatrix * lights[lightID].position) - fragPos);
    vec3 halfDir      = normalize(lightDir - viewDir);
    vec4 surfaceColor;
    if(u_ColorMode == COLOR_MODE_UNIFORM_DIFFUSE_COLOR)
        surfaceColor = material.diffuse;
    else
        surfaceColor = vec4(f_Color, 1.0);

    vec4 ambientColor  = lights[lightID].ambient * material.ambient;
    vec4 diffuseColor  = lights[lightID].diffuse * vec4(max(dot(normal, lightDir), 0.0)) * surfaceColor;
    vec4 specularColor = lights[lightID].specular * pow(max(dot(halfDir, normal), 0.0), material.shininess) * material.specular;

    return vec3(ambientColor + diffuseColor + specularColor);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void main()
{
    vec3 viewDir = normalize(f_ViewCenter);
    vec3 normal;
    vec3 fragPos;

    normal.xy = gl_PointCoord.xy * vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    float mag = dot(normal.xy, normal.xy);

    if(mag > 1.0) {
        discard;               // kill pixels outside circle
    }
    normal.z = sqrt(1.0 - mag);
    fragPos  = f_ViewCenter + normal * u_PointRadius;

    // correct depth
    float z = dot(vec4(fragPos, 1.0), transpose(projectionMatrix)[2]);
    float w = dot(vec4(fragPos, 1.0), transpose(projectionMatrix)[3]);
    gl_FragDepth = 0.5 * (z / w + 1.0);

    vec3 shadeColor = vec3(0, 0, 0);
    for(int i = 0; i < u_NumLights; ++i) {
        shadeColor += shadeLight(i, normal, fragPos, viewDir);
    }

    outColor = vec4(shadeColor, 1.0);
}
