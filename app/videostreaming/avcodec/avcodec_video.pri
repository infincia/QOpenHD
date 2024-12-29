# Common Include Paths and Sources
INCLUDEPATH += $$PWD

SOURCES += \
    $$PWD/QSGVideoTextureItem.cpp \
    $$PWD/gl/gl_shaders.cpp \
    $$PWD/gl/gl_videorenderer.cpp \
    $$PWD/texturerenderer.cpp \
    $$PWD/avcodec_decoder.cpp \

HEADERS += \
    $$PWD/QSGVideoTextureItem.h \
    $$PWD/gl/gl_shaders.h \
    $$PWD/gl/gl_videorenderer.h \
    $$PWD/texturerenderer.h \
    $$PWD/avcodec_decoder.h \
    $$PWD/avcodec_helper.hpp

# Windows-Specific Configuration
win32 {
    INCLUDEPATH += $$PWD/../../../build-libs-windows/ffmpeg/include
    LIBS += -L$$PWD/../../../build-libs-windows/ffmpeg/lib -lavcodec -lavutil -lavformat

    INCLUDEPATH += $$PWD/../../../build-libs-windows/angle-x64/include
    LIBS += -L$$PWD/../../../build-libs-windows/angle-x64/bin -lGLESv2 -lEGL

    DEFINES += EGL_EGLEXT_PROTOTYPES
    DEFINES += GL_GLEXT_PROTOTYPES
}

# Unix (Linux/Android) Configuration
unix {
    #include(../../lib/h264/h264.pri)
    LIBS += -lGLESv2 -lEGL

    # Optional: Include Linux-specific paths or libraries
    exists(/usr/local/include/uvgrtp) {
        INCLUDEPATH += /usr/local/include/uvgrtp
        LIBS += -L/usr/local/lib -luvgrtp
    }

    # Platform Detection for Raspberry Pi or Rock
    packagesExist(mmal) {
        DEFINES += IS_PLATFORM_RPI
    }
    exists(/usr/local/share/openhd/platform/rock/) {
        message(This is a Rock)
        DEFINES += IS_PLATFORM_ROCK
    } else {
        message(This is not a Rock)
    }
}

# macOS-Specific Configuration (if needed)
macx {
    # Add macOS-specific libraries or configurations here
}

# Shared Definitions
DEFINES += QOPENHD_ENABLE_VIDEO_VIA_AVCODEC

# Debugging Build Information
message(Building for platform $$QMAKE_PLATFORM)
win32: message(Windows build activated.)
unix: message(Unix build activated.)
macx: message(macOS build activated.)
