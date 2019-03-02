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
#include <LibQtApps/Forward.h>
#include <LibQtApps/OpenGLMainWindow.h>
#include "Forward.h"

using namespace  NTCodeBase;
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class MainWindow : public OpenGLMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = 0);
    virtual ~MainWindow();

protected:
    virtual bool processKeyPressEvent(QKeyEvent* event) override;
    virtual void showEvent(QShowEvent* ev);

public slots:
    void updateWindowTitle(const QString& filePath);
    void updateStatusCurrentFrame(int currentFrame);
    void updateStatusNumParticles();
    void updateNumFrames(int startFrame, int numFrames);
    void updateStatusReadInfo(double readTime, unsigned int bytes);
    void updateStatusMemoryUsage();

private:
    void setupRenderWidgets();
    void setupStatusBar();
    void connectWidgets();
    void setupPlayList();
    ////////////////////////////////////////////////////////////////////////////////
    RenderWidget* m_RenderWidget = nullptr;
    Controller*   m_Controller   = nullptr;
    QSlider*      m_sldFrame     = nullptr;

    QLabel* m_lblStatusCurrentFrame     = nullptr;
    QLabel* m_lblStatusNumFrames        = nullptr;
    QLabel* m_lblStatusNumVizPrimitives = nullptr;
    QLabel* m_lblStatusReadInfo         = nullptr;
    QLabel* m_lblStatusMemoryUsage      = nullptr;

    DataReader* m_DataReader = nullptr;
    DataList*   m_DataList   = nullptr;
};
