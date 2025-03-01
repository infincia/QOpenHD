import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import QtQuick.Shapes 1.0
import QtQuick.Controls.Material 2.0

import Qt.labs.settings 1.0

import OpenHD 1.0

import "../elements"

SideBarBasePanel {
    override_title: "Air recording"

    function takeover_control(){
        recording_mode.takeover_control();
    }

    Column {
        anchors.top: parent.top
        anchors.topMargin: secondaryUiHeight / 8
        spacing: 5

        Rectangle {
            color: "transparent"
            height: recording_mode.height
            width: recording_mode.width
            MavlinkChoiceElement2 {
                id: recording_mode
                anchors.centerIn: parent
                anchors.leftMargin: 40
                m_title: "Recording"
                m_param_id: "AIR_RECORDING_E"
                m_settings_model: _airCameraSettingsModel
                onGoto_previous: {
                    sidebar.regain_control_on_sidebar_stack()
                }
                onGoto_next: {
                    // Do nothing
                }
            }
        }
        Rectangle {
            color: "#171d25"
            height: recording_mode.height
            width: recording_mode.width
            Text {
                anchors.centerIn: parent
                anchors.leftMargin: 40
                text: {
                    var tmp = "  Status";
                    if (!_ohdSystemAir.is_alive) {
                        return tmp + "                   disabled ";
                    }
                    return tmp + _cameraStreamModelPrimary.camera_recording_mode_to_string(_cameraStreamModelPrimary.air_recording_active)
                }
                font.pixelSize: 18
                color: "white"
                smooth: true
            }
        }
        Rectangle {
            color: secondaryUiColor
            height: recording_mode.height+120
            width: recording_mode.width
        }
    }
}
