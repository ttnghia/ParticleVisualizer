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
    void updateVisualizationParameters(const QString& sceneFile);
    void setParticleDiffuseColorMode(int colorMode);
    void setMaterial(const MaterialData& materialData);
    void browseOutputPath();

    void browseInputPath();
    void increaseDelay();
    void decreaseDelay();

public slots:
    void setInputPath(const QString& dataPath);
    void setParticleRadius(float radius);

protected:
    void connectWidgets();

    ////////////////////////////////////////////////////////////////////////////////
    RenderWidget* m_RenderWidget = nullptr;
    DataReader*   m_DataReader   = nullptr;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    void setupColorControllers();
    MaterialSelector* m_msMaterial;
    QSignalMapper*    m_smColorMode;
    ColorPicker*      m_pkrColor;
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
    void setupParticleRadiusControllers();
    QCheckBox* m_chkOverrideParticleRadius;
    QLineEdit* m_txtParticleRadius;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // buttons
    void setupDataPlayerButtons();
    QPushButton* m_btnPause;
    QPushButton* m_btnNextFrame;
    QPushButton* m_btnReset;
    QPushButton* m_btnReverse;
    QPushButton* m_btnRepeatPlay;
    ////////////////////////////////////////////////////////////////////////////////
};
