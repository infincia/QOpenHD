import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import QtQuick.Controls.Material 2.12

import Qt.labs.settings 1.0

import OpenHD 1.0

import "../../../ui" as Ui
import "../../elements"

Column {
    width: parent.width
    height: parent.height
    spacing: 1

    property bool m_is_alive: _fcMavlinkSystem.is_alive
    function get_alive_text(){
        return m_is_alive ? "Yes" : "No"
    }
    function get_alive_text_color(){
        //return m_is_alive ? "green" : "red"
        return m_is_alive ? "green" : "black"
    }

    StatusCardRow{
        m_left_text: qsTr("Firmware:") //Autopilot
        m_right_text:  _fcMavlinkSystem.autopilot_type_str
    }

    StatusCardRow{
        m_left_text: qsTr("Ping:")
        m_right_text: _fcMavlinkSystem.last_ping_result_flight_ctrl
    }

    StatusCardRow{
        m_left_text: qsTr("Type:") // MAV type
        m_right_text: _fcMavlinkSystem.mav_type_str
    }

    StatusCardRow{
        m_left_text: qsTr("Alive:")
        m_right_text: get_alive_text();
        m_right_text_color: get_alive_text_color()
    }

    StatusCardRow{
        m_left_text: qsTr("SysID:")
        m_right_text: _fcMavlinkSystem.for_osd_sys_id == -1 ? "N/A" : qsTr(""+_fcMavlinkSystem.for_osd_sys_id)
    }
    // Padding
    Item{
        Layout.fillWidth: true
        Layout.fillHeight: true
    }
}
