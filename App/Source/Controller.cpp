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
    setupColorControllers();
    setupOutputControllers();
    setupFrameControllers();
    setupParticleRadiusControllers();
    setupInputControllers();

    setupDataPlayerButtons();
    connectWidgets();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::updateVisualizationParameters(const QString& sceneFile) {
    std::ifstream inFile(sceneFile.toStdString());
    __NT_REQUIRE(inFile.is_open());
    try {
        nlohmann::json jParams = nlohmann::json::parse(inFile);
        inFile.close();
        ////////////////////////////////////////////////////////////////////////////////
        __NT_REQUIRE(jParams.find("VisualizationParameters") != jParams.end());
        auto jVizParams = jParams["VisualizationParameters"];
        {
            ////////////////////////////////////////////////////////////////////////////////
            if(JSONHelpers::readValue(jVizParams, m_RenderWidget->getVizData()->systemDimension, "Dimension")) {
                __NT_REQUIRE(m_RenderWidget->getVizData()->systemDimension == 2 ||
                             m_RenderWidget->getVizData()->systemDimension == 3);
                m_RenderWidget->updateSystemDimension();
            }
            ////////////////////////////////////////////////////////////////////////////////
            // box
            if(jVizParams.find("DomainBox") != jVizParams.end()) {
                JParams jBoxParams = jVizParams["DomainBox"];
                JSONHelpers::readVector(jBoxParams, m_RenderWidget->getVizData()->domainBMin, "BoxMin");
                JSONHelpers::readVector(jBoxParams, m_RenderWidget->getVizData()->domainBMax, "BoxMax");
                m_RenderWidget->setBox(m_RenderWidget->getVizData()->domainBMin, m_RenderWidget->getVizData()->domainBMax);
                m_RenderWidget->updateSystemDimension();
            }
            if(bool bRender; JSONHelpers::readBool(jVizParams, bRender, "RenderDomainBox")) {
                this->m_chkRenderBox->setChecked(bRender);
            } else {
                this->m_chkRenderBox->setChecked(true);
            }
            ////////////////////////////////////////////////////////////////////////////////
            if(Vec3f backgroundColor; JSONHelpers::readVector(jVizParams, backgroundColor, "BackgroundColor")) {
                m_RenderWidget->setClearColor(backgroundColor);
            } else {
                m_RenderWidget->setClearColor(QtAppUtils::getDefaultClearColor());
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
            setMaterial(DefaultVisualizationParameters::DefaultRenderMaterial);
            if(jVizParams.find("ColorAndMaterials") != jVizParams.end()) {
                auto name2ColorMode = [](const auto& name) -> int {
                                          if(name == "Uniform") {
                                              return RenderColorMode::UniformColor;
                                          } else if(name == "Random") {
                                              return RenderColorMode::Random;
                                          } else if(name == "Ramp") {
                                              return RenderColorMode::Ramp;
                                          }
                                          __NT_DIE("Invalid color mode!");
                                          return -1; // to disable warning
                                      };
                for(auto& jColorMaterial :jVizParams["ColorAndMaterials"]) {
                    if(String colorMode; JSONHelpers::readValue(jColorMaterial, colorMode, "ColorMode")) {
                        setParticleDiffuseColorMode(name2ColorMode(colorMode));
                    }
                    if(jColorMaterial.find("Material") != jColorMaterial.end()) {
                        auto& jMaterial    = jColorMaterial["Material"];
                        auto  materialData = DefaultVisualizationParameters::DefaultRenderMaterial;
                        JSONHelpers::readVector(jMaterial, materialData.ambient,  "Ambient");
                        JSONHelpers::readVector(jMaterial, materialData.diffuse,  "Diffuse");
                        JSONHelpers::readVector(jMaterial, materialData.specular, "Specular");
                        JSONHelpers::readValue(jMaterial, materialData.shininess, "Shininess");
                        setMaterial(materialData);
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
            if(Int frameDelay; JSONHelpers::readValue(jVizParams, frameDelay, "FrameDelay")) {
                m_sldFrameDelay->setValue(frameDelay);
            }
            if(Int frameStep; JSONHelpers::readValue(jVizParams, frameStep, "FrameStep")) {
                m_sldFrameStep->setValue(frameStep);
            }
        }
    } catch(std::exception& e) {
        std::cerr << std::endl << std::endl << "Error while loading json scene: " << e.what() << std::endl;
        QMessageBox::critical(nullptr, QString("Error"), QString("Error while loading json scene: ") + QString(e.what()));
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setParticleDiffuseColorMode(int colorMode) {
    (static_cast<QRadioButton*>(m_smColorMode->mapping(colorMode)))->setChecked(true);
    m_RenderWidget->getVizData()->colorMode = colorMode;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setMaterial(const MaterialData& materialData) {
    m_msMaterial->setCustomMaterial(materialData);
    m_msMaterial->setDefaultCustomMaterial(true);
    m_RenderWidget->setMaterial(materialData);
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
void Controller::setParticleRadius(float radius) {
    m_txtParticleRadius->setText(QString("%1").arg(radius));
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::connectWidgets() {
    connect(m_smColorMode, static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mapped), [&](int colorMode) {
                m_RenderWidget->getVizData()->colorMode = colorMode;
            });
    connect(m_msMaterial,      &MaterialSelector::materialChanged, m_RenderWidget, &RenderWidget::setMaterial);
    ////////////////////////////////////////////////////////////////////////////////
    connect(m_OutputPath,      &BrowsePathWidget::pathChanged,     m_RenderWidget, &RenderWidget::setCapturePath);
    connect(m_chkEnableOutput, &QCheckBox::toggled,                [&](bool bEnable) { m_RenderWidget->getVizData()->bCaptureImage = bEnable; });
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // frame Controllers
    connect(m_sldFrameDelay->getSlider(), &QSlider::valueChanged, m_DataReader, &DataReader::setFrameDelayTime);
    connect(m_sldFrameStep->getSlider(),  &QSlider::valueChanged, m_DataReader, &DataReader::setFrameStep);
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // data path
    connect(m_InputPath,  &BrowsePathWidget::pathChanged,     m_DataReader, &DataReader::setSequenceFile);
    connect(m_DataReader, &DataReader::inputSequenceAccepted, [&](const QString& dataPath) {
                m_btnPause->setChecked(false);
                ////////////////////////////////////////////////////////////////////////////////
                QDir dataDir(dataPath);
                dataDir.setNameFilters(QStringList() << "*.json");
                if(dataDir.entryList().count() != 0) {
                    QString sceneFile = dataPath + "/" + dataDir.entryList()[0];
                    updateVisualizationParameters(sceneFile);
                }
            });
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // particle radius
    connect(m_chkOverrideParticleRadius, &QCheckBox::toggled, [&] (bool bChecked) {
                m_RenderWidget->getVizData()->bRadiusOverrided = bChecked;
                if(bChecked) {
                    try {
                        m_RenderWidget->getVizData()->particleRadius = std::stof(m_txtParticleRadius->text().toStdString());
                    } catch(std::exception&) {}
                } else {
                    m_RenderWidget->getVizData()->particleRadius = 0;
                    m_RenderWidget->getVizData()->computeParticleRadius();
                }
            });
    connect(m_txtParticleRadius, &QLineEdit::textChanged, [&](const QString& txt) {
                if(m_RenderWidget->getVizData()->bRadiusOverrided) {
                    try {
                        m_RenderWidget->getVizData()->particleRadius = std::stof(txt.toStdString());
                    } catch(std::exception&) {}
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
    m_msMaterial = new MaterialSelector;
    m_msMaterial->setCustomMaterial(DefaultVisualizationParameters::DefaultRenderMaterial);
    m_msMaterial->setDefaultCustomMaterial(true);
    QVBoxLayout* layoutMaterial = new QVBoxLayout;
    layoutMaterial->addLayout(m_msMaterial->getLayout());
    layoutMaterial->addSpacing(5);
    m_smColorMode = new QSignalMapper(this);
    QRadioButton* rdbColorRandom  = new QRadioButton("Random");
    QRadioButton* rdbColorRamp    = new QRadioButton("Ramp");
    QRadioButton* rdbColorUniform = new QRadioButton("Uniform Color");
    rdbColorRamp->setChecked(true);
    ////////////////////////////////////////////////////////////////////////////////
    QGridLayout* layoutDiffuseColorModes = new QGridLayout;
    layoutDiffuseColorModes->addWidget(rdbColorRandom,  0, 0, 1, 1);
    layoutDiffuseColorModes->addWidget(rdbColorRamp,    0, 1, 1, 1);
    layoutDiffuseColorModes->addWidget(rdbColorUniform, 1, 0, 1, 1);
    ////////////////////////////////////////////////////////////////////////////////
    connect(rdbColorRandom,  SIGNAL(clicked()), m_smColorMode, SLOT(map()));
    connect(rdbColorRamp,    SIGNAL(clicked()), m_smColorMode, SLOT(map()));
    connect(rdbColorUniform, SIGNAL(clicked()), m_smColorMode, SLOT(map()));
    ////////////////////////////////////////////////////////////////////////////////
    m_smColorMode->setMapping(rdbColorRandom,  static_cast<int>(RenderColorMode::Random));
    m_smColorMode->setMapping(rdbColorRamp,    static_cast<int>(RenderColorMode::Ramp));
    m_smColorMode->setMapping(rdbColorUniform, static_cast<int>(RenderColorMode::UniformColor));
    layoutMaterial->addLayout(layoutDiffuseColorModes);
    QGroupBox* grMaterial = new QGroupBox("Material");
    grMaterial->setLayout(layoutMaterial);
    m_LayoutMainControllers->addWidget(grMaterial);
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
    m_InputPath = new BrowsePathWidget("Browse", false);
    QGroupBox* grpInput = new QGroupBox;
    grpInput->setTitle("Input");
    grpInput->setLayout(m_InputPath->getLayout());
    ////////////////////////////////////////////////////////////////////////////////
    m_LayoutMainControllers->addWidget(grpInput);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupParticleRadiusControllers() {
    m_chkOverrideParticleRadius = new QCheckBox("Override radius: ");
    m_txtParticleRadius         = new QLineEdit;
    m_txtParticleRadius->setText("0.01");
    QHBoxLayout* layoutPRadius = new QHBoxLayout;
    layoutPRadius->addWidget(m_chkOverrideParticleRadius);
    layoutPRadius->addWidget(m_txtParticleRadius);
    QGroupBox* grpPRadius = new QGroupBox;
    grpPRadius->setTitle("Particle Radius");
    grpPRadius->setLayout(layoutPRadius);
    m_LayoutMainControllers->addWidget(grpPRadius);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupDataPlayerButtons() {
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
