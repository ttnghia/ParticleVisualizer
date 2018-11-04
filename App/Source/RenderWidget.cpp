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
    glCall(glBindVertexArray(m_RDataParticle.VAO));
    glCall(glEnableVertexAttribArray(m_RDataParticle.v_Position));
    m_RDataParticle.buffPosition->bind();
    glCall(glVertexAttribPointer(m_RDataParticle.v_Position, m_VizData->systemDimension, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid*>(0)));
    glCall(glBindVertexArray(0));
    doneCurrent();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::updateVizData() {
    makeCurrent();
    Q_ASSERT(m_RDataParticle.bInitialized);
    if(m_VizData->nParticles > 0) {
        size_t dataSize = m_VizData->nParticles * m_VizData->systemDimension * sizeof(float);
        Q_ASSERT(m_VizData->particlePositionPtrs != nullptr);
        m_RDataParticle.buffPosition->uploadDataAsync(m_VizData->particlePositionPtrs, 0, dataSize);
    }

    doneCurrent();
    m_VizData->bVizDataUploaded = true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::setMaterial(const MaterialData& material) {
    Q_ASSERT(m_RDataParticle.material != nullptr);
    makeCurrent();
    m_RDataParticle.material->setMaterial(material);
    m_RDataParticle.material->uploadDataToGPU();
    doneCurrent();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initMaterials() {
    m_RDataParticle.material = std::make_shared<Material>();
    m_RDataParticle.material->setMaterial(DefaultVisualizationParameters::DefaultRenderMaterial);
    m_RDataParticle.material->uploadDataToGPU();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initRDataParticle() {
    m_RDataParticle.shader = std::make_shared<QtAppShaderProgram>("RenderParticles");
    m_RDataParticle.shader->addVertexShaderFromResource(":/Shaders/particle.vs.glsl");
    m_RDataParticle.shader->addFragmentShaderFromResource(":/Shaders/particle.fs.glsl");
    m_RDataParticle.shader->link();
    ////////////////////////////////////////////////////////////////////////////////
    m_RDataParticle.v_Position  = m_RDataParticle.shader->getAtributeLocation("v_Position");
    m_RDataParticle.ub_CamData  = m_RDataParticle.shader->getUniformBlockIndex("CameraData");
    m_RDataParticle.ub_Light    = m_RDataParticle.shader->getUniformBlockIndex("Lights");
    m_RDataParticle.ub_Material = m_RDataParticle.shader->getUniformBlockIndex("Material");
    ////////////////////////////////////////////////////////////////////////////////
    m_RDataParticle.u_nParticles   = m_RDataParticle.shader->getUniformLocation("u_nParticles");
    m_RDataParticle.u_PointRadius  = m_RDataParticle.shader->getUniformLocation("u_PointRadius");
    m_RDataParticle.u_PointScale   = m_RDataParticle.shader->getUniformLocation("u_PointScale");
    m_RDataParticle.u_Dimension    = m_RDataParticle.shader->getUniformLocation("u_Dimension");
    m_RDataParticle.u_ScreenHeight = m_RDataParticle.shader->getUniformLocation("u_ScreenHeight");
    m_RDataParticle.u_DomainHeight = m_RDataParticle.shader->getUniformLocation("u_DomainHeight");
    m_RDataParticle.u_ColorMode    = m_RDataParticle.shader->getUniformLocation("u_ColorMode");
    m_RDataParticle.u_ClipPlane    = m_RDataParticle.shader->getUniformLocation("u_ClipPlane");
    ////////////////////////////////////////////////////////////////////////////////
    m_RDataParticle.buffPosition = std::make_shared<OpenGLBuffer>();
    m_RDataParticle.buffPosition->createBuffer(GL_ARRAY_BUFFER, 1, nullptr, GL_DYNAMIC_DRAW);
    ////////////////////////////////////////////////////////////////////////////////
    m_RDataParticle.bInitialized = true;
    initParticleVAO();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initParticleVAO() {
    Q_ASSERT(m_RDataParticle.bInitialized);
    glCall(glGenVertexArrays(1, &m_RDataParticle.VAO));
    glCall(glBindVertexArray(m_RDataParticle.VAO));
    ////////////////////////////////////////////////////////////////////////////////
    glCall(glEnableVertexAttribArray(m_RDataParticle.v_Position));
    m_RDataParticle.buffPosition->bind();
    glCall(glVertexAttribPointer(m_RDataParticle.v_Position, m_VizData->systemDimension, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid*>(0)));
    ////////////////////////////////////////////////////////////////////////////////
    glCall(glBindVertexArray(0));
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::renderParticles() {
    ////////////////////////////////////////////////////////////////////////////////
    Q_ASSERT(m_RDataParticle.bInitialized);
    if(m_VizData->nParticles == 0) {
        return;
    }
    m_UBufferCamData->bindBufferBase();
    m_Lights->bindUniformBuffer();
    m_RDataParticle.shader->bind();
    ////////////////////////////////////////////////////////////////////////////////
    m_RDataParticle.shader->bindUniformBlock(m_RDataParticle.ub_CamData, m_UBufferCamData->getBindingPoint());
    m_RDataParticle.shader->bindUniformBlock(m_RDataParticle.ub_Light,   m_Lights->getBufferBindingPoint());
    ////////////////////////////////////////////////////////////////////////////////
    m_RDataParticle.shader->setUniformValue(m_RDataParticle.u_PointRadius,  m_VizData->particleRadius);
    m_RDataParticle.shader->setUniformValue(m_RDataParticle.u_PointScale,   m_RadiusScale);
    m_RDataParticle.shader->setUniformValue(m_RDataParticle.u_Dimension,    m_VizData->systemDimension);
    m_RDataParticle.shader->setUniformValue(m_RDataParticle.u_ScreenHeight, height());
    m_RDataParticle.shader->setUniformValue(m_RDataParticle.u_DomainHeight, (m_Camera->getOrthoBoxMax().y - m_Camera->getOrthoBoxMin().y) * 0.9f);
    m_RDataParticle.shader->setUniformValue(m_RDataParticle.u_ClipPlane,    m_ClipPlane);
    ////////////////////////////////////////////////////////////////////////////////
    m_RDataParticle.material->bindUniformBuffer();
    m_RDataParticle.shader->bindUniformBlock(m_RDataParticle.ub_Material, m_RDataParticle.material->getBufferBindingPoint());
    m_RDataParticle.shader->setUniformValue(m_RDataParticle.u_ColorMode, m_VizData->colorMode);

    ////////////////////////////////////////////////////////////////////////////////
    glCall(glEnable(GL_VERTEX_PROGRAM_POINT_SIZE));
    glCall(glBindVertexArray(m_RDataParticle.VAO));
    m_RDataParticle.shader->setUniformValue(m_RDataParticle.u_nParticles, m_VizData->nParticles);
    glCall(glDrawArrays(GL_POINTS, 0, m_VizData->nParticles));
    ////////////////////////////////////////////////////////////////////////////////
    glCall(glBindVertexArray(0));
    m_RDataParticle.shader->release();
}
