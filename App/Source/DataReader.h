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

#include <QObject>
#include <QFileSystemWatcher>
#include <QTimer>

#include <LibCommon/CommonSetup.h>
#include <LibQtApps/Forward.h>

#include "Forward.h"
#include "VisualizationData.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class DataReader : public QObject {
    Q_OBJECT
public:
    DataReader(const SharedPtr<VisualizationData>& vizData);
    void setInputPath(const QString& dataPath);
    void refresh() { setInputPath(m_CurrentDataPath); }

signals:
    void inputPathAccepted(const QString& dataPath);
private slots:
    void countFrames();
private:
    void resetData();
    bool checkDataFolder(const QString& dataPath);
    ////////////////////////////////////////////////////////////////////////////////
    bool                             m_bValidDataPath = false;
    QFileSystemWatcher*              m_DataDirWatcher = new QFileSystemWatcher;
    SharedPtr<ParticleSerialization> m_DataReader[VisualizationType::nParticleTypes()];
    bool                             m_DataAvailability[VisualizationType::nParticleTypes()];

    SharedPtr<VisualizationData> m_VizData;
    QString                      m_CurrentDataPath;
    QStringList                  m_WatchingPaths;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // frames
signals:
    void numFramesChanged(int numFrames);
    void currentFrameChanged(int currentFrame);
    void frameReadInfoChanged(double readTime, size_t bytes);

public slots:
    void setFrameDelayTime(int delayTime) { m_AutoTimer->setInterval(delayTime); }
    void setFrameStep(int frameStride) { m_FrameStep = frameStride; }
    void enableRepeat(bool bRepeat) { m_bRepeat = bRepeat; }
    void enableReverse(bool bReverse) { m_bReverse = bReverse; }
    void pause(bool bPaused) { m_bPause = bPaused; }
    void readFirstFrame() { readFrame(0); }
    void readLastFrame() { readFrame(m_nFrames - 1); }
    void readNextFrame(bool bBackward = false);
    void readPrevFrame() { readNextFrame(true); }
    void readFrame(int frame);
private:
    void readNextFrameByTimer() { if(!m_bPause) { readNextFrame(); } }
    ////////////////////////////////////////////////////////////////////////////////
    int  m_nFrames      = 0;
    int  m_CurrentFrame = 0;
    int  m_FrameStep    = 1;
    bool m_bPause       = false;
    bool m_bReverse     = false;
    bool m_bRepeat      = false;

    SharedPtr<AvgTimer> m_ReadTimer;
    SharedPtr<QTimer>   m_AutoTimer = nullptr;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // viz primitives
signals:
    void systemDimensionChanged();
    void numVizPrimitivesChanged();
    void vizDataChanged(int currentFrame);
public slots:
    void setParticleColorMode(int colorMode) { m_ColorMode = colorMode; }
private:
    std::pair<bool, size_t> readFrameData(int frameID);
    QStringList m_DataList;
    int         m_ColorMode = RenderColorMode::Ramp;

    static inline const StdVT<String> s_lstAttrPosition       = { "dimension", "particle_radius", "particle_position" };
    static inline const StdVT<String> s_lstAttrPositionObjIdx = { "dimension", "particle_radius", "particle_position", "object_index", "NObjects" };
    ////////////////////////////////////////////////////////////////////////////////
};
