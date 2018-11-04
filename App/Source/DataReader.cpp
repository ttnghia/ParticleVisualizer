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

#include <LibCommon/Geometry/MeshLoader.h>
#include <LibCommon/Geometry/GeometryObjects.h>
#include <LibCommon/Utils/JSONHelpers.h>

#include <LibQtApps/AvgTimer.h>
#include <LibOpenGL/LightAndMaterialData.h>

#include <LibParticle/ParticleHelpers.h>

#include <fstream>
#include <QDir>
#include <QDebug>
#include <QtConcurrent>

#include "DataReader.h"
#include "VisualizationData.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
DataReader::DataReader(const SharedPtr<VisualizationData>& vizData) : m_VizData(vizData),
    m_ReadTimer(std::make_shared<AvgTimer>()), m_AutoTimer(std::make_shared<QTimer>(this)) {
    __NT_REQUIRE(vizData != nullptr);
    ////////////////////////////////////////////////////////////////////////////////
    connect(m_DataDirWatcher,  &QFileSystemWatcher::directoryChanged, this, &DataReader::countFrames);
    connect(m_AutoTimer.get(), &QTimer::timeout,                      this, &DataReader::readNextFrameByTimer);
    m_AutoTimer->start(0);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void DataReader::setSequenceFile(const QString& sampleFileName) {
    if(m_ReadFrameFutureObj.isRunning()) {
        m_ReadFrameFutureObj.waitForFinished();
    }
    if(analyzeSequence(sampleFileName)) {
        resetData();
        auto dataFolder = QFileInfo(sampleFileName).dir().absolutePath();
        m_WatchingPath = dataFolder;
        m_DataDirWatcher->addPath(dataFolder);
        m_SampleFileName = sampleFileName;
        m_bValidDataPath = true;
        countFrames();
        m_CurrentFrame = m_StartFrame - 1;
        emit inputSequenceAccepted(dataFolder);
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void DataReader::resetData() {
    m_bValidDataPath = false;
    m_VizData->resetData();
    ////////////////////////////////////////////////////////////////////////////////
    m_CurrentFrame = -1;
    m_nFrames      = 0;
    m_bPause       = false;
    emit numFramesChanged(0);
    ////////////////////////////////////////////////////////////////////////////////
    if(m_WatchingPath != "") {
        m_DataDirWatcher->removePath(m_WatchingPath);
        m_WatchingPath = "";
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
bool DataReader::analyzeSequence(const QString& sampleFileName) {
    auto file = QFileInfo(sampleFileName);
    if(!file.exists()) {
        return false;
    }
    auto prefix    = file.dir().absolutePath().replace("\\\\", "/") + QString("/") + file.baseName();
    auto extension = file.suffix();
    ////////////////////////////////////////////////////////////////////////////////
    if(extension.toUpper() == "BIN") {
        m_FileExtension    = FileExtensions::BIN;
        m_FileExtensionStr = QString(".bin");
    } else if(extension.toUpper() == "POS") {
        m_FileExtension    = FileExtensions::BIN;
        m_FileExtensionStr = QString(".pos");
    } else if(extension.toUpper() == "BGEO") {
        m_FileExtension    = FileExtensions::BGEO;
        m_FileExtensionStr = QString(".bgeo");
    } else if(extension.toUpper() == "OBJ") {
        m_FileExtension    = FileExtensions::OBJ;
        m_FileExtensionStr = QString(".obj");
    } else {
        return false; // wrong file extension
    }
    ////////////////////////////////////////////////////////////////////////////////
    auto enumerateWidth = file.completeSuffix().indexOf('.');
    if(enumerateWidth < 1) {
        return false;
    }
    if(enumerateWidth == 4) {
        m_EnumerateType = EnumerateTypes::Width4_0Prefix;
    } else if(enumerateWidth == 3) {
        m_EnumerateType = EnumerateTypes::Width3_0Prefix;
    } else {
        m_EnumerateType = EnumerateTypes::NoPrefix;
    }
    ////////////////////////////////////////////////////////////////////////////////
    m_DataSequencePrefix = prefix + QString(".");
    return true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
QString DataReader::getFilePath(int idx) {
    auto getFrameEnumerate = [&, idx] {
                                 if(m_EnumerateType == EnumerateTypes::NoPrefix) {
                                     return QString("%1").arg(idx);
                                 } else if(m_EnumerateType == EnumerateTypes::Width4_0Prefix) {
                                     return QString("%1").arg(idx, 4, 10, QChar('0'));
                                 } else {
                                     return QString("%1").arg(idx, 3, 10, QChar('0'));
                                 }
                             };
    return m_DataSequencePrefix + getFrameEnumerate() + m_FileExtensionStr;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void DataReader::countFrames() {
    QDir dataDir(m_WatchingPath);
    dataDir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
    dataDir.setNameFilters(QStringList() << ("*" + m_FileExtensionStr));
    UInt nFrames = dataDir.entryList().count();
    ////////////////////////////////////////////////////////////////////////////////
    m_StartFrame = 0;
    try {
        while(true) {
            QString file = getFilePath(m_StartFrame);
            if(!QFileInfo::exists(file)) {
                ++m_StartFrame;
            } else {
                break;
            }
        }
    } catch(std::exception& e) {
        std::cerr << "Exception during couting files: " << e.what() << std::endl;
    }
    if(nFrames != m_nFrames) {
        m_nFrames = nFrames;
        emit numFramesChanged(m_nFrames);
    }
    m_EndFrame = m_StartFrame + m_nFrames - 1;
    if(m_CurrentFrame > m_EndFrame) {
        m_CurrentFrame = m_EndFrame - 1;
        readNextFrame();
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void DataReader::readNextFrame(bool bBackward /*= false*/) {
    if(m_VizData->bCaptureImage && !m_VizData->bCaptureImageSaved) {
        return;
    }
    if(m_ReadFrameFutureObj.isRunning()) {
        return;
    }
    int nextFrame = (m_bReverse ^ bBackward) ? m_CurrentFrame - m_FrameStep : m_CurrentFrame + m_FrameStep;
    if(!m_bRepeat && (nextFrame < m_StartFrame || nextFrame > m_EndFrame)) {
        return;
    }
    if(nextFrame < m_StartFrame) {
        nextFrame = m_EndFrame;
    } else if(nextFrame > m_EndFrame) {
        nextFrame = m_StartFrame;
    }
    m_ReadFrameFutureObj = QtConcurrent::run([&, nextFrame] { readFrame(nextFrame); });
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void DataReader::readFrame(int frame) {
    if(!m_bValidDataPath) {
        return;
    }
    ////////////////////////////////////////////////////////////////////////////////
    m_ReadTimer->tick();
    auto [bSuccess, bytesReads] = readFrameData(frame);
    m_ReadTimer->tock();
    if(bSuccess) {
        m_CurrentFrame = frame;
        m_VizData->bVizDataUploaded   = false;
        m_VizData->currentFrame       = frame;
        m_VizData->bCaptureImageSaved = false;
        emit currentFrameChanged(m_CurrentFrame);
        emit vizDataChanged(m_CurrentFrame);
        emit frameReadInfoChanged(m_ReadTimer->getAvgTime(), bytesReads);
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
std::pair<bool, UInt> DataReader::readFrameData(int frameID) {
    bool   bSuccess   = false;
    UInt   nBytesRead = 0;
    UInt   nParticles = 0;
    String file       = getFilePath(frameID).toStdString();
    ////////////////////////////////////////////////////////////////////////////////
    auto readParticles = [&](const auto& fileName, auto& buffer, auto& bSuccess, auto& nBytesRead) {
                             buffer.resize(0);
                             if(m_FileExtension == FileExtensions::BGEO) {
                                 bSuccess = ParticleHelpers::loadParticlesFromBGEO(fileName, buffer, m_VizData->particleRadius);
                             } else if(m_FileExtension == FileExtensions::BIN) {
                                 bSuccess = ParticleHelpers::loadParticlesFromBinary(fileName, buffer, m_VizData->particleRadius);
                             } else {
                                 bSuccess = ParticleHelpers::loadParticlesFromObj(fileName, buffer);
                             }
                             nParticles = static_cast<UInt>(buffer.size());
                             nBytesRead = static_cast<UInt>(buffer.size() * sizeof(buffer.front()));
                             return nParticles;
                         };
    if(m_VizData->systemDimension == 3) {
        readParticles(file, m_VizData->buffPositions3D, bSuccess, nBytesRead);
        m_VizData->particlePositionPtrs = reinterpret_cast<char*>(m_VizData->buffPositions3D.data());
    } else {
        readParticles(file, m_VizData->buffPositions2D, bSuccess, nBytesRead);
        m_VizData->particlePositionPtrs = reinterpret_cast<char*>(m_VizData->buffPositions2D.data());
    }
    if(!m_VizData->bRadiusOverrided && m_VizData->particleRadius < float(1.0e-10)) {
        m_VizData->computeParticleRadius();
        particleRadiusChanged(m_VizData->particleRadius);
    }
    ////////////////////////////////////////////////////////////////////////////////
    if(nParticles != m_VizData->nParticles) {
        m_VizData->nParticles = nParticles;
        emit numVizPrimitivesChanged();
    }

    if(!bSuccess) {
        return { false, 0 };
    } else {
        return { true, nBytesRead };
    }
}
