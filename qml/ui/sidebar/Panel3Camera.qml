import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import QtQuick.Shapes 1.0
import QtQuick.Controls.Material 2.0

import Qt.labs.settings 1.0

import OpenHD 1.0

import "../elements"

SideBarBasePanel{
    override_title: "Camera"

    function takeover_control(){
        brightness.takeover_control();
    }

    Column {
        anchors.top: parent.top
        anchors.topMargin: secondaryUiHeight/8
        spacing: 5
        MavlinkChoiceElement2{
            id: brightness
            m_title: "Brightness"
            m_param_id: "BRIGHTNESS"
            m_settings_model: _airCameraSettingsModel
            onGoto_previous: {
                sidebar.regain_control_on_sidebar_stack()
            }
            onGoto_next: {
                exposure.takeover_control();
            }
        }
        MavlinkChoiceElement2{
            id: exposure
            m_title: "Exposure"
            m_param_id: "EXPOSURE"
            m_settings_model: _airCameraSettingsModel
            onGoto_previous: {
               brightness.takeover_control();
            }
            onGoto_next: {
                metering.takeover_control();
            }
        }
        MavlinkChoiceElement2{
            id: metering
            m_title: "Metering"
            m_param_id: "METERING"
            m_settings_model: _airCameraSettingsModel
            onGoto_previous: {
               exposure.takeover_control();
            }
            onGoto_next: {
                sharpness.takeover_control();
            }
        }
        MavlinkChoiceElement2{
            id: sharpness
            m_title: "Sharpness"
            m_param_id: "SHARPNESS"
            m_settings_model: _airCameraSettingsModel
            onGoto_previous: {
               contrast.takeover_control();
            }
            onGoto_next: {
                sidebar.regain_control_on_sidebar_stack()
            }
        }

    }
}
