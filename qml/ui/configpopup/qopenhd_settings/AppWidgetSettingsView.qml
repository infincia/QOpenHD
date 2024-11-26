import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import QtQuick.Controls.Material 2.12

import Qt.labs.settings 1.0

import OpenHD 1.0

import "../../../ui" as Ui
import "../../elements"

ScrollView {
    id: appWidgetSettingsView
    width: parent.width
    height: parent.height
    contentHeight: widgetColumn.height

    clip: true

    function close_after_preset(){
        settings_form.close_all();
    }

    function disable_or_enable_all(enable_elements){
        settings.show_downlink_rssi= enable_elements;
        settings.downlink_show_dbm_and_packets_per_card= enable_elements;
        settings.downlink_dbm_per_card_show_multiple_antennas=enable_elements;
        settings.downlink_show_current_bitrate= enable_elements;
        settings.downlink_cards_right= enable_elements;
        settings.downlink_signal_quality_show= enable_elements;
        settings.downlink_pollution_show=enable_elements;
        settings.show_uplink_rssi=enable_elements;
        settings.show_rc_rssi= enable_elements;
        settings.show_bitrate=enable_elements;
        settings.camera_bitrate_widget_show_recording= enable_elements;
        settings.show_air_battery= enable_elements;
        settings.show_ground_battery=enable_elements;
        settings.show_gps=enable_elements;
        settings.show_home_distance=enable_elements;
        settings.show_flight_time= enable_elements;
        settings.show_flight_mode= enable_elements;
        settings.show_flight_distance=enable_elements;
        settings.show_flight_mah= enable_elements;
        settings.show_flight_mah_km=enable_elements;
        settings.show_press_temp=enable_elements;
        settings.show_press_temp2=enable_elements
        settings.show_press_temp3=enable_elements
        settings.show_imu_temp=enable_elements;
        settings.show_airspeed_temp=enable_elements;
        settings.show_esc_temp=enable_elements;
        settings.show_battery_temp=enable_elements;
        settings.show_ground_status= enable_elements;
        settings.show_air_status=enable_elements;
        settings.show_message_hud= enable_elements;
        settings.show_horizon=enable_elements;
        settings.show_performance_horizon=enable_elements;
        settings.show_heading_ladder=enable_elements;
        settings.show_flight_path_vector=enable_elements;
        settings.speed_ladder_show=enable_elements;
        settings.speed_ladder_show_ladder=enable_elements;
        settings.show_speed_second=enable_elements;
        settings.altitude_ladder_show=enable_elements;
        settings.altitude_ladder_show_ladder=enable_elements;
        settings.show_altitude_second=enable_elements;
        settings.show_control=enable_elements;
        settings.show_vertical_speed_simple_widget=enable_elements;
        settings.show_vertical_speed_gauge_widget=enable_elements;
        settings.show_wind=enable_elements;
        settings.show_mission=enable_elements;
        settings.show_aoa=enable_elements;
        settings.wb_link_rate_control_widget_show=enable_elements;
        settings.show_arrow=enable_elements;
        settings.show_throttle=enable_elements;
        // dev only element and therefore always off
        settings.qrenderstats_show=false;
    }

    function enable_basic_elements(){
        settings.show_downlink_rssi= true;
        settings.show_uplink_rssi= true;
        settings.show_bitrate=true;
        settings.show_performance_horizon=true;
        settings.wb_link_rate_control_widget_show=true;
        settings.show_air_battery=true;
        settings.show_home_distance=true;
        settings.show_arrow=true;
    }

    function enable_medium_elements(){
        settings.show_vertical_speed_simple_widget=true;
        settings.altitude_ladder_show=true;
        settings.altitude_ladder_show_ladder=false;
    }

    function configure_preset1(){
        disable_or_enable_all(true);
        // Show the advanced horizon (but not 2 at the same time)
        settings.show_performance_horizon=false;
        settings.show_horizon=true;
        close_after_preset();
    }

    function configure_preset2(){
        disable_or_enable_all(false);
        enable_basic_elements();
        enable_medium_elements();
        close_after_preset();
    }

    function configure_preset3(){
        disable_or_enable_all(false);
        enable_basic_elements();
        close_after_preset();
    }

    Item {
        anchors.fill: parent
        Column {
            id: widgetColumn
            spacing: 0
            anchors.left: parent.left
            anchors.right: parent.right


            // TODO finalize
            /*SettingBaseElement{
                m_short_description: "Presets"
                m_long_description: "You can use these presets to quickly enable / disable a lot of elements before refining things further.\n"+
                                    "P1 (Preset 1): Enable all elements (completely overloaded)\n"+
                                    "P2 (Preset 2): Example for fixed wing usage, a lot less elements.\n"+
                                    "P3 (Preset 3): Example for small quad (inav/betaflight) usage, minimal\n";
                Row{
                    anchors.right: parent.right
                    anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 5
                    Button{
                        text:"P1"
                        onClicked: {
                            configure_preset1()
                        }
                    }
                    Button{
                        text: "P2"
                        onClicked: {
                            configure_preset2()
                        }
                    }
                    Button{
                        text: "P3";
                        onClicked: {
                            configure_preset3()
                        }
                    }
                }
            }*/

            SettingsCategory{
                m_description: "OHD LINK / STREAMING WIDGETS"

                SettingBaseElement{
                    m_short_description: "Show Widgets"
                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_widgets
                        onCheckedChanged: settings.show_widgets = checked
                    }
                }

                SettingBaseElement{
                    m_short_description: "Show Downlink RSSI"
                    m_long_description: "RSSI / Stats about downlink"
                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_downlink_rssi
                        onCheckedChanged: settings.show_downlink_rssi = checked
                    }
                }

                SettingBaseElement{
                    m_short_description: "Show Uplink RSSI"
                    m_long_description: "RSSI / Stats about uplink"
                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_uplink_rssi
                        onCheckedChanged: settings.show_uplink_rssi = checked
                    }
                }
                SettingBaseElement{
                    m_short_description: "Show live rate control widget"
                    m_long_description: "Trade range / stability for bitrate at run time. Requires hw support."
                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.wb_link_rate_control_widget_show
                        onCheckedChanged: settings.wb_link_rate_control_widget_show = checked
                    }
                }
                SettingBaseElement{
                    m_short_description: "Show video widget"
                    m_long_description: "More stats about each camera stream, quick resolution changes and quick air recording."
                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_bitrate
                        onCheckedChanged: settings.show_bitrate = checked
                    }
                }

            }
            SettingsCategory{
                m_description: "UAV (FC) WIDGETS"



                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show GPS")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_gps
                        onCheckedChanged: settings.show_gps = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Home Distance")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_home_distance
                        onCheckedChanged: settings.show_home_distance = checked
                    }
                }
                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show RC RSSI (not OpenHD RC)")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_rc_rssi
                        onCheckedChanged: settings.show_rc_rssi = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Flight Timer")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_flight_time
                        onCheckedChanged: settings.show_flight_time = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Flight Mode")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_flight_mode
                        onCheckedChanged: settings.show_flight_mode = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show IMU Sensor Temperature")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_imu_temp
                        onCheckedChanged: settings.show_imu_temp = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Pressure Sensor Temperature")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_press_temp
                        onCheckedChanged: settings.show_press_temp = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Pressure Sensor 2 Temperature")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_press_temp2
                        onCheckedChanged: settings.show_press_temp2 = checked
                    }
                }
                
                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Pressure Sensor 3 Temperature")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_press_temp3
                        onCheckedChanged: settings.show_press_temp3 = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Airspeed Sensor Temperature")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_airspeed_temp
                        onCheckedChanged: settings.show_airspeed_temp = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Esc Temperature")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_esc_temp
                        onCheckedChanged: settings.show_esc_temp = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Battery Temperature")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_battery_temp
                        onCheckedChanged: settings.show_battery_temp = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Ground Status")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_ground_status
                        onCheckedChanged: settings.show_ground_status = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Ground Battery")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_ground_battery
                        onCheckedChanged: settings.show_ground_battery = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Air Status")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_air_status
                        onCheckedChanged: settings.show_air_status = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Air Battery")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_air_battery
                        onCheckedChanged: settings.show_air_battery = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show mAh")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_flight_mah
                        onCheckedChanged: settings.show_flight_mah = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Efficiency in mAh/km")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_flight_mah_km
                        onCheckedChanged: settings.show_flight_mah_km = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Total Flight Distance")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_flight_distance
                        onCheckedChanged: settings.show_flight_distance = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Horizon")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_horizon
                        onCheckedChanged: settings.show_horizon = checked
                    }
                }
                // Exp
                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show performance horizon")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_performance_horizon
                        onCheckedChanged: settings.show_performance_horizon = checked
                    }
                }
                // Exp end

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Flight Path Vector")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_flight_path_vector
                        onCheckedChanged: settings.show_flight_path_vector = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Altitude")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.altitude_ladder_show
                        onCheckedChanged: settings.altitude_ladder_show = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Speed")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.speed_ladder_show
                        onCheckedChanged: settings.speed_ladder_show = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Second Speed")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_speed_second
                        onCheckedChanged: settings.show_speed_second = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Heading")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_heading
                        onCheckedChanged: settings.show_heading = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Second Altitude")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_altitude_second
                        onCheckedChanged: settings.show_altitude_second = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Bank Angle Indicator")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.bank_angle_indicator_widget_show
                        onCheckedChanged: settings.bank_angle_indicator_widget_show = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Home Arrow")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_arrow
                        onCheckedChanged: settings.show_arrow = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Throttle")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_throttle
                        onCheckedChanged: settings.show_throttle = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Control Inputs")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_control
                        onCheckedChanged: settings.show_control = checked
                    }
                }



                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Vibration")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_vibration
                        onCheckedChanged: settings.show_vibration = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Vertical Speed (minimal)")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_vertical_speed_simple_widget
                        onCheckedChanged: settings.show_vertical_speed_simple_widget = checked
                    }
                }
                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Vertical Speed Gauge")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_vertical_speed_gauge_widget
                        onCheckedChanged: settings.show_vertical_speed_gauge_widget = checked
                    }
                }
                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Wind (Experimental)")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_wind
                        onCheckedChanged: settings.show_wind = checked
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Missions")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_mission
                        onCheckedChanged: {
                            settings.show_mission = checked;
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Angle of Attack")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_aoa
                        onCheckedChanged: {
                            settings.show_aoa = checked;
                        }
                    }
                }


                SettingBaseElement{
                    m_short_description: "Show distance sensor widget"
                    m_long_description: "For UAVs with a distance sensor"
                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_distance_sensor_widget
                        onCheckedChanged: settings.show_distance_sensor_widget = checked
                    }
                }

                SettingBaseElement{
                    m_short_description: "Show (GPS) time widget"
                    m_long_description: "For UAVs with GPS, shows the time as reported by the UAV"
                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_time_widget
                        onCheckedChanged: settings.show_time_widget = checked
                    }
                }
            }

            SettingsCategory {
                m_description: "OTHER"

                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show Map")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_map
                        onCheckedChanged: {
                            // Weird qt specifcs, see https://stackoverflow.com/questions/56817460/qml-appswitch-sending-oncheckedchanged-signal-each-time-page-is-opened
                            if(settings.show_map != checked){
                                settings.show_map = checked;
                                _restartqopenhdmessagebox.show();
                            }
                            //settings.show_map = checked
                        }
                    }
                }
                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show ADSB")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.adsb_enable
                        onCheckedChanged: settings.adsb_enable = checked
                    }
                }
                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show log messages on-screen")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_message_hud
                        onCheckedChanged: settings.show_message_hud = checked
                    }
                }
                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"
                    enabled: true//false // Not yet functional
                    Text {
                        text: qsTr("Show Sidebar")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_sidebar
                        onCheckedChanged: settings.show_sidebar = checked
                    }
                }
                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

                    Text {
                        text: qsTr("Show QT Render stats")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.qrenderstats_show
                        onCheckedChanged: {
                            settings.qrenderstats_show = checked;
                        }
                    }
                }
                Rectangle {
                    width: parent.width
                    height: rowHeight
                    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"
                    enabled: false // Not yet functional

                    Text {
                        text: qsTr("Show GPIO")
                        font.weight: Font.Bold
                        font.pixelSize: 13
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 224
                        height: elementHeight
                        anchors.left: parent.left
                    }

                    Switch {
                        width: 32
                        height: elementHeight
                        anchors.rightMargin: Qt.inputMethod.visible ? 96 : 36

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: settings.show_gpio
                        onCheckedChanged: settings.show_gpio = checked
                    }
                }
            }
        }

    }
}

