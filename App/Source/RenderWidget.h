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

#include <LibQtApps/OpenGLWidget.h>
#include "Forward.h"
#include "VisualizationData.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class RenderWidget : public OpenGLWidget {
    Q_OBJECT
public:
    RenderWidget(QWidget* parent);
    auto& getVizData() const { return m_VizData; }

protected:
    virtual void initOpenGL();
    virtual void resizeOpenGLWindow(int, int);
    virtual void renderOpenGL();
    SharedPtr<VisualizationData> m_VizData = std::make_shared<VisualizationData>();

public:
    void  randomizeDiffuseColors();
    void  setRandomizedDiffuseColor(int vizType, float r, float g, float b);
    Vec4f getRandomizedDiffuseColor(int vizType);

public slots:
    void updateCamera();
    void updateSystemDimension();
    void updateVizData();
    void hideVisualization(int vizType, bool bHide) { m_bRender[vizType] = !bHide; }
    void setParticleDiffuseColorMode(int vizType, int colorMode);
    void enableRandomizedDiffuseColor(bool bEnable) { m_bRandomizeDiffuseColor = bEnable; }
    void setMaterial(int vizType, const MaterialData& material);

protected:
    SharedPtr<Material> m_Materials[VisualizationType::nVisualizationTypes() - 1];
    SharedPtr<Material> m_RndMaterials[VisualizationType::nVisualizationTypes() - 1];
    bool                m_bRender[VisualizationType::nVisualizationTypes()];
    float               m_ParticleRadius         = 0;
    float               m_RadiusScale            = 0;
    bool                m_bRandomizeDiffuseColor = false;

    void initMaterials();
    ////////////////////////////////////////////////////////////////////////////////
    struct {
        SharedPtr<QtAppShaderProgram> shader        = nullptr;
        SharedPtr<OpenGLBuffer>       buffPosition  = nullptr;
        SharedPtr<OpenGLBuffer>       buffColorData = nullptr;
        float                         vColorMin     = 0;
        float                         vColorMax     = 1.0f;

        GLuint VAO;
        GLint  v_Position;
        GLint  v_iColor;
        GLuint ub_CamData;
        GLuint ub_Light;
        GLuint ub_Material;
        GLuint u_nParticles;
        GLuint u_PointRadius;
        GLuint u_PointScale;
        GLuint u_Dimension;
        GLuint u_ScreenHeight;
        GLuint u_DomainHeight;
        GLuint u_ColorMode;
        GLuint u_vColorMin;
        GLuint u_vColorMax;
        GLuint u_ColorMinVal;
        GLuint u_ColorMaxVal;
        GLuint u_ClipPlane;

        GLuint nParticles = 0;
        int    colorMode  = RenderColorMode::Ramp;
        Vec3f  colorMinVal;
        Vec3f  colorMaxVal;

        bool bInitialized = false;
    } m_RDataParticle[VisualizationType::nParticleTypes()];

    void initRDataParticle();
    void initParticleVAO();
    void renderParticles();
};
