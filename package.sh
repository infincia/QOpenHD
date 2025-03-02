#!/bin/bash
QT_VERSION=Qt5.15.7

export LC_ALL=C.UTF-8
export LANG=C.UTF-8

CUSTOM=$1
PACKAGE_ARCH=$2
OS=$3
DISTRO=$4
BUILD_TYPE=$5


TMPDIR=/tmp/qopenhd/

rm -rf /tmp/qopenhd/*

mkdir -p /tmp/qopenhd/usr/local/bin || exit 1
mkdir -p /tmp/qopenhd/etc/systemd/system || exit 1

VER2=$(git rev-parse --short HEAD)

if [[ "${DISTRO}" == "bullseye" ]] || [[ "${DISTRO}" == "bionic" ]] && [[ "${PACKAGE_ARCH}" = "armhf" ]] ; then
    QT_VERSION=Qt5.15.4
    echo "debug"
    /opt/Qt5.15.4/bin/qmake
    echo "build with qmake done"
    make -j$(nproc)|| exit 1
    echo "build with make done"
elif [[ "${DISTRO}" == "jammy" ]] && [[ "${BUILD_TYPE}" = "debug" ]] ; then
    QT_VERSION=Qt5.15.7
    touch /etc/ld.so.conf.d/qt.conf
    sudo echo "/opt/Qt5.15.7/lib/" > /etc/ld.so.conf.d/qt.conf
    sudo ldconfig
    export PATH="$PATH:/opt/Qt5.15.7/bin/"
    sudo ln -s /opt/Qt5.15.7/bin/qmake /usr/bin/qmake  
    qmake
    echo "build with qmake done"
    make -j2 || exit 1
    echo "build with make done"
else
    echo "\ndebug\ndebug\ndebug\n"
    qmake
    echo "build with qmake done"
    make -j$(nproc)|| exit 1
    echo "build with make done"
fi

#Here are the dependencies depending on the platform
if [[ "${DISTRO}" == "bullseye" ]] && [[ "${OS}" == "raspbian" ]] ; then
#Raspberry
PLATFORM_PACKAGES="-d openhd-userland -d libavcodec-dev -d libavformat-dev -d openhd-qt -d gst-plugins-good -d gst-openhd-plugins -d gstreamer1.0-gl"
else
PLATFORM_PACKAGES="-d qml-module-qtcharts -d gstreamer1.0-gl -d qtgstreamer-plugins-qt5 -d gstreamer1.0-qt5 -d libqt5texttospeech5-dev -d qml-module-qt-labs-platform -d git -d libgstreamer-plugins-base1.0-dev -d gstreamer1.0-plugins-good -d libavcodec-dev -d libavformat-dev -d qml-module-qtquick-controls2 -d libqt5concurrent5 -d libqt5core5a -d libqt5dbus5 -d libqt5gui5 -d libqt5help5 -d libqt5location5 -d libqt5location5-plugins -d libqt5multimedia5 -d libqt5multimedia5-plugins -d libqt5multimediagsttools5 -d libqt5multimediawidgets5 -d libqt5network5 -d libqt5opengl5 -d libqt5opengl5-dev -d libqt5positioning5 -d libqt5positioning5-plugins -d libqt5positioningquick5 -d libqt5printsupport5 -d libqt5qml5 -d libqt5quick5 -d libqt5quickparticles5 -d libqt5quickshapes5 -d libqt5quicktest5 -d libqt5quickwidgets5 -d libqt5sensors5 -d libqt5sql5 -d libqt5sql5-sqlite -d libqt5svg5 -d libqt5test5 -d libqt5webchannel5 -d libqt5webkit5 -d libqt5widgets5 -d libqt5x11extras5 -d libqt5xml5 -d python3-pyqt5 -d python3-pyqt5.qtopengl -d python3-pyqt5.qtsvg -d python3-pyqt5.qtwebkit -d python3-pyqtgraph -d qml-module-qt-labs-settings -d qml-module-qtgraphicaleffects -d qml-module-qtlocation -d qml-module-qtpositioning -d qml-module-qtquick-controls -d qml-module-qtquick-dialogs -d qml-module-qtquick-extras -d qml-module-qtquick-layouts -d qml-module-qtquick-privatewidgets -d qml-module-qtquick-shapes -d qml-module-qtquick-window2 -d qml-module-qtquick2 -d qt5-gtk-platformtheme -d qt5-qmake -d qt5-qmake-bin -d qt5-qmltooling-plugins -d qtbase5-dev -d qtbase5-dev-tools -d qtchooser -d qtdeclarative5-dev -d qtdeclarative5-dev-tools -d qtpositioning5-dev -d qttranslations5-l10n"
fi
ls -a release/
echo "here we should have built the file correctly"

cp release/QOpenHD /tmp/qopenhd/usr/local/bin/ || exit 1
cp systemd/h264_decode.service /tmp/qopenhd/etc/systemd/system/ || exit 1
cp systemd/h265_decode.service /tmp/qopenhd/etc/systemd/system/ || exit 1

# copying services
if [[ "${PACKAGE_ARCH}" = "armhf" ]]; then
PACKAGE_NAME=qopenhd
cp systemd/rpi_qopenhd.service /tmp/qopenhd/etc/systemd/system/qopenhd.service || exit 1
elif [[ "${PACKAGE_ARCH}" = "arm64" ]]; then
mkdir -p /tmp/qopenhd/etc/systemd/system/
cp systemd/rock3_qopenhd.service /tmp/qopenhd/etc/systemd/system/qopenhd.service
PACKAGE_NAME=qopenhd_rk3566
else
PACKAGE_NAME=qopenhd
echo "X86 doesn't work with services"
fi

# The qt_eglfs_kms_config.json file makes sure that qopenhd runs at the res
# specified in the config.txt if the user did so
mkdir -p /tmp/qopenhd/usr/local/share/qopenhd/
if [[ "${PACKAGE_ARCH}" = "armhf" ]]; then
cp rpi_qt_eglfs_kms_config.json /tmp/qopenhd/usr/local/share/qopenhd/ || exit 1
elif [[ "${PACKAGE_ARCH}" = "arm64" ]]; then
cp rock_qt_eglfs_kms_config.json /tmp/qopenhd/usr/local/share/qopenhd/ || exit 1
fi

VERSION="2.6.3-$(date -d '+1 hour' +'%m-%d-%Y--%H-%M-%S')-${VER2}"

ls -a ${TMPDIR}/usr/local/bin
echo "going into packaging stage"

rm ${PACKAGE_NAME}_${VERSION}_${PACKAGE_ARCH}.deb > /dev/null 2>&1
if [[ "${PACKAGE_ARCH}" = "armhf" ]]; then
    echo "raspberry pi"
    fpm -a ${PACKAGE_ARCH} -s dir -t deb -n ${PACKAGE_NAME} -v ${VERSION} -C ${TMPDIR} \
    -p qopenhd_VERSION_ARCH.deb \
    --after-install after-install.sh \
    ${PLATFORM_PACKAGES} || exit 1
elif [[ "${PACKAGE_ARCH}" = "arm64" ]]; then
    echo "rockchip"
    fpm -a ${PACKAGE_ARCH} -s dir -t deb -n qopenhd_rk3566 -v ${VERSION} -C ${TMPDIR} \
    -p qopenhd_rk3566_VERSION_ARCH.deb \
    --after-install after-install.sh \
    ${PLATFORM_PACKAGES} || exit 1
    
    #Rock5Package
    rm /tmp/qopenhd/etc/systemd/system/qopenhd.service
    cp systemd/rock5_qopenhd.service /tmp/qopenhd/etc/systemd/system/qopenhd.service

    fpm -a ${PACKAGE_ARCH} -s dir -t deb -n qopenhd_rk3588 -v ${VERSION} -C ${TMPDIR} \
    -p qopenhd_rk3588_VERSION_ARCH.deb \
    --after-install after-install.sh \
    ${PLATFORM_PACKAGES} || exit 1
    #Rock5aPackage
    rm /tmp/qopenhd/etc/systemd/system/qopenhd.service
    cp systemd/rock5_qopenhd.service /tmp/qopenhd/etc/systemd/system/qopenhd.service

    fpm -a ${PACKAGE_ARCH} -s dir -t deb -n qopenhd_rk3588a -v ${VERSION} -C ${TMPDIR} \
    -p qopenhd_rk3588a_VERSION_ARCH.deb \
    --after-install after-install.sh \
    ${PLATFORM_PACKAGES} || exit 1
else
    echo "everything else"

    fpm -a ${PACKAGE_ARCH} -s dir -t deb -n ${PACKAGE_NAME} -v ${VERSION} -C ${TMPDIR} \
    -p qopenhd_VERSION_ARCH.deb \
    --after-install after-install.sh \
    ${PLATFORM_PACKAGES} || exit 1
fi
if [ -d "/out" ]; then
  ls -a
  cp *.deb /out/
fi
