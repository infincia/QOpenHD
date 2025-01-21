import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import QtQuick.Controls.Material 2.12

import Qt.labs.settings 1.0

import OpenHD 1.0

import "../../../ui" as Ui
import "../../elements"

Column {
    //Layout.fillWidth: true
    //Layout.fillHeight: true
    // from https://doc.qt.io/qt-6/qml-qtquick-layouts-rowlayout.html
    //anchors.fill: parent
    width: parent.width
    height: parent.height
    spacing: 1

    property bool m_is_ground: false

    property var m_model: m_is_ground ? _ohdSystemGround : _ohdSystemAir

    property string m_version: m_model.openhd_version
    property bool m_is_alive: m_model.is_alive
    property string m_qopenhd_version: _qopenhd.version_string

    //fucking hell qt
    property int m_font_pixel_size: 13

    function get_gnd_active_cards(){
        var ret=0;
        if(_wifi_card_gnd0.alive) ret++;
        if(_wifi_card_gnd1.alive) ret++;
        if(_wifi_card_gnd2.alive) ret++;
        if(_wifi_card_gnd3.alive) ret++;
        return ret;
    }

    function gnd_uplink_state(){
        if(!_ohdSystemGround.is_alive)return 0;
        if(!_ohdSystemAir.is_alive)return 0;
        if(!_ohdSystemAir.curr_rx_last_packet_status_good)return -1;
        return 1;
    }
    function gnd_uplink_state_text(){
        var curr_gnd_uplink_state=gnd_uplink_state();
        if(curr_gnd_uplink_state==0)return "N/A";
        if(curr_gnd_uplink_state==1) return "OK";
        return "ERROR";
    }

    function get_text_qopenhd_openhd_ground_version_mismatch(){
        var ret="Your version of QOpenHD ["+m_qopenhd_version+"] is incompatible with your ground station OpenHD version: ["+m_version+"]"+
                "\nPlease update QOpenHD / your Ground station.";
        return ret;
    }
    function get_text_openhd_air_ground_version_mismatch(){
        var ret="Your version of OpenHD AIR ["+_ohdSystemAir.openhd_version+"] is incompatible with OpenHD GROUND: ["+_ohdSystemGround.openhd_version+"]"+
                "\nPlease update your AIR / GROUND unit.";
        return ret;
    }

    StatusCardRow{
        m_left_text: qsTr("Platform:")
        m_right_text: m_is_ground ? _ohdSystemGround.ohd_platform_type_as_string : _ohdSystemAir.ohd_platform_type_as_string
        m_has_error: false
    }

    StatusCardRow{
        m_left_text: qsTr("Version:")
        m_right_text: m_version
        m_has_error: {
            if(m_is_ground){
                // Show if ground reported version is valid and there is a mismatch OpenHD ground / QOpenHD
                if(_ohdSystemGround.openhd_version=="N/A"){
                    return false;
                }
                return _ohdSystemGround.openhd_version != m_qopenhd_version;
            }else{
                // Show if ground and air reported version is valid and there is a mismatch
                if(_ohdSystemGround.openhd_version=="N/A" || _ohdSystemAir.openhd_version=="N/A"){
                    return false;
                }
                return _ohdSystemGround.openhd_version != _ohdSystemAir.openhd_version
            }
        }
        m_error_text: {
            var text_warning= m_is_ground ? get_text_qopenhd_openhd_ground_version_mismatch() : get_text_openhd_air_ground_version_mismatch()
            return text_warning;
        }
        m_error_view: true
    }
    StatusCardRow {
        m_left_text: qsTr("Link:")

        m_right_text: {
            function pingToMsString(pingValue) {
                var pingNum = parseFloat(pingValue)

                // If it's not a number (NaN) or zero, return an empty string
                if (isNaN(pingNum) || pingNum === 0) {
                    return ""
                }

                // Otherwise, convert to an integer (or leave as a string)
                // e.g., rounding down to remove decimals
                return Math.floor(pingNum).toString()
            }

            if (m_is_ground) {
                if (!_ohdSystemGround.is_alive) {
                    return "N/A"
                }
                var alive_count = get_gnd_active_cards()
                if (alive_count === 0) {
                    return "waiting"
                }
                if (alive_count === 1) {
                    return _wifi_card_gnd0.card_type_as_string
                }
                var pingStrGnd = pingToMsString(m_model.last_ping_result_openhd)
                if (pingStrGnd !== "") {
                    return alive_count + "x array " + pingStrGnd + " ms"
                } else {
                    return alive_count + "x array"
                }
            } else {
                if (!_wifi_card_air.alive) {
                    return "n/a"
                }
                var pingStrAir = pingToMsString(m_model.last_ping_result_openhd)
                var cardTypeStr = _wifi_card_air.card_type_as_string
                if (pingStrAir !== "") {
                    return cardTypeStr + " " + pingStrAir + " ms"
                } else {
                    return cardTypeStr
                }
            }
        }

        m_has_error: {
            var show_message_card_unsupported = false
            if (m_is_ground) {
                if (_wifi_card_gnd0.alive && !_wifi_card_gnd0.card_type_supported) {
                    show_message_card_unsupported = true
                }
            } else {
                if (_wifi_card_air.alive && !_wifi_card_air.card_type_supported) {
                    show_message_card_unsupported = true
                }
            }
            return show_message_card_unsupported
        }

        m_error_text: {
            var message = "Using unsupported card(s) has side effects like non-working frequency changes, no uplink gnd-air or bad range. be warned !"
            return message
        }

        m_error_view: true
    }
    // StatusCardRow{
    //     visible: m_is_ground
    //     m_left_text: "Uplink:"
    //     m_right_text: {
    //         return gnd_uplink_state_text()
    //     }
    //     m_has_error: {
    //         return m_right_text=="ERROR"
    //     }
    //     m_error_text: {
    //         var message="Looks like your uplink (GND to AIR) is not functional - please use a supported card on your GND station"+
    //         " and make sure passive (listen only) mode is disabled on your ground station."
    //         return message;
    //     }
    // }
    StatusCardRow{
        m_left_text: "Hidden:"
        m_right_text: "No"
    }
    StatusCardRow{
        visible: !m_is_ground
        m_left_text: "SysId:"
        m_right_text: {
            var air_fc_sys_id=_ohdSystemAir.air_reported_fc_sys_id;
            if(air_fc_sys_id==-1){
                return "NOT FOUND"
            }
            return "FOUND:"+air_fc_sys_id;
        }
        m_error_text: {
            var air_fc_sys_id=_ohdSystemAir.air_reported_fc_sys_id;
            if(air_fc_sys_id==-1){
                return "Your AIR Unit cannot find your FC (UART). Please check:\n"+
                        "1) Wiring\n"+
                        "2) Right serial port enabled (OpenHD AIR param set)\n"+
                        "3) MAVLINK selected on your FC uart";
            }
            return "Please make sure:\n"+
                    "1) ARDUPILOT / PX4: FC sys id needs to be set to 1 (DEFAULT ARDUPILOT SYSID)\n"+
                    "2) INAV / Betaflight: Nothing needs to be changed, sys id should be 0";
        }
        m_has_error: {
            if(!_ohdSystemAir.is_alive)return false;
            var air_fc_sys_id=_ohdSystemAir.air_reported_fc_sys_id;
            // We allow 0 (Betaflight / inav / ... ) and 1 (Arupilot)
            if(air_fc_sys_id==0 || air_fc_sys_id==1)return false;
            return true;
        }
        m_error_view: true;
    }
    StatusCardRow{
        m_left_text: "WiFi HS:"
        m_right_text: {
            if(!m_model.is_alive || m_model.wifi_hotspot_state<0){
                return "N/A";
            }
            if(m_model.wifi_hotspot_state==0){
                return "UNAVAILABLE";
            }
            if(m_model.wifi_hotspot_state==1){
                return "DISABLED";
            }
            return "ENABLED";
        }
        m_right_text_color: {
            if(m_right_text=="DISABLED" || m_right_text=="ENABLED"){
                return "green"
            }
            return "black";
        }
        m_right_text_color_error: "black";
        m_error_text: {
            if(m_model.wifi_hotspot_state==0){
                var message="To use the WiFi hotspot feature on your air / ground unit you need to use a RPI / ROCK with integrated wifi module. "
                return message;
            }
            if(m_model.wifi_hotspot_state==1 || m_model.wifi_hotspot_state==2){
                var message="WiFi hotspot is automatically disabled when armed, and enabled when disarmed. To change this behaviour (discouraged) set it to either always off / always on";
                return message;
            }
            return "I should not appear";
        }
        m_has_error: {
            return m_right_text=="UNAVAILABLE";
        }
    }
    StatusCardRow{
        visible: m_is_ground;
        m_left_text: "Shared:"
        m_right_text: {
            if(_ohdSystemGround.external_devices_count<0){
                return "No";
            }
            if(_ohdSystemGround.external_devices_count==0){
                return "Yes";
            }
            return _ohdSystemGround.external_devices_count+"x";
        }
    }
    StatusCardRow{
        visible: false;
        m_left_text: "Licence:"
        m_right_text: "Community"
    }

    // Padding
    Item{
        Layout.fillWidth: true
        Layout.fillHeight: true
    }
}
