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
    if(analyzeSequence(sampleFileName)) {
        resetData();
        auto dataFolder = QFileInfo(sampleFileName).dir();
        m_DataDirWatcher->addPath(dataFolder.absolutePath());
        m_bValidDataPath = true;
        countFrames();
        emit inputSequenceAccepted(sampleFileName);
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
    auto file      = QFileInfo(sampleFileName);
    auto prefix    = file.dir().absolutePath().replace("\\\\", "/") + QString("/") + file.baseName();
    auto extension = file.suffix();
    ////////////////////////////////////////////////////////////////////////////////
    if(extension.toUpper() == "BIN") {
        m_FileExtension    = FileExtensions::BIN;
        m_FileExtensionStr = QString(".bin");
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
    UInt nFrames = 0;
    try {
        while(true) {
            QString file = getFilePath(nFrames);
            if(QFileInfo::exists(file) || nFrames == 0) {
                ++nFrames;
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
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void DataReader::readNextFrame(bool bBackward /*= false*/) {
    if(m_VizData->bCaptureImage && !m_VizData->bCaptureImageSaved) {
        return;
    }
    int nextFrame = (m_bReverse ^ bBackward) ? m_CurrentFrame - m_FrameStep : m_CurrentFrame + m_FrameStep;
    if(!m_bRepeat && (nextFrame < 0 || nextFrame >= m_nFrames)) {
        return;
    }
    if(nextFrame < 0) {
        nextFrame = m_nFrames - 1;
    } else if(nextFrame >= m_nFrames) {
        nextFrame = 0;
    }
    readFrame(nextFrame);
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
std::pair<bool, size_t> DataReader::readFrameData(int frameID) {
    bool   bSuccess   = false;
    size_t nBytesRead = 0;
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
                             nBytesRead = buffer.size() * sizeof(Vec3f);
                             return nParticles;
                         };
    if(m_VizData->systemDimension == 3) {
        readParticles(file, m_VizData->buffPositions3D, bSuccess, nBytesRead);
        m_VizData->particlePositionPtrs = reinterpret_cast<char*>(m_VizData->buffPositions3D.data());
        m_VizData->particleRadius       = computeParticleRadius(m_VizData->buffPositions3D);
    } else {
        readParticles(file, m_VizData->buffPositions2D, bSuccess, nBytesRead);
        m_VizData->particlePositionPtrs = reinterpret_cast<char*>(m_VizData->buffPositions2D.data());
        m_VizData->particleRadius       = computeParticleRadius(m_VizData->buffPositions2D);
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

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N>
float DataReader::computeParticleRadius(const StdVT<VecX<N, float>>& positions) {
    auto maxN = positions.size();
    if(maxN > 100) {
        maxN = 100;
    }
    float averageDistance = 0;
    for(size_t p = 0; p < maxN; ++p) {
        auto minDistanceSqr = float(1e10);
        for(size_t q = 0; q < maxN; ++q) {
            if(p == q) {
                continue;
            }
            auto tmp = glm::length2(positions[p] - positions[q]);
            if(minDistanceSqr > tmp) {
                minDistanceSqr = tmp;
            }
        }
        averageDistance += minDistanceSqr;
    }
    averageDistance = std::sqrt(averageDistance / static_cast<float>(maxN));
    return averageDistance * 0.5f;
}
