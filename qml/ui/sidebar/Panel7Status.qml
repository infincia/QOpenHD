import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import QtQuick.Shapes 1.0
import QtQuick.Controls.Material 2.0

import Qt.labs.settings 1.0

import OpenHD 1.0

import "../elements"

SideBarBasePanel {
    override_title: "Status"

    Rectangle {
        color: "#171d25"
        anchors.top: parent.top
        anchors.topMargin: 50
        height: 340
        width: 320

        ColumnLayout {
            spacing: 10
            anchors.fill: parent
            anchors.topMargin: 12

            InfoElement2 {
                Layout.alignment: Qt.AlignHCenter
                override_text_left: "Connection:"
                override_color_right: {
                    if (_ohdSystemAir.is_alive || _ohdSystemGround.is_alive) {
                        return _ohdSystemAir.is_alive && _ohdSystemGround.is_alive ? "#20b383" : "#df4c7c";
                    }
                    return "#df4c7c";
                }
                override_text_right: {
                    if (_ohdSystemAir.is_alive || _ohdSystemGround.is_alive) {
                        return _ohdSystemAir.is_alive && _ohdSystemGround.is_alive ? "Connected" : (_ohdSystemAir.is_alive ? "AIR only" : "GND only");
                    }
                    return "Not connected";
                }
            }            InfoElement2 {
                Layout.alignment: Qt.AlignHCenter
                override_text_left: "QOpenHD Version:"
                override_color_right: "#20b383"
                override_text_right: {
                    return _qopenhd.version_string;
                }
            }
            InfoElement2 {
                Layout.alignment: Qt.AlignHCenter
                override_text_left: "OpenHD Version:"
                override_color_right: {
                    if (_ohdSystemGround.openhd_version !== "N/A") {
                        return _ohdSystemGround.openhd_version !== _ohdSystemAir.openhd_version ? "#df4c7c" : "#20b383";
                    }
                    return "#df4c7c";
                }
                override_text_right: {
                    if (_ohdSystemGround.openhd_version !== "N/A") {
                        return _ohdSystemGround.openhd_version !== _ohdSystemAir.openhd_version ? "Version mismatch" : _ohdSystemGround.openhd_version;
                    }
                    return "Not connected";
                }
            }
            InfoElement2 {
                Layout.alignment: Qt.AlignHCenter
                override_text_left: "Chipset GND:"
                override_color_right: _ohdSystemGround.is_alive ? "#20b383" : "#df4c7c"
                override_text_right: _ohdSystemGround.is_alive ? _ohdSystemGround.card_type_as_string : "Not connected"
            }
            InfoElement2 {
                Layout.alignment: Qt.AlignHCenter
                override_text_left: "Chipset AIR:"
                override_color_right: _ohdSystemAir.is_alive ? "#20b383" : "#df4c7c"
                override_text_right: _ohdSystemAir.is_alive ? _ohdSystemAir.card_type_as_string : "Not connected"
            }
            InfoElement2 {
                Layout.alignment: Qt.AlignHCenter
                override_text_left: "Camera:"
                override_color_right: _ohdSystemAir.is_alive ? "#20b383" : "#df4c7c"
                override_text_right: _ohdSystemAir.is_alive ? _ohdSystemAir.ohd_cam_type : "Not connected"
            }
            Button {
                Layout.alignment: Qt.AlignHCenter
                id: button
                text: "Advanced Menu"
                onClicked: {
                    settings_panel.visible = true
                }
            }
        }
    }
}
