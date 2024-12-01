INCLUDEPATH += $$PWD

# just using the something something webrtc from stephen was the easiest solution.
include(../../../lib/h264/h264.pri)

SOURCES += \
    $$PWD/rtp/ParseRTP.cpp \
    $$PWD/rtp/rtpreceiver.cpp \
    $$PWD/udp/UDPReceiver.cpp \
    $$PWD/decodingstatistcs.cpp \

HEADERS += \
    $$PWD/nalu/CodecConfigFinder.hpp \
    $$PWD/nalu/NALUnitType.hpp \
    $$PWD/rtp/ParseRTP.h \
    $$PWD/rtp/RTP.hpp \
    $$PWD/rtp/rtpreceiver.h \
    $$PWD/udp/UDPReceiver.h \
    $$PWD/decodingstatistcs.h \
    $$PWD/QOpenHDVideoHelper.hpp \


HEADERS += \
    $$PWD/ExternalDecodeService.hpp \
    $$PWD/audio_playback.h \
    $$PWD/custom/rawreceiver.h

SOURCES += \
    $$PWD/audio_playback.cpp \
    $$PWD/custom/rawreceiver.cpp

