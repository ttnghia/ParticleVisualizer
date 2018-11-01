//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// ______          _   _      _        _   _ _                 _ _
// | ___ \        | | (_)    | |      | | | (_)               | (_)
// | |_/ /_ _ _ __| |_ _  ___| | ___  | | | |_ ___ _   _  __ _| |_ _______ _ __
// |  __/ _` | '__| __| |/ __| |/ _ \ | | | | / __| | | |/ _` | | |_  / _ \ '__|
// | | | (_| | |  | |_| | (__| |  __/ \ \_/ / \__ \ |_| | (_| | | |/ /  __/ |
// \_|  \__,_|_|   \__|_|\___|_|\___|  \___/|_|___/\__,_|\__,_|_|_/___\___|_|
//
// Created: Nov. 2018 by NT (https://ttnghia.github.io). All rights reserved.
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// vertex shader, particle render
#version 410 core

#define COLOR_MODE_UNIFORM_DIFFUSE_COLOR 0
#define COLOR_MODE_RANDOM                1
#define COLOR_MODE_RAMP                  2
#define COLOR_MODE_OBJ_INDEX             3

uniform vec3 colorRamp[] = vec3[] (vec3(1.0, 0.0, 0.0),
                                   vec3(1.0, 0.5, 0.0),
                                   vec3(1.0, 1.0, 0.0),
                                   vec3(1.0, 0.0, 1.0),
                                   vec3(0.0, 1.0, 0.0),
                                   vec3(0.0, 1.0, 1.0),
                                   vec3(0.0, 0.0, 1.0));

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

uniform int  u_Dimension;
uniform uint u_nParticles;
uniform int  u_ColorMode;
uniform int  u_ParticleType;

uniform float u_vColorMin;
uniform float u_vColorMax;
uniform vec3  u_ColorMinVal;
uniform vec3  u_ColorMaxVal;

uniform vec4  u_ClipPlane;
uniform float u_PointRadius;
uniform float u_PointScale;
uniform int   u_ScreenHeight;
uniform float u_DomainHeight;

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
in vec3  v_Position;
in vec4  v_Orientation;
in float v_fColor;
in int   v_iColor;

out vec3 g_Position;
out vec4 g_Orientation;

flat out vec3 f_ViewCenter;
flat out vec3 f_Color;

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
float rand(vec2 co)
{
    float a  = 12.9898f;
    float b  = 78.233f;
    float c  = 43758.5453f;
    float dt = dot(co.xy, vec2(a, b));
    float sn = mod(dt, 3.14);
    return fract(sin(sn) * c);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
vec3 generateVertexColor()
{
    if(u_ColorMode == COLOR_MODE_RANDOM) {
        return vec3(rand(vec2(gl_VertexID, gl_VertexID)),
                    rand(vec2(gl_VertexID + 1, gl_VertexID)),
                    rand(vec2(gl_VertexID, gl_VertexID + 1)));
    } else if(u_ColorMode == COLOR_MODE_RAMP) {
        float segmentSize = float(u_nParticles) / 6.0f;
        float segment     = floor(float(gl_VertexID) / segmentSize);
        float t           = (float(gl_VertexID) - segmentSize * segment) / segmentSize;
        vec3  startVal    = colorRamp[int(segment)];
        vec3  endVal      = colorRamp[int(segment) + 1];
        return mix(startVal, endVal, t);
    } else if(u_ColorMode == COLOR_MODE_OBJ_INDEX) {
        float t = (float(v_iColor) - u_vColorMin) / (u_vColorMax - u_vColorMin);
        return mix(u_ColorMinVal, u_ColorMaxVal, t);
    } else {
        return vec3(0);
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void main()
{
    vec4 eyeCoord = viewMatrix * vec4(v_Position, 1.0);
    vec3 posEye   = vec3(eyeCoord);
    /////////////////////////////////////////////////////////////////
    if(u_ParticleType == 2) {
        g_Position    = v_Position;
        g_Orientation = v_Orientation;
    }
    f_ViewCenter = posEye;
    f_Color      = generateVertexColor();

    gl_PointSize       = (u_Dimension == 3) ? u_PointRadius * u_PointScale / length(posEye) : u_PointRadius * 2.0 * float(u_ScreenHeight) / u_DomainHeight;
    gl_Position        = projectionMatrix * eyeCoord;
    gl_ClipDistance[0] = dot(vec4(v_Position, 1.0), u_ClipPlane);
}
