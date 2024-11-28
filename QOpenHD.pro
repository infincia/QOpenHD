BASEDIR = $$IN_PWD

LANGUAGE = C++
CONFIG += c++17
CONFIG+=sdk_no_version_check
CONFIG += resources_big

TRANSLATIONS = translations/QOpenHD_en.ts \
               translations/QOpenHD_de.ts \
               translations/QOpenHD_ru.ts \
               translations/QOpenHD_nl.ts \
               translations/QOpenHD_es.ts \
               translations/QOpenHD_fr.ts \
               translations/QOpenHD_zh.ts \
               translations/QOpenHD_it.ts \
               translations/QOpenHD_ro.ts


include(platforms.pri)

include(git.pri)
# this library can be included via either .pri or cmake
# since it is a library "specifically for qt"
include(lib/lqtutils_master/lqtutils.pri)

# No one knows what this block below does, from stephen most likely
CONFIG(debug, debug|release) {
    DESTDIR = $${OUT_PWD}/debug

    CONFIG += debug
    DEFINES += QMLJSDEBUGGER
} else:CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG
    CONFIG += installer
    CONFIG += force_debug_info
    DESTDIR = $${OUT_PWD}/release
    DEFINES += QMLJSDEBUGGER
}

#https://doc.qt.io/qt-6/portingguide.html
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050F00

# See https://www.qt.io/blog/qml-type-registration-in-qt-5.15
# avoid using qmlRegisterType<>, use macro instead
CONFIG += qmltypes
QML_IMPORT_NAME = OpenHD
QML_IMPORT_MAJOR_VERSION = 1

OBJECTS_DIR  = $${OUT_PWD}/obj
MOC_DIR      = $${OUT_PWD}/moc
UI_DIR       = $${OUT_PWD}/ui
RCC_DIR      = $${OUT_PWD}/rcc

#from https://mavlink.io/en/mavgen_c/
#You can supress the warnings in CMake using ...
#and https://stackoverflow.com/questions/2987062/configuring-the-gcc-compiler-switches-in-qt-qtcreator-and-qmake
#this can not be used in MSVC (windows)
#NOTE: While disabling warnings is often discouraged, in the scope of QOpenHD, those warnings distract more from actual issues than help
#However, if you feel so, re-enable and fix them ;)
LinuxBuild {
    QMAKE_CXXFLAGS += -Wno-address-of-packed-member
    QMAKE_CXXFLAGS += -Wno-cast-align
    QMAKE_CXXFLAGS += -Wno-unused-function
    QMAKE_CXXFLAGS += -Wno-unused-variable
    QMAKE_CXXFLAGS += -Wno-unused-parameter
    QMAKE_CXXFLAGS += -Wno-sign-compare
}

# These are the QT libraries we always need when building QOpenHD - they are intentially kept as small in number as possible
# (aka all these really should come with pretty much any qt install)
# In general, parts of QOpenHD that need additional libraries should have their code in a subdirectory with a .pri where those
# dependencies are added such that you can easily compile the project even on systems that might lack some of those qt functionalities
QT +=core quick qml gui \
    widgets
QT += opengl
QT += charts
#QT += gui-private
#LIBS += Ldrm

INCLUDEPATH += $$PWD/lib
INCLUDEPATH += $$PWD/app
INCLUDEPATH += $$PWD/app/exp

# QOpenHD telemetry (mavlink)
# REQUIRED - without it QOpenHD will compile, but be pretty much non functional
include(app/telemetry/telemetry.pri)

# This code is platform and library independent, and always compiled.
include(app/videostreaming/vscommon/vscommon.pri)

# Video is quite platform dependent, if you have compile issues, try compiling without any videostreaming enabled first ;)
LinuxBuild {
    # Avcodec decode and display, all sources
    # Replaced gstreamer for now
    # Comment the line below to build without avcodec (you won't have any video decoding capabilities in this case though)
    # However, this can be usefully for figuring out compiler issue(s) on different platform(s)
    # NOTE: QT Creator is quite bad at figuring out changes here, you might need a "full" rebuild or manualy delete
    # the build dir/cache, then rebuild
    include(app/videostreaming/avcodec/avcodec_video.pri)

    # Gstreamer / qmlglsink decode and display, all sources
    # r.n only used for secondary video and for primary video only on platforms we cannot do primary video via QSG / avcodec
    include(app/videostreaming/gstreamer/gst_video.pri)
}

# All Generic files / files that literally have 0!! dependencies other than qt
SOURCES += \
    app/adsb/adsbvehicle.cpp \
    app/adsb/adsbvehiclemanager.cpp \
    app/adsb/qmlobjectlistmodel.cpp \
    app/logging/hudlogmessagesmodel.cpp \
    app/logging/logmessagesmodel.cpp \
    app/util/mousehelper.cpp \
    app/util/qopenhd.cpp \
    app/util/WorkaroundMessageBox.cpp \
    app/util/qrenderstats.cpp \
    app/util/restartqopenhdmessagebox.cpp \
    app/main.cpp \

HEADERS += \
    app/adsb/adsbvehicle.h \
    app/adsb/adsbvehiclemanager.h \
    app/adsb/qmlobjectlistmodel.h \
    app/common/ThreadsafeQueue.hpp \
    app/common/util_fs.h \
    app/common/StringHelper.hpp \
    app/common/TimeHelper.hpp \
    app/logging/hudlogmessagesmodel.h \
    app/logging/loghelper.h \
    app/logging/logmessagesmodel.h \
    app/util/mousehelper.h \
    app/util/qopenhd.h \
    app/util/WorkaroundMessageBox.h \
    app/util/qrenderstats.h \
    app/util/restartqopenhdmessagebox.h \
    app/util/lqutils_include.h \


# Geographic lib updated to c-2.0, so much cleaner
SOURCES += $$PWD/lib/geographiclib-c-2.0/src/geodesic.c
HEADERS += $$PWD/lib/geographiclib-c-2.0/src/geodesic.h

# All files for the OSD elements - these are QT QQuickPaintedItem's that are written in c++
SOURCES += \
    app/osd/headingladder.cpp \
    app/osd/horizonladder.cpp \
    app/osd/speedladder.cpp \
    app/osd/altitudeladder.cpp \
    app/osd/flightpathvector.cpp \
    app/osd/aoagauge.cpp \

HEADERS += \
    app/osd/headingladder.h \
    app/osd/horizonladder.h \
    app/osd/speedladder.h \
    app/osd/altitudeladder.h \
    app/osd/flightpathvector.h \
    app/osd/debug_overdraw.hpp \
    app/osd/aoagauge.h \


RESOURCES += qml/qml.qrc


DISTFILES += \
    README.md \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/drawable-hdpi/splashscreen.png \
    android/res/drawable-ldpi/splashscreen.png \
    android/res/drawable-mdpi/splashscreen.png \
    android/res/drawable-xhdpi/splashscreen.png \
    android/res/drawable-xxhdpi/splashscreen.png \
    android/res/drawable-xxxhdpi/splashscreen.png \
    android/res/values/libs.xml \
    android/res/values/styles.xml \
    android/src/QOpenHDActivity.java \
    qml/qtquickcontrols2.conf \
    qml/ui/qmldir \


iOSBuild {
    QMAKE_INFO_PLIST    = ios/Info.plist
    QMAKE_TARGET_BUNDLE_PREFIX = com.infincia
    QMAKE_BUNDLE = qopenhd
    QMAKE_DEVELOPMENT_TEAM = G738Z89QKM
    ICON                = $${BASEDIR}/icons/macos.icns
    DISTFILES        += ios/Info.plist \
                        icons/LaunchScreen.png \
                        icons/LaunchScreen.storyboard
    LIBS += -framework VideoToolbox -framework AudioToolbox -framework CoreAudio -framework CoreVideo -framework CoreMedia
    CONFIG -= bitcode
    #CONFIG += EnableGamepads
    CONFIG += EnableSpeech
    CONFIG += EnableMainVideo
    #CONFIG += EnableLog //does not work due to filepath not set

    app_launch_images.files = $$PWD/icons/LaunchScreen.png $$files($$PWD/icons/LaunchScreen.storyboard)
    QMAKE_BUNDLE_DATA += app_launch_images

    QMAKE_ASSET_CATALOGS += $$PWD/icons/ios/Assets.xcassets

    HEADERS += \
        src/platform/appleplatform.h

    SOURCES += \
        src/platform/appleplatform.mm
}

MacBuild {
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15
    QMAKE_INFO_PLIST = mac/Info.plist
    ICON = $${BASEDIR}/icons/macos.icns
    DISTFILES += mac/Info.plist
    LIBS += -framework ApplicationServices
    LIBS += -framework VideoToolbox -framework CoreVideo -framework CoreMedia
    CONFIG += EnableSpeech
}

LinuxBuild {
    CONFIG += EnableSpeech
    message("LinuxBuild - config")
}

JetsonBuild {
    message("JetsonBuild")
    CONFIG += EnableSpeech
}

WindowsBuild {
    # This aparently makes qt use absolute paths, otherwise we get compile issues ?
    QMAKE_PROJECT_DEPTH = 0
    include(app/videostreaming/avcodec/avcodec_video.pri)
}

AndroidBuild {
    message("AndroidBuild")
    # Text to speech crashes on android for some weird reason
    #CONFIG += EnableSpeech
    QT += androidextras

    include(app/videostreaming/gstreamer/gst_video.pri)
    include(app/videostreaming/android/videostreamingandroid.pri)
    # needed to build android from windows, but not from linux
    QMAKE_PROJECT_DEPTH = 0
}

EnableSpeech {
    message("EnableSpeech")
    DEFINES += ENABLE_SPEECH
    QT += texttospeech
}


android{
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}
