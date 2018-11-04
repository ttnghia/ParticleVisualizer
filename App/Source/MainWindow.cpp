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

#include <QMouseEvent>
#include <LibCommon/Utils/Formatters.h>
#include <LibCommon/Utils/MemoryUsage.h>
#include <LibQtApps/DataList.h>

#include "RenderWidget.h"
#include "DataReader.h"
#include "Controller.h"
#include "MainWindow.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
MainWindow::MainWindow(QWidget* parent) : OpenGLMainWindow(parent), m_DataList(new DataList(nullptr, true, true)) {
    m_RenderWidget = new RenderWidget(this);
    m_DataReader   = new DataReader(m_RenderWidget->getVizData());
    setupOpenglWidget(m_RenderWidget);
    ////////////////////////////////////////////////////////////////////////////////
    setupRenderWidgets();
    setupStatusBar();
    connectWidgets();
    setupPlayList();
    ////////////////////////////////////////////////////////////////////////////////
    setArthurStyle();
    setFocusPolicy(Qt::StrongFocus);
    setWindowTitle("Particle Visualizer");
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
bool MainWindow::processKeyPressEvent(QKeyEvent* event) {
    switch(event->key()) {
        case Qt::Key_B:
            m_Controller->browseInputPath();
            return true;

        case Qt::Key_O:
            m_Controller->browseOutputPath();
            return true;

        case Qt::Key_R:
            m_Controller->m_btnReverse->click();
            return true;

        case Qt::Key_Space:
            m_Controller->m_btnPause->click();
            return true;

        case Qt::Key_N:
            m_DataReader->readNextFrame();
            return true;

        case Qt::Key_BracketLeft:
            m_DataReader->readPrevFrame();
            return true;

        case Qt::Key_BracketRight:
            m_DataReader->readNextFrame();
            return true;

        case Qt::Key_F1:
            m_DataReader->readFirstFrame();
            return true;

        case Qt::Key_Home:
            m_DataReader->readFirstFrame();
            return true;

        case Qt::Key_End:
            m_DataReader->readLastFrame();
            return true;

        case Qt::Key_F5:
            m_DataReader->refresh();
            return true;

        case Qt::Key_S:
            m_Controller->increaseDelay();
            return true;

        case Qt::Key_F:
            m_Controller->decreaseDelay();
            return true;

        default:
            return OpenGLMainWindow::processKeyPressEvent(event);
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void MainWindow::showEvent(QShowEvent* ev) {
    QMainWindow::showEvent(ev);
    ////////////////////////////////////////////////////////////////////////////////
    static bool showed = false;
    if(!showed) {
        updateStatusMemoryUsage();
        ////////////////////////////////////////////////////////////////////////////////
        if(m_DataList->getListSize() > 0) {
            QTimer::singleShot(100, this, [&]() {
                                   m_DataList->show();
                                   m_DataList->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignBottom | Qt::AlignRight,
                                                                               m_DataList->size(), qApp->desktop()->availableGeometry()));
                               });
        }
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void MainWindow::updateWindowTitle(const QString& filePath) {
    setWindowTitle(QString("Particle Visualizer: ") + filePath);
}

void MainWindow::updateStatusCurrentFrame(int currentFrame) {
    m_lblStatusCurrentFrame->setText(QString("Current frame: %1").arg(currentFrame));
}

void MainWindow::updateStatusNumParticles() {
    String status = String("Num. particles: ") + Formatters::toString(m_RenderWidget->getVizData()->nParticles);
    m_lblStatusNumVizPrimitives->setText(QString::fromStdString(status));
}

void MainWindow::updateNumFrames(int numFrames) {
    m_sldFrame->setRange(0, numFrames - 1);
    m_lblStatusNumFrames->setText(QString("Total frames: %1").arg(numFrames));
}

void MainWindow::updateStatusReadInfo(double readTime, unsigned int bytes) {
    m_lblStatusReadInfo->setText(QString("Load data: %1 (ms) | %2 (MBs)").arg(readTime).arg(static_cast<double>(bytes) / 1048576.0));
}

void MainWindow::updateStatusMemoryUsage() {
    m_lblStatusMemoryUsage->setText(QString("Memory usage: %1 (MBs)").arg(QString::fromStdString(Formatters::toString(getCurrentRSS() / 1048576.0))));
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void MainWindow::setupRenderWidgets() {
    ////////////////////////////////////////////////////////////////////////////////
    m_sldFrame = new QSlider(Qt::Horizontal);
    m_sldFrame->setRange(1, 1);
    m_sldFrame->setValue(1);
    m_sldFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    ////////////////////////////////////////////////////////////////////////////////
    QVBoxLayout* loCenter = new QVBoxLayout;
    loCenter->addWidget(m_GLWidget, 1);
    loCenter->addWidget(m_sldFrame);
    ////////////////////////////////////////////////////////////////////////////////
    m_Controller = new Controller(m_RenderWidget, m_DataReader, this);
    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addLayout(loCenter);
    mainLayout->addWidget(m_Controller);
    ////////////////////////////////////////////////////////////////////////////////
    QWidget* mainWidget = new QWidget(this);
    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void MainWindow::setupStatusBar() {
    m_lblStatusCurrentFrame = new QLabel(this);
    m_lblStatusCurrentFrame->setMargin(5);
    statusBar()->addPermanentWidget(m_lblStatusCurrentFrame, 1);
    ////////////////////////////////////////////////////////////////////////////////
    m_lblStatusNumFrames = new QLabel(this);
    m_lblStatusNumFrames->setMargin(5);
    statusBar()->addPermanentWidget(m_lblStatusNumFrames, 1);
    ////////////////////////////////////////////////////////////////////////////////
    m_lblStatusNumVizPrimitives = new QLabel(this);
    m_lblStatusNumVizPrimitives->setMargin(5);
    statusBar()->addPermanentWidget(m_lblStatusNumVizPrimitives, 1);
    ////////////////////////////////////////////////////////////////////////////////
    m_lblStatusReadInfo = new QLabel(this);
    m_lblStatusReadInfo->setMargin(5);
    statusBar()->addPermanentWidget(m_lblStatusReadInfo, 1);
    ////////////////////////////////////////////////////////////////////////////////
    m_lblStatusMemoryUsage = new QLabel(this);
    m_lblStatusMemoryUsage->setMargin(5);
    statusBar()->addPermanentWidget(m_lblStatusMemoryUsage, 1);
    ////////////////////////////////////////////////////////////////////////////////
    QTimer* memTimer = new QTimer(this);
    connect(memTimer, &QTimer::timeout, [&] { this->updateStatusMemoryUsage(); });
    memTimer->start(5000);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void MainWindow::connectWidgets() {
    connect(m_sldFrame,   &QSlider::sliderMoved,            m_DataReader, &DataReader::readFrame);
    ////////////////////////////////////////////////////////////////////////////////
    connect(m_DataReader, &DataReader::currentFrameChanged, [&](int frame) {
                m_sldFrame->blockSignals(true);
                m_sldFrame->setValue(frame);
                m_sldFrame->blockSignals(false);
            });
    connect(m_DataReader, &DataReader::currentFrameChanged,     this,           &MainWindow::updateStatusCurrentFrame);
    connect(m_DataReader, &DataReader::numFramesChanged,        this,           &MainWindow::updateNumFrames);
    connect(m_DataReader, &DataReader::frameReadInfoChanged,    this,           &MainWindow::updateStatusReadInfo);
    connect(m_DataReader, &DataReader::numVizPrimitivesChanged, this,           &MainWindow::updateStatusNumParticles);
    connect(m_DataReader, &DataReader::inputSequenceAccepted,   this,           &MainWindow::updateWindowTitle);
    connect(m_DataReader, &DataReader::vizDataChanged,          m_RenderWidget, &RenderWidget::updateVizData);
    ////////////////////////////////////////////////////////////////////////////////
    connect(m_DataReader, &DataReader::particleRadiusChanged,   [&](float radius) {
                m_Controller->blockSignals(true);
                m_Controller->setParticleRadius(radius);
                m_Controller->blockSignals(false);
            });
    connect(m_DataList, &DataList::currentTextChanged, m_Controller, &Controller::setInputPath);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void MainWindow::setupPlayList() {
    assert(m_DataList != nullptr);
    m_DataList->setWindowTitle("Data List");
    ////////////////////////////////////////////////////////////////////////////////
    const QString listFile(QDir::currentPath() + "/PlayList.txt");
    if(QFile::exists(listFile)) {
        m_DataList->loadListFromFile(listFile);
    } else {
        qDebug() << "PlayList.txt does not exist. No play list loaded";
    }
}
