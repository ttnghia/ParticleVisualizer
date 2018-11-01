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

#include <LibParticle/ParticleSerialization.h>
#include <LibQtApps/AvgTimer.h>
#include <LibOpenGL/LightAndMaterialData.h>

#include <LibCommon/Geometry/MeshLoader.h>
#include <LibCommon/Geometry/GeometryObjects.h>
#include <LibCommon/Utils/JSONHelpers.h>

#include <fstream>
#include <QDir>

#include "DataReader.h"
#include "VisualizationData.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
DataReader::DataReader(const SharedPtr<VisualizationData>& vizData) : m_VizData(vizData),
    m_ReadTimer(std::make_shared<AvgTimer>()), m_AutoTimer(std::make_shared<QTimer>(this)) {
    __NT_REQUIRE(vizData != nullptr);
    for(int vizType = 0; vizType < VisualizationType::nParticleTypes(); ++vizType) {
        m_DataReader[vizType] = std::make_shared<ParticleSerialization>();
    }
    ////////////////////////////////////////////////////////////////////////////////
    connect(m_DataDirWatcher,  &QFileSystemWatcher::directoryChanged, this, &DataReader::countFrames);
    connect(m_AutoTimer.get(), &QTimer::timeout,                      this, &DataReader::readNextFrameByTimer);
    m_AutoTimer->start(0);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void DataReader::setInputPath(const QString& dataPath) {
    if(checkDataFolder(dataPath)) {
        resetData();
        for(int vizType = 0; vizType < VisualizationType::nParticleTypes(); ++vizType) {
            auto vizTypeFolder = dataPath.toStdString() + String("/FrameOutput/") + VizNames[vizType];
            if(FileHelpers::fileExisted(vizTypeFolder)) {
                m_DataReader[vizType]->setDataPath(dataPath.toStdString(), String("FrameOutput/") + VizNames[vizType], "frame");
                m_DataAvailability[vizType] = true;
                m_DataDirWatcher->addPath(QString::fromStdString(vizTypeFolder));
                m_WatchingPaths.append(QString::fromStdString(vizTypeFolder));
            }
        }
        m_CurrentDataPath = dataPath;
        m_DataDirWatcher->addPath(dataPath);
        m_bValidDataPath = true;
        countFrames();
        emit inputPathAccepted(dataPath);
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
bool DataReader::checkDataFolder(const QString& dataPath) {
    QDir dataDir(dataPath);
    dataDir.setNameFilters(QStringList() << "*.json");
    if(dataDir.entryList().count() == 0) {
        return false;
    }
    return true;
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
    for(int vizType = 0; vizType < VisualizationType::nParticleTypes(); ++vizType) {
        m_DataAvailability[vizType] = false;
    }
    if(m_WatchingPaths.size() > 0) {
        m_DataDirWatcher->removePaths(m_WatchingPaths);
        m_WatchingPaths.clear();
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void DataReader::countFrames() {
    m_nFrames = 0;
    for(int vizType = 0; vizType < VisualizationType::nParticleTypes(); ++vizType) {
        auto vizTypeFolder = m_CurrentDataPath.toStdString() + String("/FrameOutput/") + VizNames[vizType];
        if(m_DataAvailability[vizType]) {
            try {
                auto frames = static_cast<int>(FileHelpers::countFiles(vizTypeFolder));
                if(m_nFrames < frames) {
                    m_nFrames = frames;
                }
            } catch(std::exception& e) {
                std::cerr << "Exception during couting files: " << e.what() << std::endl;
            }
        }
    }
    emit numFramesChanged(m_nFrames);
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
    size_t bytesRead         = 0;
    bool   bSuccess          = false;
    bool   bNParticleChanged = false;
    for(int vizType = 0; vizType < VisualizationType::nParticleTypes(); ++vizType) {
        if(!m_DataAvailability[vizType]) {
            continue;
        }
        if(m_ColorMode != RenderColorMode::ObjectIndex) {
            bSuccess |= m_DataReader[vizType]->read(frameID, s_lstAttrPosition);
        } else {
            bSuccess |= m_DataReader[vizType]->read(frameID, s_lstAttrPositionObjIdx, false);
        }
        bytesRead += m_DataReader[vizType]->getBytesRead();

        UInt nParticles = m_DataReader[vizType]->getNParticles();

        if(nParticles > 0) {
            int dimension;
            __NT_REQUIRE(m_DataReader[vizType]->getFixedAttribute("dimension", dimension));
            if(dimension == 2) {
                m_DataReader[vizType]->getParticleAttribute("particle_position", m_VizData->buffsPositions2D[vizType]);
                assert(m_VizData->buffsPositions2D.size() > 0);
                m_VizData->particlePositionPtrs[vizType] = reinterpret_cast<char*>(m_VizData->buffsPositions2D[vizType].data());
            } else {
                m_DataReader[vizType]->getParticleAttribute("particle_position", m_VizData->buffsPositions3D[vizType]);
                assert(m_VizData->buffsPositions3D.size() > 0);
                m_VizData->particlePositionPtrs[vizType] = reinterpret_cast<char*>(m_VizData->buffsPositions3D[vizType].data());
            }

            if(m_ColorMode == RenderColorMode::ObjectIndex) {
                m_DataReader[vizType]->getParticleAttribute("object_index", m_VizData->buffsObjectIndex[vizType]);
                m_VizData->objectIndexPtrs[vizType] = reinterpret_cast<char*>(m_VizData->buffsObjectIndex[vizType].data());
                m_DataReader[vizType]->getFixedAttribute("NObjects", m_VizData->nObjects[vizType]);
            }
            ////////////////////////////////////////////////////////////////////////////////
            if(m_VizData->systemDimension != dimension) {
                m_VizData->systemDimension = dimension;
                emit systemDimensionChanged();
            }
            __NT_REQUIRE(m_DataReader[vizType]->getFixedAttribute("particle_radius", m_VizData->particleRadius));
        }
        if(nParticles != m_VizData->nParticles[vizType]) {
            m_VizData->nParticles[vizType] = nParticles;
            bNParticleChanged = true;
        }
    }
    if(!bSuccess) {
        return { false, 0 };
    }
    if(bNParticleChanged) {
        emit numVizPrimitivesChanged();
    }
    ////////////////////////////////////////////////////////////////////////////////
    return { true, bytesRead };
}
