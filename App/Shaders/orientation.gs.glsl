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

// geometry shader, orientation render
#version 410 core

#define THICK_LINE

layout(points) in;
layout(line_strip, max_vertices = 18) out;

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

uniform float u_PointRadius;
uniform float u_AxisLength   = 2.5f;
uniform int   u_ScreenWidth  = 1920;
uniform int   u_ScreenHeight = 1080;

in vec3  g_Position[];
in vec4  g_Orientation[];
out vec3 fColor;

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void emitLine(vec4 p1, vec4 p2, vec3 color)
{
    gl_Position = p1;
    fColor      = color;
    EmitVertex();
    gl_Position = p2;
    fColor      = color;
    EmitVertex();
    EndPrimitive();

#ifdef THICK_LINE
    vec3 p1_ndc = p1.xyz / p1.w;
    vec3 p2_ndc = p2.xyz / p2.w;

    if(p1_ndc.z < -1.0f || p2_ndc.z < -1.0f) {
        return;
    }

    vec2 dir        = normalize(p2.xy - p1.xy);
    vec2 normal     = vec2(-dir.y, dir.x);
    vec2 offsetUnit = vec2(2.0 / u_ScreenWidth, 2.0 / u_ScreenHeight);
    vec3 offset     = vec3(normal.x * offsetUnit.x, normal.y * offsetUnit.y, 0);

    vec4 p3 = vec4(p1_ndc + offset, 1);
    vec4 p4 = vec4(p2_ndc + offset, 1);
    vec4 p5 = vec4(p1_ndc - offset, 1);
    vec4 p6 = vec4(p2_ndc - offset, 1);

    gl_Position = p3;
    fColor      = color;
    EmitVertex();
    gl_Position = p4;
    fColor      = color;
    EmitVertex();
    EndPrimitive();

    gl_Position = p5;
    fColor      = color;
    EmitVertex();
    gl_Position = p6;
    fColor      = color;
    EmitVertex();
    EndPrimitive();
#endif
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void main()
{
    mat4  VPM = projectionMatrix * viewMatrix;
    vec4  q   = g_Orientation[0];
    float qxx = q.x * q.x;
    float qyy = q.y * q.y;
    float qzz = q.z * q.z;
    float qww = q.w * q.w;
    float qxy = q.x * q.y;
    float qxz = q.x * q.z;
    float qxw = q.x * q.w;
    float qyz = q.y * q.z;
    float qyw = q.y * q.w;
    float qzw = q.z * q.w;
    ////////////////////////////////////////////////////////////////////////////////
    vec3 d1 = vec3(qxx - qyy - qzz + qww,
                   2 * (qxy + qzw),
                   2 * (qxz - qyw));

    vec3 d2 = vec3(2 * (qxy - qzw),
                   -qxx + qyy - qzz + qww,
                   2 * (qyz + qxw));

    vec3 d3 = vec3(2 * (qxz + qyw),
                   2 * (qyz - qxw),
                   -qxx - qyy + qzz + qww);

    emitLine(gl_in[0].gl_Position, VPM * vec4(g_Position[0] + u_PointRadius * u_AxisLength * normalize(d1), 1.0), vec3(1, 0, 0));
    emitLine(gl_in[0].gl_Position, VPM * vec4(g_Position[0] + u_PointRadius * u_AxisLength * normalize(d2), 1.0), vec3(0, 1, 0));
    emitLine(gl_in[0].gl_Position, VPM * vec4(g_Position[0] + u_PointRadius * u_AxisLength * normalize(d3), 1.0), vec3(0, 0, 1));
}
