#-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
#-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
# ______          _   _      _        _   _ _                 _ _
# | ___ \        | | (_)    | |      | | | (_)               | (_)
# | |_/ /_ _ _ __| |_ _  ___| | ___  | | | |_ ___ _   _  __ _| |_ _______ _ __
# |  __/ _` | '__| __| |/ __| |/ _ \ | | | | / __| | | |/ _` | | |_  / _ \ '__|
# | | | (_| | |  | |_| | (__| |  __/ \ \_/ / \__ \ |_| | (_| | | |/ /  __/ |
# \_|  \__,_|_|   \__|_|\___|_|\___|  \___/|_|___/\__,_|\__,_|_|_/___\___|_|
#
# Created: Nov. 2018 by NT (https://ttnghia.github.io). All rights reserved.
#-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
#-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

QT += core gui widgets concurrent

TARGET = ParticleVisualizer
TEMPLATE = app

include($$PWD/../Externals/LibCommon/LibCommon.pri)
INCLUDEPATH += $$PWD/../Externals/LibCommon
INCLUDEPATH += $$PWD/../Externals/LibOpenGL
INCLUDEPATH += $$PWD/../Externals/LibQtApps
INCLUDEPATH += $$PWD/../Externals/LibParticle
INCLUDEPATH += $$PWD/../Externals/Simulation
INCLUDEPATH += $$PWD/Source

HEADERS += $$files(Source/*.h, true)
SOURCES += $$files(Source/*.cpp, true)

DISTFILES += $$files(*.txt, true)
DISTFILES += $$files(*.ini, true)
DISTFILES += $$files(Shaders/*.glsl, true)
RESOURCES += Shader.qrc

#-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

win32 {
    CONFIG(debug, debug|release) {
        PRE_TARGETDEPS += $$PWD/../Build/Debug/LibCommon.lib
        PRE_TARGETDEPS += $$PWD/../Build/Debug/LibOpenGL.lib
        PRE_TARGETDEPS += $$PWD/../Build/Debug/LibQtApps.lib
        PRE_TARGETDEPS += $$PWD/../Build/Debug/LibParticle.lib

        LIBS += $$PWD/../Build/Debug/LibCommon.lib
        LIBS += $$PWD/../Build/Debug/LibOpenGL.lib
        LIBS += $$PWD/../Build/Debug/LibQtApps.lib
        LIBS += $$PWD/../Build/Debug/LibParticle.lib
    } else {
        static {
            PRE_TARGETDEPS += $$PWD/../Build/ReleaseStaticBuild/LibCommon.lib
            PRE_TARGETDEPS += $$PWD/../Build/ReleaseStaticBuild/LibOpenGL.lib
            PRE_TARGETDEPS += $$PWD/../Build/ReleaseStaticBuild/LibQtApps.lib
            PRE_TARGETDEPS += $$PWD/../Build/ReleaseStaticBuild/LibParticle.lib

            LIBS += $$PWD/../Build/ReleaseStaticBuild/LibCommon.lib
            LIBS += $$PWD/../Build/ReleaseStaticBuild/LibOpenGL.lib
            LIBS += $$PWD/../Build/ReleaseStaticBuild/LibQtApps.lib
            LIBS += $$PWD/../Build/ReleaseStaticBuild/LibParticle.lib
        } else {
            PRE_TARGETDEPS += $$PWD/../Build/Release/LibCommon.lib
            PRE_TARGETDEPS += $$PWD/../Build/Release/LibOpenGL.lib
            PRE_TARGETDEPS += $$PWD/../Build/Release/LibQtApps.lib
            PRE_TARGETDEPS += $$PWD/../Build/Release/LibParticle.lib

            LIBS += $$PWD/../Build/Release/LibCommon.lib
            LIBS += $$PWD/../Build/Release/LibOpenGL.lib
            LIBS += $$PWD/../Build/Release/LibQtApps.lib
            LIBS += $$PWD/../Build/Release/LibParticle.lib
        }
    }
}

macx|unix {
    QMAKE_LFLAGS += -Wl,--start-group
    LIBS += -lLibCommon -lLibOpenGL -lLibQtApps -lLibParticle -lstdc++fs

    CONFIG(debug, debug|release) {
        LIBS += -L$$PWD/../Build/Debug
        PRE_TARGETDEPS += $$PWD/../Build/Debug/libLibCommon.a
        PRE_TARGETDEPS += $$PWD/../Build/Debug/libLibOpenGL.a
        PRE_TARGETDEPS += $$PWD/../Build/Debug/libLibQtApps.a
        PRE_TARGETDEPS += $$PWD/../Build/Debug/libLibParticle.a
    } else {
        LIBS += -L$$PWD/../Build/Release
        PRE_TARGETDEPS += $$PWD/../Build/Release/libLibCommon.a
        PRE_TARGETDEPS += $$PWD/../Build/Release/libLibOpenGL.a
        PRE_TARGETDEPS += $$PWD/../Build/Release/libLibQtApps.a
        PRE_TARGETDEPS += $$PWD/../Build/Release/libLibParticle.a
    }
}

