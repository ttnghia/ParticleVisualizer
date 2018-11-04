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

#include <QObject>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QFuture>

#include <LibCommon/CommonSetup.h>
#include <LibQtApps/Forward.h>

#include "Forward.h"
#include "VisualizationData.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class DataReader : public QObject {
    Q_OBJECT
public:
    DataReader(const SharedPtr<VisualizationData>& vizData);
    ~DataReader() { stopDataReader(); }
    void setSequenceFile(const QString& sampleFileName);
    void refresh() { setSequenceFile(m_SampleFileName); }
    void computeParticleRadius();
    void stopDataReader();

signals:
    void inputSequenceAccepted(const QString& dataFolder);
private slots:
    void countFrames();
private:
    void    resetData();
    bool    analyzeSequence(const QString& sampleFileName);
    QString getFilePath(int idx);
    ////////////////////////////////////////////////////////////////////////////////
    enum class EnumerateTypes {
        NoPrefix,
        Width3_0Prefix,
        Width4_0Prefix
    };
    enum class FileExtensions { BIN, BGEO, OBJ };
    ////////////////////////////////////////////////////////////////////////////////
    EnumerateTypes      m_EnumerateType    = EnumerateTypes::NoPrefix;
    FileExtensions      m_FileExtension    = FileExtensions::BGEO;
    QString             m_FileExtensionStr = QString(".bgeo");
    bool                m_bValidDataPath   = false;
    QFileSystemWatcher* m_DataDirWatcher   = new QFileSystemWatcher;

    SharedPtr<VisualizationData> m_VizData;
    QString                      m_SampleFileName;
    QString                      m_DataSequencePrefix;
    QString                      m_WatchingPath;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // frames
signals:
    void particleRadiusChanged(float radius);
    void numFramesChanged(int numFrames);
    void currentFrameChanged(int currentFrame);
    void frameReadInfoChanged(double readTime, unsigned int bytes);

public slots:
    void setFrameDelayTime(int delayTime) { m_AutoTimer->setInterval(delayTime); }
    void setFrameStep(int frameStride) { m_FrameStep = frameStride; }
    void enableRepeat(bool bRepeat) { m_bRepeat = bRepeat; }
    void enableReverse(bool bReverse) { m_bReverse = bReverse; }
    void pause(bool bPaused) { m_bPause = bPaused; }
    void readFirstFrame() { readFrame(m_StartFrame); }
    void readLastFrame() { readFrame(m_EndFrame); }
    void readNextFrame(bool bBackward = false);
    void readPrevFrame() { readNextFrame(true); }
    void readFrame(int frame);
private:
    void readNextFrameByTimer() { if(!m_bPause) { readNextFrame(); } }
    ////////////////////////////////////////////////////////////////////////////////
    int  m_nFrames      = 0;
    int  m_CurrentFrame = 0;
    int  m_StartFrame   = 0;
    int  m_EndFrame     = 0;
    int  m_FrameStep    = 1;
    bool m_bPause       = false;
    bool m_bReverse     = false;
    bool m_bRepeat      = false;

    SharedPtr<AvgTimer> m_ReadTimer;
    SharedPtr<QTimer>   m_AutoTimer = nullptr;
    QFuture<void>       m_ReadFrameFutureObj;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // viz primitives
signals:
    void numVizPrimitivesChanged();
    void vizDataChanged(int currentFrame);
private:
    std::pair<bool, UInt> readFrameData(int frameID);
};
