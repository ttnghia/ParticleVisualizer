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

#pragma once
#include <LibCommon/CommonSetup.h>
#include <LibOpenGL/LightAndMaterialData.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
struct RenderColorMode {
    enum {
        UniformColor = 0,
        Random,
        Ramp,
        ObjectIndex,
        NumColorModes
    };
};

struct VisualizationType {
    enum {
        StandardMPMParticle = 0,
        VertexParticle,
        QuadratureParticle,
        GhostBoundaryParticle,
        TriangleMesh,
        ParticleOrientation
    };
    static constexpr int nParticleTypes() { return 4; }
    static constexpr int nVisualizationTypes() { return 6; }
};

static inline std::map<int, String> VizNames = {
    { VisualizationType::StandardMPMParticle, "StandardParticle" },
    { VisualizationType::VertexParticle, "VertexParticle" },
    { VisualizationType::QuadratureParticle, "QuadParticle" },
    { VisualizationType::GhostBoundaryParticle, "GhostParticle" },
    { VisualizationType::TriangleMesh, "Mesh" }
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
namespace DefaultVisualizationParameters {
static inline Vec3f DefaultCameraPosition = Vec3f(3.0, 0.8, 0);
static inline Vec3f DefaultCameraFocus    = Vec3f(0, -0.2, 0);
static inline Vec4f DefaultLight0Position = Vec4f(-10, 20, 10, 1);
static inline Vec4f DefaultLight1Position = Vec4f(10, -20, -10, 1);

static inline std::map<int, MaterialData> DefaultRenderMaterials = {
    { VisualizationType::StandardMPMParticle, { Vec4f(0.05f, 0.05f, 0.05f, 1.0f),
                                                Vec4f(1.0f,    0, 0.5f, 1.0f),
                                                Vec4f(1.0f, 1.0f, 1.0f, 1.0f),
                                                200.0f,      "CustomMaterial" } },
    { VisualizationType::VertexParticle, { Vec4f(0.05f, 0.05f, 0.05f, 1.0f),
                                           Vec4f(1.0f,    0,    0, 1.0f),
                                           Vec4f(1.0f, 1.0f, 1.0f, 1.0f),
                                           200.0f,      "CustomMaterial" } },
    { VisualizationType::QuadratureParticle, { Vec4f(0.05f, 0.05f, 0.05f, 1.0f),
                                               Vec4f(   0,    0,    1, 1.0f),
                                               Vec4f(1.0f, 1.0f, 1.0f, 1.0f),
                                               200.0f,      "CustomMaterial" } },
    { VisualizationType::GhostBoundaryParticle, { Vec4f(0.15f, 0.15f, 0.15f, 1.0f),
                                                  Vec4f(0.392f,    0,    0, 1.0f),
                                                  Vec4f(  0.5f, 0.5f, 0.5f, 1.0f),
                                                  200.0f,      "CustomMaterial" } },
    { VisualizationType::TriangleMesh, BuildInMaterials::MT_Brass }
};
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
    char*                orientationPtr;
    std::map<Int, char*> particlePositionPtrs;
    std::map<Int, char*> objectIndexPtrs;
    std::map<Int, UInt>  nParticles;
    std::map<Int, UInt>  nObjects;
    float                particleRadius;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // buffer, only used if needed
    std::map<Int, StdVT_Vec4f>  buffsOrientation;
    std::map<Int, StdVT_Vec2f>  buffsPositions2D;
    std::map<Int, StdVT_Vec3f>  buffsPositions3D;
    std::map<Int, StdVT_UInt16> buffsObjectIndex;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // mesh data
    // todo
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
        particleRadius = 0;
        orientationPtr = nullptr;
        particlePositionPtrs.clear();
        objectIndexPtrs.clear();
        nParticles.clear();
        nObjects.clear();
        ////////////////////////////////////////////////////////////////////////////////
        for(auto& [vizType, buffer]: buffsOrientation) {
            buffer.resize(0);
        }
        for(auto& [vizType, buffer]: buffsPositions2D) {
            buffer.resize(0);
        }
        for(auto& [vizType, buffer]: buffsPositions3D) {
            buffer.resize(0);
        }
        for(auto& [vizType, buffer]: buffsObjectIndex) {
            buffer.resize(0);
        }
        ////////////////////////////////////////////////////////////////////////////////
        bVizDataUploaded   = false;
        bCaptureImageSaved = true;
        currentFrame       = 0;
    }
};
