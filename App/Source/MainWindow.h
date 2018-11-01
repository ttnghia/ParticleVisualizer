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
#include <LibQtApps/Forward.h>
#include <LibQtApps/OpenGLMainWindow.h>
#include "Forward.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class MainWindow : public OpenGLMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = 0);

protected:
    virtual bool processKeyPressEvent(QKeyEvent* event) override;
    virtual void showEvent(QShowEvent* ev);

public slots:
    void updateWindowTitle(const QString& filePath);
    void updateStatusCurrentFrame(int currentFrame);
    void updateStatusNumVizPrimitives();
    void updateNumFrames(int numFrames);
    void updateStatusReadInfo(double readTime, size_t bytes);
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
