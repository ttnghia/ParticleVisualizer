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

#include <LibOpenGL/LightAndMaterialData.h>

#include <LibQtApps/Forward.h>
#include <LibQtApps/OpenGLController.h>

#include "Forward.h"
#include "VisualizationData.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class Controller : public OpenGLController {
    Q_OBJECT
    friend class MainWindow;
public:
    explicit Controller(RenderWidget* renderWidget, DataReader* dataReader, QWidget* parent = nullptr, int width = 300);
    JParams updateVisualizationParameters(const QString& sceneFile); // return jparams for visualization parameters
    void    setParticleDiffuseColorMode(int vizType, int colorMode);
    void    setMaterial(int vizType, const MaterialData& materialData);
    void    hideVisualization(int vizType, bool bHide);
    void    browseOutputPath();

    void browseInputPath();
    void increaseDelay();
    void decreaseDelay();

public slots:
    void setInputPath(const QString& dataPath);

protected:
    void setupGUI();
    void connectWidgets();

    ////////////////////////////////////////////////////////////////////////////////
    RenderWidget* m_RenderWidget = nullptr;
    DataReader*   m_DataReader   = nullptr;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    void setupColorControllers();
    EnhancedComboBox* m_cbVizType;
    int               m_MaterialActiveIndex;
    MaterialSelector* m_msMaterial[VisualizationType::nVisualizationTypes() - 1];
    QSignalMapper*    m_smColorMode[VisualizationType::nVisualizationTypes() - 1];
    QGridLayout*      m_loDiffuseColorModes[VisualizationType::nVisualizationTypes() - 1];
    QButtonGroup*     m_ColorModeGroups[VisualizationType::nVisualizationTypes() - 1];

    QCheckBox*   m_chkOverrideDiffuseColor;
    QPushButton* m_btnRndAllColor;
    ColorPicker* m_pkrColor[VisualizationType::nVisualizationTypes() - 1];
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    void setupOutputControllers();
    QCheckBox*        m_chkEnableOutput;
    BrowsePathWidget* m_OutputPath;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // frame controller
    void setupFrameControllers();
    EnhancedSlider* m_sldFrameStep;
    EnhancedSlider* m_sldFrameDelay;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    void setupInputControllers();
    BrowsePathWidget* m_InputPath;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // buttons
    void setupParticleVizButtons();
    void setupDataPlayerButtons();
    QPushButton* m_btnHideVisualization[VisualizationType::nVisualizationTypes()];
    QPushButton* m_btnPause;
    QPushButton* m_btnNextFrame;
    QPushButton* m_btnReset;
    QPushButton* m_btnReverse;
    QPushButton* m_btnRepeatPlay;
    ////////////////////////////////////////////////////////////////////////////////
};
