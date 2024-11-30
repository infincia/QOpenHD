# Enable GStreamer QMLGLSink for video rendering
DEFINES += QOPENHD_ENABLE_GSTREAMER_QMLGLSINK

# Uncomment for primary video, keep secondary for now
# DEFINES += QOPENHD_GSTREAMER_PRIMARY_VIDEO
DEFINES += QOPENHD_GSTREAMER_SECONDARY_VIDEO

# Sources and headers specific to GStreamer
SOURCES += \
    $$PWD/gstqmlglsinkstream.cpp \
    $$PWD/gstrtpaudioplayer.cpp \
    $$PWD/gstrtpreceiver.cpp

HEADERS += \
    $$PWD/gst_helper.hpp \
    $$PWD/gstqmlglsinkstream.h \
    $$PWD/gstrtpaudioplayer.h \
    $$PWD/gstrtpreceiver.h

# Platform-specific configurations
android {
    message("Configuring GStreamer for Android")

    # Define the base directory for the GStreamer prebuilt binaries
    DOWNLOADED_GST_FOLDER = $$PWD/../../../lib/gstreamer_prebuilts/gstreamer-1.0-android-universal

    # Determine the GStreamer architecture folder based on the target architecture
    GSTREAMER_ARCH_FOLDER = armv7
    contains(ANDROID_TARGET_ARCH, armeabi-v7a) {
        GSTREAMER_ARCH_FOLDER = armv7
    } else:contains(ANDROID_TARGET_ARCH, arm64-v8a) {
        GSTREAMER_ARCH_FOLDER = arm64
    } else:contains(ANDROID_TARGET_ARCH, x86_64) {
        GSTREAMER_ARCH_FOLDER = x86_64
    } else {
        message("Unknown ANDROID_TARGET_ARCH: $$ANDROID_TARGET_ARCH")
        GSTREAMER_ARCH_FOLDER = armv7
    }

    # Define the root path for GStreamer
    GSTREAMER_ROOT_ANDROID = $$DOWNLOADED_GST_FOLDER/$$GSTREAMER_ARCH_FOLDER
    message("GStreamer root for Android: $$GSTREAMER_ROOT_ANDROID")

    GST_ROOT = $$GSTREAMER_ROOT_ANDROID

    # Check if the GStreamer root directory exists
    exists($$GST_ROOT) {
        message("Setting up GStreamer for Android build")
        QMAKE_CXXFLAGS += -pthread
        CONFIG += VideoEnabled

        # Link GStreamer libraries
        LIBS += -L$$GST_ROOT/lib/gstreamer-1.0 \
            -lgstvideo-1.0 -lgstcoreelements -lgstplayback -lgstudp \
            -lgstrtp -lgstrtsp -lgstx264 -lgstlibav -lgstsdpelem \
            -lgstvideoparsersbad -lgstrtpmanager -lgstisomp4 \
            -lgstmatroska -lgstmpegtsdemux -lgstandroidmedia \
            -lgstopengl -lgsttcp -lgstapp -lgstalaw -lgstautodetect

        LIBS += -L$$GST_ROOT/lib \
            -lgraphene-1.0 -ljpeg -lpng16 -lgstfft-1.0 -lm \
            -lgstnet-1.0 -lgio-2.0 -lgstphotography-1.0 -lgstgl-1.0 -lEGL \
            -lgstaudio-1.0 -lgstcodecparsers-1.0 -lgstbase-1.0 -lgstreamer-1.0 \
            -lgstrtp-1.0 -lgstpbutils-1.0 -lgstrtsp-1.0 -lgsttag-1.0 \
            -lgstvideo-1.0 -lavformat -lavcodec -lavutil -lx264 -lavfilter \
            -lswresample -lgstriff-1.0 -lgstcontroller-1.0 -lgstapp-1.0 \
            -lgstsdp-1.0 -lbz2 -lgobject-2.0 -lgstmpegts-1.0 \
            -Wl,--export-dynamic -lgmodule-2.0 -pthread -lglib-2.0 -lorc-0.4 -liconv -lffi -lintl

        # Include paths for GStreamer
        INCLUDEPATH += \
            $$GST_ROOT/include/gstreamer-1.0 \
            $$GST_ROOT/lib/gstreamer-1.0/include \
            $$GST_ROOT/include/glib-2.0 \
            $$GST_ROOT/lib/glib-2.0/include
    } else {
        message("GStreamer prebuilt directory does not exist")
    }
} else {
    # GStreamer configuration for non-Android platforms
    GST_ROOT = c:/gstreamer/1.0/x86_64

    exists($$GST_ROOT) {
        CONFIG += VideoEnabled

        # Link GStreamer libraries
        LIBS += -L$$GST_ROOT/lib -lopengl32

        LIBS += -lgstfft-1.0 \
                -lgstnet-1.0 -lgio-2.0 \
                -lgstaudio-1.0 -lgstcodecparsers-1.0 -lgstbase-1.0 \
                -lgstreamer-1.0 -lgstrtp-1.0 -lgstpbutils-1.0 -lgstrtsp-1.0 -lgsttag-1.0 \
                -lgstvideo-1.0 \
                -lgstriff-1.0 -lgstcontroller-1.0 -lgstapp-1.0 \
                -lgstsdp-1.0 -lbz2 -lgobject-2.0 \
                -lgstgl-1.0 -lgraphene-1.0 -lpng16 -ljpeg -lgstphotography-1.0 \
                -lgmodule-2.0 -lglib-2.0 -lorc-0.4 -lffi -lintl

        # Include paths for GStreamer
        INCLUDEPATH += \
            $$GST_ROOT/include/gstreamer-1.0 \
            $$GST_ROOT/include/glib-2.0 \
            $$GST_ROOT/lib/gstreamer-1.0/include \
            $$GST_ROOT/lib/glib-2.0/include

    }
}
