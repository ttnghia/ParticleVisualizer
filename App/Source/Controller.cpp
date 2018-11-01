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
#include <LibCommon/Utils/JSONHelpers.h>
#include <LibCommon/Geometry/GeometryObjects.h>

#include <LibQtApps/BrowsePathWidget.h>
#include <LibQtApps/ColorPicker.h>
#include <LibQtApps/EnhancedComboBox.h>
#include <LibQtApps/EnhancedSlider.h>
#include <LibQtApps/MaterialSelector.h>
#include <LibQtApps/PointLightEditor.h>
#include <LibQtApps/QtAppUtils.h>

#include "Controller.h"
#include "RenderWidget.h"
#include "DataReader.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
Controller::Controller(RenderWidget* renderWidget, DataReader* dataReader, QWidget* parent, int width) :
    OpenGLController(static_cast<OpenGLWidget*>(renderWidget), parent, width), m_RenderWidget(renderWidget), m_DataReader(dataReader) {
    setupGUI();
    setupFrameControllers();
    setupDataPlayerButtons();
    setupInputControllers();
    connectWidgets();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
JParams Controller::updateVisualizationParameters(const QString& sceneFile) {
    std::ifstream inFile(sceneFile.toStdString());
    __NT_REQUIRE(inFile.is_open());
    try {
        nlohmann::json jParams = nlohmann::json::parse(inFile);
        inFile.close();
        ////////////////////////////////////////////////////////////////////////////////
        __NT_REQUIRE(jParams.find("SimulationParameters") != jParams.end());
        {
            JParams jSimParams = jParams["SimulationParameters"];
            __NT_REQUIRE(jSimParams.find("SimulationDomainBox") != jSimParams.end());
            {
                JParams jBoxParams = jSimParams["SimulationDomainBox"];
                jBoxParams["GeometryType"] = String("Box");
                auto box = GeometryObjects::BoxObject<3, float>(jBoxParams);
                m_RenderWidget->getVizData()->domainBMin = box.getTransformedBoxMin() - Vec3f(m_RenderWidget->getVizData()->particleRadius);
                m_RenderWidget->getVizData()->domainBMax = box.getTransformedBoxMax() + Vec3f(m_RenderWidget->getVizData()->particleRadius);
                m_RenderWidget->setBox(m_RenderWidget->getVizData()->domainBMin, m_RenderWidget->getVizData()->domainBMax);
                m_RenderWidget->updateSystemDimension();
            }
        }
        ////////////////////////////////////////////////////////////////////////////////
        if(jParams.find("VisualizationParameters") != jParams.end()) {
            auto jVizParams = jParams["VisualizationParameters"];
            ////////////////////////////////////////////////////////////////////////////////
            if(Vec3f backgroundColor; JSONHelpers::readVector(jVizParams, backgroundColor, "BackgroundColor")) {
                m_RenderWidget->setClearColor(backgroundColor);
            } else {
                m_RenderWidget->setClearColor(QtAppUtils::getDefaultClearColor());
            }

            if(bool bRender; JSONHelpers::readBool(jVizParams, bRender, "RenderDomainBox")) {
                this->m_chkRenderBox->setChecked(bRender);
            } else {
                this->m_chkRenderBox->setChecked(true);
            }
            ////////////////////////////////////////////////////////////////////////////////
            if(!JSONHelpers::readVector(jVizParams, m_RenderWidget->getVizData()->cameraPosition, "CameraPosition")) {
                m_RenderWidget->getVizData()->cameraPosition = DefaultVisualizationParameters::DefaultCameraPosition;
            }
            if(!JSONHelpers::readVector(jVizParams, m_RenderWidget->getVizData()->cameraFocus, "CameraFocus")) {
                m_RenderWidget->getVizData()->cameraFocus = DefaultVisualizationParameters::DefaultCameraFocus;
            }
            m_RenderWidget->updateCamera();
            ////////////////////////////////////////////////////////////////////////////////
            if(String capturePath; JSONHelpers::readValue(jVizParams, capturePath, "CapturePath")) {
                m_OutputPath->setPath(QString::fromStdString(capturePath));
            } else {
                m_OutputPath->setPath(QtAppUtils::getDefaultCapturePath());
            }
            ////////////////////////////////////////////////////////////////////////////////
            for(int vizType = 0; vizType < VisualizationType::nVisualizationTypes() - 1; ++vizType) {
                setMaterial(vizType, DefaultVisualizationParameters::DefaultRenderMaterials[vizType]);
            }
            if(jVizParams.find("ColorAndMaterials") != jVizParams.end()) {
                auto name2VizType = [](const auto& name) -> int {
                                        for(int vizType = 0; vizType < VisualizationType::nVisualizationTypes(); ++vizType) {
                                            if(VizNames[vizType] == name) {
                                                return vizType;
                                            }
                                        }
                                        __NT_DIE("Invalid visualization name!");
                                        return -1; // to disable warning
                                    };
                auto name2ColorMode = [](const auto& name) -> int {
                                          if(name == "Uniform") {
                                              return RenderColorMode::UniformColor;
                                          } else if(name == "Random") {
                                              return RenderColorMode::Random;
                                          } else if(name == "Ramp") {
                                              return RenderColorMode::Ramp;
                                          } else if(name == "ObjectIndex") {
                                              return RenderColorMode::ObjectIndex;
                                          }
                                          __NT_DIE("Invalid color mode!");
                                          return -1; // to disable warning
                                      };
                for(auto& jColorMaterial :jVizParams["ColorAndMaterials"]) {
                    __NT_REQUIRE(jColorMaterial.find("VizType") != jColorMaterial.end());
                    auto vizType = name2VizType(jColorMaterial["VizType"]);

                    if(String colorMode; JSONHelpers::readValue(jColorMaterial, colorMode, "ColorMode")) {
                        setParticleDiffuseColorMode(vizType, name2ColorMode(colorMode));
                    }
                    if(jColorMaterial.find("Material") != jColorMaterial.end()) {
                        auto& jMaterial    = jColorMaterial["Material"];
                        auto  materialData = DefaultVisualizationParameters::DefaultRenderMaterials[vizType];
                        JSONHelpers::readVector(jMaterial, materialData.ambient,  "Ambient");
                        JSONHelpers::readVector(jMaterial, materialData.diffuse,  "Diffuse");
                        JSONHelpers::readVector(jMaterial, materialData.specular, "Specular");
                        JSONHelpers::readValue(jMaterial, materialData.shininess, "Shininess");
                        setMaterial(vizType, materialData);
                    }
                }
            }

            if(jVizParams.find("Lights") != jVizParams.end()) {
                m_RenderWidget->getVizData()->lights.resize(0);
                for(auto& jLight : jVizParams["Lights"]) {
                    m_RenderWidget->getVizData()->lights.emplace_back(PointLightData());
                    Vec3f tmp;
                    if(JSONHelpers::readVector(jLight, tmp, "Position")) { m_RenderWidget->getVizData()->lights.back().position = Vec4f(tmp, 1.0f); }
                    if(JSONHelpers::readVector(jLight, tmp, "Ambient")) { m_RenderWidget->getVizData()->lights.back().ambient = Vec4f(tmp, 1.0f); }
                    if(JSONHelpers::readVector(jLight, tmp, "Diffuse")) { m_RenderWidget->getVizData()->lights.back().diffuse = Vec4f(tmp, 1.0f); }
                    if(JSONHelpers::readVector(jLight, tmp, "Specular")) { m_RenderWidget->getVizData()->lights.back().specular = Vec4f(tmp, 1.0f); }
                }
            }
            m_LightEditor->changeLights(m_RenderWidget->getVizData()->lights);

            ////////////////////////////////////////////////////////////////////////////////
            if(bool randomizeDiffuseColors; JSONHelpers::readBool(jVizParams, randomizeDiffuseColors, "RandomizeDiffuseColors")) {
                m_chkOverrideDiffuseColor->setChecked(randomizeDiffuseColors);
            }
            ////////////////////////////////////////////////////////////////////////////////
            if(bool bHide; JSONHelpers::readBool(jVizParams, bHide, "HideStandardParticle")) {
                m_btnHideVisualization[VisualizationType::StandardMPMParticle]->setChecked(bHide);
            }
            if(bool bHide; JSONHelpers::readBool(jVizParams, bHide, "HideVertexParticle")) {
                m_btnHideVisualization[VisualizationType::VertexParticle]->setChecked(bHide);
            }
            if(bool bHide; JSONHelpers::readBool(jVizParams, bHide, "HideQuadParticle")) {
                m_btnHideVisualization[VisualizationType::QuadratureParticle]->setChecked(bHide);
            }
            if(bool bHide; JSONHelpers::readBool(jVizParams, bHide, "HideGhostParticle")) {
                m_btnHideVisualization[VisualizationType::GhostBoundaryParticle]->setChecked(bHide);
            }
            if(bool bHide; JSONHelpers::readBool(jVizParams, bHide, "HideOrientation")) {
                m_btnHideVisualization[VisualizationType::ParticleOrientation]->setChecked(bHide);
            }
            if(bool bHide; JSONHelpers::readBool(jVizParams, bHide, "HideMesh")) {
                m_btnHideVisualization[VisualizationType::TriangleMesh]->setChecked(bHide);
            }
            return jVizParams;
        }
    } catch(std::exception& e) {
        std::cerr << std::endl << std::endl << "Error while loading json scene: " << e.what() << std::endl;
        QMessageBox::critical(nullptr, QString("Error"), QString("Error while loading json scene: ") + QString(e.what()));
    }

    return JParams();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setParticleDiffuseColorMode(int vizType, int colorMode) {
    (static_cast<QRadioButton*>(m_smColorMode[vizType]->mapping(colorMode)))->setChecked(true);
    m_RenderWidget->setParticleDiffuseColorMode(vizType, colorMode);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setMaterial(int vizType, const MaterialData& materialData) {
    m_msMaterial[vizType]->setCustomMaterial(materialData);
    m_msMaterial[vizType]->setDefaultCustomMaterial(true);
    m_RenderWidget->setMaterial(vizType, materialData);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::hideVisualization(int vizType, bool bHide) {
    m_btnHideVisualization[vizType]->setChecked(bHide);
    m_RenderWidget->hideVisualization(vizType, bHide);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::browseOutputPath() {
    m_OutputPath->browse();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::browseInputPath() {
    m_InputPath->browse();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::increaseDelay() {
    auto maxVal = m_sldFrameDelay->getMaxValue();
    auto newVal = m_sldFrameDelay->getValue() + 1;
    if(newVal <= maxVal) {
        m_sldFrameDelay->setValue(newVal);
    }
}

void Controller::decreaseDelay() {
    auto minVal = m_sldFrameDelay->getMinValue();
    auto newVal = m_sldFrameDelay->getValue() - 1;
    if(newVal >= minVal) {
        m_sldFrameDelay->setValue(newVal);
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setInputPath(const QString& dataPath) {
    m_InputPath->setPath(dataPath);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupGUI() {
    setupColorControllers();
    setupOutputControllers();
    setupParticleVizButtons();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::connectWidgets() {
    connect(m_cbVizType->getComboBox(), static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [&](int index) {
                QtAppUtils::setVisibleAll(m_msMaterial[m_MaterialActiveIndex]->getLayout(), false);
                QtAppUtils::setVisibleAll(m_msMaterial[index]->getLayout(),                 true);
                QtAppUtils::setVisibleAll(m_loDiffuseColorModes[m_MaterialActiveIndex],     false);
                QtAppUtils::setVisibleAll(m_loDiffuseColorModes[index],                     true);
                m_MaterialActiveIndex = index;
            });

    connect(m_chkOverrideDiffuseColor, &QCheckBox::toggled, m_RenderWidget, &RenderWidget::enableRandomizedDiffuseColor);
    connect(m_btnRndAllColor,          &QPushButton::clicked,
            [&] {
                m_RenderWidget->randomizeDiffuseColors();
                for(int vizType = 0; vizType < VisualizationType::nVisualizationTypes() - 1; ++vizType) {
                    m_pkrColor[vizType]->setColor(m_RenderWidget->getRandomizedDiffuseColor(vizType));
                }
            });

    for(int vizType = 0; vizType < VisualizationType::nVisualizationTypes() - 1; ++vizType) {
        connect(m_smColorMode[vizType], static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mapped), [&, vizType](int colorMode) {
                    m_RenderWidget->setParticleDiffuseColorMode(vizType, colorMode);
                });
        connect(m_msMaterial[vizType], &MaterialSelector::materialChanged, [&, vizType](const MaterialData& material) {
                    m_RenderWidget->setMaterial(vizType, material);
                });
    }
    ////////////////////////////////////////////////////////////////////////////////
    connect(m_OutputPath,      &BrowsePathWidget::pathChanged, m_RenderWidget, &RenderWidget::setCapturePath);
    connect(m_chkEnableOutput, &QCheckBox::toggled,            [&](bool bEnable) { m_RenderWidget->getVizData()->bCaptureImage = bEnable; });
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // buttons
    for(int vizType = 0; vizType < VisualizationType::nVisualizationTypes(); ++vizType) {
        connect(m_btnHideVisualization[vizType], &QPushButton::toggled, [&, vizType](bool bHide) {
                    m_RenderWidget->hideVisualization(vizType, bHide);
                    if(!bHide) {
                        m_RenderWidget->updateVizData();
                    }
                });
    }
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // frame Controllers
    connect(m_sldFrameDelay->getSlider(), &QSlider::valueChanged, m_DataReader, &DataReader::setFrameDelayTime);
    connect(m_sldFrameStep->getSlider(),  &QSlider::valueChanged, m_DataReader, &DataReader::setFrameStep);
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // data path
    connect(m_InputPath,  &BrowsePathWidget::pathChanged, m_DataReader, &DataReader::setInputPath);
    connect(m_DataReader, &DataReader::inputPathAccepted, [&](const QString& dataPath) {
                m_btnPause->setChecked(false);
                ////////////////////////////////////////////////////////////////////////////////
                QDir dataDir(dataPath);
                dataDir.setNameFilters(QStringList() << "*.json");
                __NT_REQUIRE(dataDir.entryList().count() != 0);
                QString sceneFile = dataPath + "/" + dataDir.entryList()[0];
                auto jVizParams   = updateVisualizationParameters(sceneFile);
                if(Int frameDelay; JSONHelpers::readValue(jVizParams, frameDelay, "FrameDelay")) {
                    m_sldFrameDelay->setValue(frameDelay);
                }
                if(Int frameStep; JSONHelpers::readValue(jVizParams, frameStep, "FrameStep")) {
                    m_sldFrameStep->setValue(frameStep);
                }
            });
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // buttons
    connect(m_btnPause,      &QPushButton::clicked, m_DataReader, &DataReader::pause);
    connect(m_btnNextFrame,  &QPushButton::clicked, m_DataReader, &DataReader::readNextFrame);
    connect(m_btnReset,      &QPushButton::clicked, m_DataReader, &DataReader::readFirstFrame);
    connect(m_btnRepeatPlay, &QPushButton::clicked, m_DataReader, &DataReader::enableRepeat);
    connect(m_btnReverse,    &QPushButton::clicked, m_DataReader, &DataReader::enableReverse);
    ////////////////////////////////////////////////////////////////////////////////
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupColorControllers() {
    m_cbVizType = new EnhancedComboBox;
    m_cbVizType->addItem("Standard MPM Particle");
    m_cbVizType->addItem("Vertex Particle");
    m_cbVizType->addItem("Quad Particle");
    m_cbVizType->addItem("Ghost Boundary Particle");
    m_cbVizType->addItem("Triangle Mesh");
    QVBoxLayout* layoutMaterial = new QVBoxLayout;
    layoutMaterial->addLayout(m_cbVizType->getLayout());
    layoutMaterial->addSpacing(5);
    for(int vizType = 0; vizType < VisualizationType::nVisualizationTypes() - 1; ++vizType) {
        m_msMaterial[vizType] = new MaterialSelector;
        m_msMaterial[vizType]->setCustomMaterial(DefaultVisualizationParameters::DefaultRenderMaterials[vizType]);
        m_msMaterial[vizType]->setDefaultCustomMaterial(true);
        layoutMaterial->addLayout(m_msMaterial[vizType]->getLayout());
        if(vizType > 0) {
            QtAppUtils::setVisibleAll(m_msMaterial[vizType]->getLayout(), false);
        }
    }
    m_MaterialActiveIndex = 0;
    layoutMaterial->addSpacing(5);
    for(int vizType = 0; vizType < VisualizationType::nVisualizationTypes() - 1; ++vizType) {
        m_loDiffuseColorModes[vizType] = new QGridLayout;
        m_smColorMode[vizType]         = new QSignalMapper(this);
        QRadioButton* rdbColorRandom  = new QRadioButton("Random");
        QRadioButton* rdbColorRamp    = new QRadioButton("Ramp");
        QRadioButton* rdbColorUniform = new QRadioButton("Uniform Color");
        QRadioButton* rdbColorObjIdx  = new QRadioButton("Object Index");
        rdbColorRamp->setChecked(vizType != VisualizationType::TriangleMesh);
        rdbColorUniform->setChecked(vizType == VisualizationType::GhostBoundaryParticle || vizType == VisualizationType::TriangleMesh);
        ////////////////////////////////////////////////////////////////////////////////
        m_ColorModeGroups[vizType] = new QButtonGroup;
        m_ColorModeGroups[vizType]->addButton(rdbColorRandom);
        m_ColorModeGroups[vizType]->addButton(rdbColorRamp);
        m_ColorModeGroups[vizType]->addButton(rdbColorUniform);
        m_ColorModeGroups[vizType]->addButton(rdbColorObjIdx);
        m_loDiffuseColorModes[vizType]->addWidget(rdbColorRandom,  0, 0, 1, 1);
        m_loDiffuseColorModes[vizType]->addWidget(rdbColorRamp,    0, 1, 1, 1);
        m_loDiffuseColorModes[vizType]->addWidget(rdbColorUniform, 1, 0, 1, 1);
        m_loDiffuseColorModes[vizType]->addWidget(rdbColorObjIdx,  1, 1, 1, 1);
        ////////////////////////////////////////////////////////////////////////////////
        connect(rdbColorRandom,  SIGNAL(clicked()), m_smColorMode[vizType], SLOT(map()));
        connect(rdbColorRamp,    SIGNAL(clicked()), m_smColorMode[vizType], SLOT(map()));
        connect(rdbColorUniform, SIGNAL(clicked()), m_smColorMode[vizType], SLOT(map()));
        connect(rdbColorObjIdx,  SIGNAL(clicked()), m_smColorMode[vizType], SLOT(map()));
        ////////////////////////////////////////////////////////////////////////////////
        m_smColorMode[vizType]->setMapping(rdbColorRandom,  static_cast<int>(RenderColorMode::Random));
        m_smColorMode[vizType]->setMapping(rdbColorRamp,    static_cast<int>(RenderColorMode::Ramp));
        m_smColorMode[vizType]->setMapping(rdbColorUniform, static_cast<int>(RenderColorMode::UniformColor));
        m_smColorMode[vizType]->setMapping(rdbColorObjIdx,  static_cast<int>(RenderColorMode::ObjectIndex));
        if(vizType == VisualizationType::TriangleMesh) {
            rdbColorRandom->setDisabled(true);
            rdbColorRamp->setDisabled(true);
            rdbColorObjIdx->setDisabled(true);
        }
        layoutMaterial->addLayout(m_loDiffuseColorModes[vizType]);
        if(vizType > 0) {
            QtAppUtils::setVisibleAll(m_loDiffuseColorModes[vizType], false);
        }
    }
    QGroupBox* grMaterial = new QGroupBox("Material");
    grMaterial->setLayout(layoutMaterial);
    m_LayoutMainControllers->addWidget(grMaterial);
    ////////////////////////////////////////////////////////////////////////////////
    m_chkOverrideDiffuseColor = new QCheckBox("Randomize diffuse color");
    QVBoxLayout* layoutOverrideDiffuseColor = new QVBoxLayout;
    layoutOverrideDiffuseColor->addWidget(m_chkOverrideDiffuseColor);
    QHBoxLayout* layoutRndColor = new QHBoxLayout;
    for(int vizType = 0; vizType < VisualizationType::nVisualizationTypes() - 1; ++vizType) {
        m_pkrColor[vizType] = new ColorPicker;
        m_pkrColor[vizType]->setColor(DefaultVisualizationParameters::DefaultRenderMaterials[vizType].diffuse);
        layoutRndColor->addStretch(1);
        layoutRndColor->addWidget(m_pkrColor[vizType], 10);
        layoutRndColor->addStretch(1);
        connect(m_pkrColor[vizType], &ColorPicker::colorChanged, [&, vizType](float r, float g, float b) {
                    m_RenderWidget->setRandomizedDiffuseColor(vizType, r, g, b);
                });
    }
    layoutRndColor->addStretch(1);
    m_btnRndAllColor = new QPushButton("Rnd");
    layoutRndColor->addWidget(m_btnRndAllColor, 10);
    layoutOverrideDiffuseColor->addWidget(QtAppUtils::getLineSeparator());
    layoutOverrideDiffuseColor->addSpacing(5);
    layoutOverrideDiffuseColor->addLayout(layoutRndColor);
    ////////////////////////////////////////////////////////////////////////////////
    QVBoxLayout* layoutColorCtrls = new QVBoxLayout;
    layoutColorCtrls->addLayout(layoutOverrideDiffuseColor);
    ////////////////////////////////////////////////////////////////////////////////
    QGroupBox* grColorMode = new QGroupBox("Override Diffuse Color");
    grColorMode->setLayout(layoutColorCtrls);
    m_LayoutMainControllers->addWidget(grColorMode);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupOutputControllers() {
    m_OutputPath = new BrowsePathWidget("Browse");
    m_OutputPath->setPath(QtAppUtils::getDefaultCapturePath());
    m_chkEnableOutput = new QCheckBox("Export to Images");
    QVBoxLayout* layoutOutput = new QVBoxLayout;
    layoutOutput->addWidget(m_chkEnableOutput);
    layoutOutput->addLayout(m_OutputPath->getLayout());
    QGroupBox* grpOutput = new QGroupBox;
    grpOutput->setTitle("Screenshot");
    grpOutput->setLayout(layoutOutput);
    ////////////////////////////////////////////////////////////////////////////////
    m_LayoutMainControllers->addWidget(grpOutput);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupFrameControllers() {
    m_sldFrameDelay = new EnhancedSlider;
    m_sldFrameDelay->setRange(0, 500);
    m_sldFrameDelay->setTracking(false);
    m_sldFrameDelay->setValue(0);
    ///////////////////////////////////////////////////////////////////////////////
    m_sldFrameStep = new EnhancedSlider;
    m_sldFrameStep->setRange(1, 100);
    m_sldFrameStep->setTracking(false);
    ////////////////////////////////////////////////////////////////////////////////
    QVBoxLayout* layoutFrameCtr = new QVBoxLayout;
    layoutFrameCtr->addLayout(m_sldFrameDelay->getLayoutWithLabel("Delay:"));
    layoutFrameCtr->addLayout(m_sldFrameStep->getLayoutWithLabel("Step:"));
    ////////////////////////////////////////////////////////////////////////////////
    QGroupBox* grpFrameControl = new QGroupBox("Frame Controls");
    grpFrameControl->setLayout(layoutFrameCtr);
    m_LayoutMainControllers->addWidget(grpFrameControl);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupInputControllers() {
    m_InputPath = new BrowsePathWidget("Browse");
    QGroupBox* grpInput = new QGroupBox;
    grpInput->setTitle("Input");
    grpInput->setLayout(m_InputPath->getLayout());
    ////////////////////////////////////////////////////////////////////////////////
    m_LayoutMainControllers->addWidget(grpInput);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupParticleVizButtons() {
    m_btnHideVisualization[VisualizationType::StandardMPMParticle]   = new QPushButton("Hide Standard");
    m_btnHideVisualization[VisualizationType::VertexParticle]        = new QPushButton("Hide Vertex");
    m_btnHideVisualization[VisualizationType::QuadratureParticle]    = new QPushButton("Hide Quad");
    m_btnHideVisualization[VisualizationType::GhostBoundaryParticle] = new QPushButton("Hide Ghost");
    m_btnHideVisualization[VisualizationType::ParticleOrientation]   = new QPushButton("Hide Orientation");
    m_btnHideVisualization[VisualizationType::TriangleMesh]          = new QPushButton("Hide Mesh");
    for(int vizType = 0; vizType < VisualizationType::nVisualizationTypes(); ++vizType) {
        m_btnHideVisualization[vizType]->setCheckable(true);
    }
    m_btnHideVisualization[VisualizationType::ParticleOrientation]->setChecked(true);
    m_LayoutButtons->addWidget(m_btnHideVisualization[VisualizationType::StandardMPMParticle],   m_nButtonRows,   0, 1, 1);
    m_LayoutButtons->addWidget(m_btnHideVisualization[VisualizationType::VertexParticle],        m_nButtonRows++, 1, 1, 1);
    m_LayoutButtons->addWidget(m_btnHideVisualization[VisualizationType::QuadratureParticle],    m_nButtonRows,   0, 1, 1);
    m_LayoutButtons->addWidget(m_btnHideVisualization[VisualizationType::GhostBoundaryParticle], m_nButtonRows++, 1, 1, 1);
    m_LayoutButtons->addWidget(m_btnHideVisualization[VisualizationType::ParticleOrientation],   m_nButtonRows,   0, 1, 1);
    m_LayoutButtons->addWidget(m_btnHideVisualization[VisualizationType::TriangleMesh],          m_nButtonRows++, 1, 1, 1);

    m_btnPause = new QPushButton(QString("Pause"));
    m_btnPause->setCheckable(true);
    m_btnNextFrame = new QPushButton(QString("Next Frame"));
    m_btnReset     = new QPushButton(QString("Reset"));
    m_btnReverse   = new QPushButton(QString("Reverse"));
    m_btnReverse->setCheckable(true);
    m_btnRepeatPlay = new QPushButton(QString("Repeat"));
    m_btnRepeatPlay->setCheckable(true);
    ////////////////////////////////////////////////////////////////////////////////
    this->m_LayoutButtons->addWidget(QtAppUtils::getLineSeparator(), m_nButtonRows++, 0, 1, 2);

    QHBoxLayout* loResetReverseRepeat = new QHBoxLayout;
    loResetReverseRepeat->addWidget(m_btnReset);
    loResetReverseRepeat->addWidget(m_btnReverse);
    loResetReverseRepeat->addWidget(m_btnRepeatPlay);
    this->m_LayoutButtons->addLayout(loResetReverseRepeat, m_nButtonRows++, 0, 1, 2);

    this->m_LayoutButtons->addWidget(m_btnPause, m_nButtonRows, 0, 1, 1);
    this->m_LayoutButtons->addWidget(m_btnNextFrame, m_nButtonRows++, 1, 1, 1);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupDataPlayerButtons() {
    return;
    m_btnHideVisualization[VisualizationType::StandardMPMParticle]   = new QPushButton("Hide Standard");
    m_btnHideVisualization[VisualizationType::VertexParticle]        = new QPushButton("Hide Vertex");
    m_btnHideVisualization[VisualizationType::QuadratureParticle]    = new QPushButton("Hide Quad");
    m_btnHideVisualization[VisualizationType::GhostBoundaryParticle] = new QPushButton("Hide Ghost");
    m_btnHideVisualization[VisualizationType::ParticleOrientation]   = new QPushButton("Hide Orientation");
    m_btnHideVisualization[VisualizationType::TriangleMesh]          = new QPushButton("Hide Mesh");
    for(int vizType = 0; vizType < VisualizationType::nVisualizationTypes(); ++vizType) {
        m_btnHideVisualization[vizType]->setCheckable(true);
    }
    m_btnHideVisualization[VisualizationType::ParticleOrientation]->setChecked(true);
    m_LayoutButtons->addWidget(m_btnHideVisualization[VisualizationType::StandardMPMParticle],   m_nButtonRows,   0, 1, 1);
    m_LayoutButtons->addWidget(m_btnHideVisualization[VisualizationType::VertexParticle],        m_nButtonRows++, 1, 1, 1);
    m_LayoutButtons->addWidget(m_btnHideVisualization[VisualizationType::QuadratureParticle],    m_nButtonRows,   0, 1, 1);
    m_LayoutButtons->addWidget(m_btnHideVisualization[VisualizationType::GhostBoundaryParticle], m_nButtonRows++, 1, 1, 1);
    m_LayoutButtons->addWidget(m_btnHideVisualization[VisualizationType::ParticleOrientation],   m_nButtonRows,   0, 1, 1);
    m_LayoutButtons->addWidget(m_btnHideVisualization[VisualizationType::TriangleMesh],          m_nButtonRows++, 1, 1, 1);

    m_btnPause = new QPushButton(QString("Pause"));
    m_btnPause->setCheckable(true);
    m_btnNextFrame = new QPushButton(QString("Next Frame"));
    m_btnReset     = new QPushButton(QString("Reset"));
    m_btnReverse   = new QPushButton(QString("Reverse"));
    m_btnReverse->setCheckable(true);
    m_btnRepeatPlay = new QPushButton(QString("Repeat"));
    m_btnRepeatPlay->setCheckable(true);
    ////////////////////////////////////////////////////////////////////////////////
    this->m_LayoutButtons->addWidget(QtAppUtils::getLineSeparator(), m_nButtonRows++, 0, 1, 2);

    QHBoxLayout* loResetReverseRepeat = new QHBoxLayout;
    loResetReverseRepeat->addWidget(m_btnReset);
    loResetReverseRepeat->addWidget(m_btnReverse);
    loResetReverseRepeat->addWidget(m_btnRepeatPlay);
    this->m_LayoutButtons->addLayout(loResetReverseRepeat, m_nButtonRows++, 0, 1, 2);

    this->m_LayoutButtons->addWidget(m_btnPause, m_nButtonRows, 0, 1, 1);
    this->m_LayoutButtons->addWidget(m_btnNextFrame, m_nButtonRows++, 1, 1, 1);
}
