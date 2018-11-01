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

#include <LibCommon/Utils/NumberHelpers.h>
#include <LibOpenGL/Camera.h>
#include <LibOpenGL/Lights.h>
#include <LibOpenGL/Material.h>
#include <LibOpenGL/OpenGLBuffer.h>
#include <LibQtApps/QtAppShaderProgram.h>

#include "RenderWidget.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
RenderWidget::RenderWidget(QWidget* parent) : OpenGLWidget(parent) {
    updateCamera();
    for(int vizType = 0; vizType < VisualizationType::nVisualizationTypes(); ++vizType) {
        m_bRender[vizType] = (vizType != VisualizationType::ParticleOrientation); // only disable rendering for orientation
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initOpenGL() {
    initMaterials();
    initRDataParticle();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::resizeOpenGLWindow(int, int height) {
    m_RadiusScale = static_cast<GLfloat>(height) / std::tan(55.0 * 0.5 * M_PI / 180.0);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::renderOpenGL() {
    renderParticles();
    ////////////////////////////////////////////////////////////////////////////////
    if(m_VizData->bCaptureImage && m_VizData->bVizDataUploaded && !m_VizData->bCaptureImageSaved) {
        this->exportScreenToImage(m_VizData->currentFrame);
        m_VizData->bCaptureImageSaved = true;
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::updateCamera() {
    if(m_VizData->systemDimension == 3) {
        m_Camera->setCamera(m_VizData->cameraPosition, m_VizData->cameraFocus, Vec3f(0, 1, 0));
    } else {
        m_Camera->setCamera((m_VizData->domainBMin + m_VizData->domainBMax) * 0.5f + Vec3f(0, 0, 1),
                            (m_VizData->domainBMin + m_VizData->domainBMax) * 0.5f,
                            Vec3f(0, 1, 0));
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::updateSystemDimension() {
    if(m_VizData->systemDimension == 3) {
        m_Camera->setProjection(Camera::PerspectiveProjection);
    } else {
        m_Camera->setCamera((m_VizData->domainBMin + m_VizData->domainBMax) * 0.5f + Vec3f(0, 0, 1),
                            (m_VizData->domainBMin + m_VizData->domainBMax) * 0.5f,
                            Vec3f(0, 1, 0));
        m_Camera->setProjection(Camera::OrthographicProjection);
        m_Camera->setOrthoBox(m_VizData->domainBMin.x * 1.02f, m_VizData->domainBMax.x * 1.02f,
                              m_VizData->domainBMin.y * 1.02f, m_VizData->domainBMax.y * 1.02f);
    }

    makeCurrent();
    initParticleVAO();
    doneCurrent();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::updateVizData() {
    makeCurrent();
    for(int vizType = 0; vizType < VisualizationType::nParticleTypes(); ++vizType) {
        Q_ASSERT(m_RDataParticle[vizType].bInitialized);
        if(m_VizData->nParticles[vizType] > 0 && m_bRender[vizType]) {
            size_t dataSize = m_VizData->nParticles[vizType] * m_VizData->systemDimension * sizeof(float);
            Q_ASSERT(m_VizData->particlePositionPtrs[vizType] != nullptr);
            m_RDataParticle[vizType].buffPosition->uploadDataAsync(m_VizData->particlePositionPtrs[vizType], 0, dataSize);
            if(m_RDataParticle[vizType].colorMode == RenderColorMode::ObjectIndex) {
                dataSize = m_VizData->nParticles[vizType] * sizeof(Int16);
                Q_ASSERT(m_VizData->objectIndexPtrs[vizType] != nullptr);
                m_RDataParticle[vizType].buffColorData->uploadDataAsync(m_VizData->objectIndexPtrs[vizType], 0, dataSize);
                m_RDataParticle[vizType].vColorMin = 0;
                m_RDataParticle[vizType].vColorMax = m_VizData->nObjects[vizType] > 1u ? static_cast<float>(m_VizData->nObjects[vizType] - 1) : 1.0f;
            }
        }
        m_RDataParticle[vizType].nParticles = m_VizData->nParticles[vizType];
    }

    doneCurrent();
    m_ParticleRadius = m_VizData->particleRadius;
    m_VizData->bVizDataUploaded = true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::setParticleDiffuseColorMode(int vizType, int colorMode) {
    Q_ASSERT(colorMode < RenderColorMode::NumColorModes);
    m_RDataParticle[vizType].colorMode = colorMode;
    ////////////////////////////////////////////////////////////////////////////////
    if(colorMode == RenderColorMode::ObjectIndex) {
        updateVizData(); // there is data for object coloring but not uploaded yet
        makeCurrent();
        initParticleVAO();
        doneCurrent();
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::setMaterial(int vizType, const MaterialData& material) {
    Q_ASSERT(vizType < VisualizationType::nVisualizationTypes() - 1 && m_Materials[vizType] != nullptr);
    makeCurrent();
    m_Materials[vizType]->setMaterial(material);
    m_Materials[vizType]->uploadDataToGPU();

    // set the same material except diffuse color
    auto currentRndDiffuse = m_RndMaterials[vizType]->getDiffuseColor();
    m_RndMaterials[vizType]->setMaterial(material);
    m_RndMaterials[vizType]->setDiffuseColor(currentRndDiffuse);
    m_RndMaterials[vizType]->uploadDataToGPU();
    doneCurrent();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::randomizeDiffuseColors() {
    makeCurrent();
    for(int vizType = 0; vizType < VisualizationType::nVisualizationTypes() - 1; ++vizType) {
        m_RndMaterials[vizType]->setDiffuseColor(Vec4f(NumberHelpers::fRand01<float>::vrnd<Vec3f>(), 1.0f));
        m_RndMaterials[vizType]->uploadDataToGPU();
    }
    doneCurrent();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::setRandomizedDiffuseColor(int vizType, float r, float g, float b) {
    m_RndMaterials[vizType]->setDiffuseColor(Vec4f(r, g, b, 1.0f));
    m_RndMaterials[vizType]->uploadDataToGPU();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
Vec4f RenderWidget::getRandomizedDiffuseColor(int vizType) {
    return m_RndMaterials[vizType]->getDiffuseColor();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initMaterials() {
    for(int vizType = 0; vizType < VisualizationType::nVisualizationTypes() - 1; ++vizType) {
        m_Materials[vizType] = std::make_shared<Material>();
        m_Materials[vizType]->setMaterial(DefaultVisualizationParameters::DefaultRenderMaterials[vizType]);
        m_Materials[vizType]->uploadDataToGPU();

        m_RndMaterials[vizType] = std::make_shared<Material>();
        m_RndMaterials[vizType]->setMaterial(DefaultVisualizationParameters::DefaultRenderMaterials[vizType]);
        m_RndMaterials[vizType]->uploadDataToGPU();
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initRDataParticle() {
    for(int vizType = 0; vizType < VisualizationType::nParticleTypes(); ++vizType) {
        m_RDataParticle[vizType].shader = std::make_shared<QtAppShaderProgram>("RenderParticles");
        m_RDataParticle[vizType].shader->addVertexShaderFromResource(":/Shaders/particle.vs.glsl");
        m_RDataParticle[vizType].shader->addFragmentShaderFromResource(":/Shaders/particle.fs.glsl");
        m_RDataParticle[vizType].shader->link();
        ////////////////////////////////////////////////////////////////////////////////
        m_RDataParticle[vizType].v_Position = m_RDataParticle[vizType].shader->getAtributeLocation("v_Position");
        m_RDataParticle[vizType].v_iColor   = m_RDataParticle[vizType].shader->getAtributeLocation("v_iColor");
        ////////////////////////////////////////////////////////////////////////////////
        m_RDataParticle[vizType].ub_CamData  = m_RDataParticle[vizType].shader->getUniformBlockIndex("CameraData");
        m_RDataParticle[vizType].ub_Light    = m_RDataParticle[vizType].shader->getUniformBlockIndex("Lights");
        m_RDataParticle[vizType].ub_Material = m_RDataParticle[vizType].shader->getUniformBlockIndex("Material");
        ////////////////////////////////////////////////////////////////////////////////
        m_RDataParticle[vizType].u_nParticles   = m_RDataParticle[vizType].shader->getUniformLocation("u_nParticles");
        m_RDataParticle[vizType].u_PointRadius  = m_RDataParticle[vizType].shader->getUniformLocation("u_PointRadius");
        m_RDataParticle[vizType].u_PointScale   = m_RDataParticle[vizType].shader->getUniformLocation("u_PointScale");
        m_RDataParticle[vizType].u_Dimension    = m_RDataParticle[vizType].shader->getUniformLocation("u_Dimension");
        m_RDataParticle[vizType].u_ScreenHeight = m_RDataParticle[vizType].shader->getUniformLocation("u_ScreenHeight");
        m_RDataParticle[vizType].u_DomainHeight = m_RDataParticle[vizType].shader->getUniformLocation("u_DomainHeight");
        m_RDataParticle[vizType].u_ColorMode    = m_RDataParticle[vizType].shader->getUniformLocation("u_ColorMode");
        m_RDataParticle[vizType].u_vColorMin    = m_RDataParticle[vizType].shader->getUniformLocation("u_vColorMin");
        m_RDataParticle[vizType].u_vColorMax    = m_RDataParticle[vizType].shader->getUniformLocation("u_vColorMax");
        m_RDataParticle[vizType].u_ColorMinVal  = m_RDataParticle[vizType].shader->getUniformLocation("u_ColorMinVal");
        m_RDataParticle[vizType].u_ColorMaxVal  = m_RDataParticle[vizType].shader->getUniformLocation("u_ColorMaxVal");
        m_RDataParticle[vizType].u_ClipPlane    = m_RDataParticle[vizType].shader->getUniformLocation("u_ClipPlane");
        ////////////////////////////////////////////////////////////////////////////////
        m_RDataParticle[vizType].buffPosition  = std::make_shared<OpenGLBuffer>();
        m_RDataParticle[vizType].buffColorData = std::make_shared<OpenGLBuffer>();
        m_RDataParticle[vizType].buffPosition->createBuffer(GL_ARRAY_BUFFER, 1, nullptr, GL_DYNAMIC_DRAW);
        m_RDataParticle[vizType].buffColorData->createBuffer(GL_ARRAY_BUFFER, 1, nullptr, GL_DYNAMIC_DRAW);
        ////////////////////////////////////////////////////////////////////////////////
        if(vizType == VisualizationType::GhostBoundaryParticle) {
            m_RDataParticle[vizType].colorMode = RenderColorMode::UniformColor;
        }
        ////////////////////////////////////////////////////////////////////////////////
        m_RDataParticle[vizType].bInitialized = true;
    }
    initParticleVAO();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initParticleVAO() {
    for(int vizType = 0; vizType < VisualizationType::nParticleTypes(); ++vizType) {
        Q_ASSERT(m_RDataParticle[vizType].bInitialized);
        glCall(glGenVertexArrays(1, &m_RDataParticle[vizType].VAO));
        glCall(glBindVertexArray(m_RDataParticle[vizType].VAO));
        ////////////////////////////////////////////////////////////////////////////////
        glCall(glEnableVertexAttribArray(m_RDataParticle[vizType].v_Position));
        m_RDataParticle[vizType].buffPosition->bind();
        glCall(glVertexAttribPointer(m_RDataParticle[vizType].v_Position, m_VizData->systemDimension, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid*>(0)));
        ////////////////////////////////////////////////////////////////////////////////
        if(m_RDataParticle[vizType].colorMode == RenderColorMode::ObjectIndex && m_RDataParticle[vizType].buffColorData != nullptr) {
            m_RDataParticle[vizType].buffColorData->bind();
            glCall(glEnableVertexAttribArray(m_RDataParticle[vizType].v_iColor));
            glCall(glVertexAttribIPointer(m_RDataParticle[vizType].v_iColor, 1, GL_SHORT, 0, reinterpret_cast<GLvoid*>(0)));
        }
        ////////////////////////////////////////////////////////////////////////////////
        glCall(glBindVertexArray(0));
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::renderParticles() {
    m_UBufferCamData->bindBufferBase();
    m_Lights->bindUniformBuffer();
    ////////////////////////////////////////////////////////////////////////////////
    for(int vizType = 0; vizType < VisualizationType::nParticleTypes(); ++vizType) {
        Q_ASSERT(m_RDataParticle[vizType].bInitialized);
        if(!m_bRender[vizType] || m_RDataParticle[vizType].nParticles == 0) {
            continue;
        }
        m_RDataParticle[vizType].shader->bind();
        ////////////////////////////////////////////////////////////////////////////////
        m_RDataParticle[vizType].shader->bindUniformBlock(m_RDataParticle[vizType].ub_CamData, m_UBufferCamData->getBindingPoint());
        m_RDataParticle[vizType].shader->bindUniformBlock(m_RDataParticle[vizType].ub_Light,   m_Lights->getBufferBindingPoint());
        ////////////////////////////////////////////////////////////////////////////////
        m_RDataParticle[vizType].shader->setUniformValue(m_RDataParticle[vizType].u_PointRadius,  m_ParticleRadius);
        m_RDataParticle[vizType].shader->setUniformValue(m_RDataParticle[vizType].u_PointScale,   m_RadiusScale);
        m_RDataParticle[vizType].shader->setUniformValue(m_RDataParticle[vizType].u_Dimension,    m_VizData->systemDimension);
        m_RDataParticle[vizType].shader->setUniformValue(m_RDataParticle[vizType].u_ScreenHeight, height());
        m_RDataParticle[vizType].shader->setUniformValue(m_RDataParticle[vizType].u_DomainHeight, (m_Camera->getOrthoBoxMax().y - m_Camera->getOrthoBoxMin().y) * 0.9f);
        m_RDataParticle[vizType].shader->setUniformValue(m_RDataParticle[vizType].u_ClipPlane,    m_ClipPlane);
        ////////////////////////////////////////////////////////////////////////////////
        if(m_RDataParticle[vizType].colorMode == RenderColorMode::ObjectIndex) {
            m_RDataParticle[vizType].shader->setUniformValue(m_RDataParticle[vizType].u_ColorMinVal, m_RDataParticle[vizType].colorMinVal);
            m_RDataParticle[vizType].shader->setUniformValue(m_RDataParticle[vizType].u_ColorMaxVal, m_RDataParticle[vizType].colorMaxVal);
            m_RDataParticle[vizType].shader->setUniformValue(m_RDataParticle[vizType].u_vColorMin, m_RDataParticle[vizType].vColorMin);
            m_RDataParticle[vizType].shader->setUniformValue(m_RDataParticle[vizType].u_vColorMax, m_RDataParticle[vizType].vColorMax);
        }

        if(!m_bRandomizeDiffuseColor) {
            m_Materials[vizType]->bindUniformBuffer();
            m_RDataParticle[vizType].shader->bindUniformBlock(m_RDataParticle[vizType].ub_Material, m_Materials[vizType]->getBufferBindingPoint());
            m_RDataParticle[vizType].shader->setUniformValue(m_RDataParticle[vizType].u_ColorMode, m_RDataParticle[vizType].colorMode);
        } else {
            m_RndMaterials[vizType]->bindUniformBuffer();
            m_RDataParticle[vizType].shader->bindUniformBlock(m_RDataParticle[vizType].ub_Material, m_RndMaterials[vizType]->getBufferBindingPoint());
            m_RDataParticle[vizType].shader->setUniformValue(m_RDataParticle[vizType].u_ColorMode,    RenderColorMode::UniformColor);
        }

        ////////////////////////////////////////////////////////////////////////////////
        glCall(glEnable(GL_VERTEX_PROGRAM_POINT_SIZE));
        glCall(glBindVertexArray(m_RDataParticle[vizType].VAO));
        m_RDataParticle[vizType].shader->setUniformValue(m_RDataParticle[vizType].u_nParticles, m_RDataParticle[vizType].nParticles);
        glCall(glDrawArrays(GL_POINTS, 0, m_RDataParticle[vizType].nParticles));
        ////////////////////////////////////////////////////////////////////////////////
        glCall(glBindVertexArray(0));
        m_RDataParticle[vizType].shader->release();
    }
}
