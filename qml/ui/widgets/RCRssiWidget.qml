import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Qt.labs.settings 1.0

import OpenHD 1.0

// RSSI reported by the FC, aka the RSSI from the (non-wifibroadcast!) rc controll link
// (In case rc over wb is used, this widget should be disabled)
BaseWidget {
    id: rcRSSIWidget
    width: 50
    height: 30

    visible: settings.show_rc_rssi && settings.show_widgets

    widgetIdentifier: "rc_rssi_widget"
    bw_verbose_name: "RC RSSI"

    defaultAlignment: 1
    defaultXOffset: 12
    defaultYOffset: 64
    defaultHCenter: false
    defaultVCenter: false

    hasWidgetDetail: true

    // -1 denotes N/A
    // Otherwise,value in percent
    property int rc_rssi_percentage: _fcMavlinkSystem.rc_rssi_percentage

    function is_valid_rssi(){
        if (rc_rssi_percentage <= -1) {
            return false;
        }
        return true;
    }

    function get_color(){
        if(is_valid_rssi()){
            return settings.color_shape
        }
        return "red"
    }

    widgetDetailComponent: ScrollView {

        contentHeight: idBaseWidgetDefaultUiControlElements.height
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        clip: true

        BaseWidgetDefaultUiControlElements{
            id: idBaseWidgetDefaultUiControlElements
        }
    }

    Item {
        id: widgetInner

        anchors.fill: parent

        Item {
            anchors.fill: parent
            anchors.centerIn: parent
            transform: Scale {
                origin.x: 25
                origin.y: 15
                xScale: bw_current_scale
                yScale: bw_current_scale
            }

            Rectangle {
                id: rcRSSI_0to20
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                width: 3
                height: 5
                border.color: settings.color_glow
                border.width: 1
                color: get_color()
                //always visible
            }

            Rectangle {
                id: rcRSSI_21to40
                anchors.left: rcRSSI_0to20.right
                anchors.bottom: parent.bottom
                anchors.leftMargin: 2
                width: 3
                height: 10
                border.color: settings.color_glow
                border.width: 1
                color: get_color()
                visible: {
                    if (rc_rssi_percentage > 20 || !is_valid_rssi()) {
                        return true
                    } else {
                        return false
                    }
                }
            }

            Rectangle {
                id: rcRSSI_41to60
                anchors.left: rcRSSI_21to40.right
                anchors.bottom: parent.bottom
                anchors.leftMargin: 2
                border.color: settings.color_glow
                border.width: 1
                width: 3
                height: 15
                color: get_color()
                visible: {
                    if (rc_rssi_percentage > 40 || !is_valid_rssi()) {
                        return true
                    } else {
                        return false
                    }
                }
            }

            Rectangle {
                id: rcRSSI_61to80
                anchors.left: rcRSSI_41to60.right
                anchors.bottom: parent.bottom
                anchors.leftMargin: 2
                border.color: settings.color_glow
                border.width: 1
                width: 3
                height: 20
                color: get_color()
                visible: {
                    if (rc_rssi_percentage > 60 || !is_valid_rssi()) {
                        return true
                    } else {
                        return false
                    }
                }
            }

            Rectangle {
                id: rcRSSI_81to100
                anchors.left: rcRSSI_61to80.right
                anchors.bottom: parent.bottom
                anchors.leftMargin: 2
                border.color: settings.color_glow
                border.width: 1
                width: 3
                height: 25
                color: get_color()
                visible: {
                    if (rc_rssi_percentage > 80 || !is_valid_rssi()) {
                        return true
                    } else {
                        return false
                    }
                }
            }

            Text {
                id: percent_symbol
                color: settings.color_text
                opacity: bw_current_opacity
                text: is_valid_rssi() ? qsTr("%") : qsTr("")
                anchors.right: rcRSSI_21to40.left
                anchors.rightMargin: 0
                anchors.top: parent.top
                anchors.topMargin: 0
                horizontalAlignment: Text.AlignLeft
                font.pixelSize: 10
                verticalAlignment: Text.AlignTop
                wrapMode: Text.NoWrap
                elide: Text.ElideRight
                style: Text.Outline
                styleColor: settings.color_glow
            }

            Text {
                id: rc_rssi
                color: settings.color_text
                opacity: bw_current_opacity
                text: is_valid_rssi() ? rc_rssi_percentage : qsTr("N/A")
                anchors.right: percent_symbol.left
                anchors.rightMargin: 2
                anchors.top: parent.top
                anchors.topMargin: 0
                horizontalAlignment: Text.AlignRight
                font.pixelSize: 14
                font.family: settings.font_text
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.NoWrap
                elide: Text.ElideRight
                style: Text.Outline
                styleColor: settings.color_glow
            }
        }
    }
}
