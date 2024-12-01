import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Qt.labs.settings 1.0

import OpenHD 1.0

SOCStatusWidgetGeneric {
    id: airStatusWidget
    width: 150
    height: 24

    visible: settings.show_air_status && settings.show_widgets

    defaultAlignment: 1
    defaultXOffset: 128
    defaultYOffset: 0
    defaultHCenter: false
    defaultVCenter: false

    hasWidgetDetail: true
    hasWidgetAction: true
    widgetIdentifier: "air_status_widget"
    bw_verbose_name: "CPU LOAD AIR"

    m_is_air: true
}
