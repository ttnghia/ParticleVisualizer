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

#pragma once
#include <LibCommon/CommonSetup.h>
#include <LibOpenGL/LightAndMaterialData.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
struct RenderColorMode {
    enum {
        UniformColor = 0,
        Random,
        Ramp,
        NumColorModes
    };
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
namespace DefaultVisualizationParameters {
static inline Vec3f DefaultCameraPosition = Vec3f(3.0, 0.8, 0);
static inline Vec3f DefaultCameraFocus    = Vec3f(0, -0.2, 0);
static inline Vec4f DefaultLight0Position = Vec4f(-10, 20, 10, 1);
static inline Vec4f DefaultLight1Position = Vec4f(10, -20, -10, 1);

static inline MaterialData DefaultRenderMaterial = { Vec4f(0.05f, 0.05f, 0.05f, 1.0f),
                                                     Vec4f(1.0f,    0, 0.5f, 1.0f),
                                                     Vec4f(1.0f, 1.0f, 1.0f, 1.0f),
                                                     200.0f,      "CustomMaterial" };
} // end namespace DefaultVisualizationParameters

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
struct VisualizationData {
    ////////////////////////////////////////////////////////////////////////////////
    Int   systemDimension;
    Vec3f cameraPosition;
    Vec3f cameraFocus;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // domain box
    Vec3f domainBMin;
    Vec3f domainBMax;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // light
    StdVT<PointLightData> lights;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // visualization data
    char* particlePositionPtrs;
    UInt  nParticles;
    float particleRadius;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // buffer, only used if needed
    StdVT_Vec2f buffPositions2D;
    StdVT_Vec3f buffPositions3D;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // opengl upload and capture
    bool bVizDataUploaded   = false;
    bool bCaptureImage      = false;
    bool bCaptureImageSaved = true;
    int  currentFrame       = 0;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    VisualizationData() { resetData(); }
    void resetData() {
        systemDimension = 3;
        cameraPosition  = DefaultVisualizationParameters::DefaultCameraPosition;
        cameraFocus     = DefaultVisualizationParameters::DefaultCameraFocus;
        ////////////////////////////////////////////////////////////////////////////////
        domainBMin = Vec3f(-1.0);
        domainBMax = Vec3f(1.0);
        ////////////////////////////////////////////////////////////////////////////////
        lights.resize(2);
        lights[0].position = DefaultVisualizationParameters::DefaultLight0Position;
        lights[1].position = DefaultVisualizationParameters::DefaultLight1Position;
        ////////////////////////////////////////////////////////////////////////////////
        particleRadius       = 0;
        nParticles           = 0;
        particlePositionPtrs = nullptr;
        buffPositions2D.resize(0);
        buffPositions3D.resize(0);
        ////////////////////////////////////////////////////////////////////////////////
        bVizDataUploaded   = false;
        bCaptureImageSaved = true;
        currentFrame       = 0;
    }
};
