import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import OpenHD 1.0
import "../../../ui" as Ui
import "../../elements"

// Main container
Rectangle {
    id: elementAppDeveloperStats
    width: parent.width
    height: parent.height
    color: "#eaeaea"

    function yes_or_no_as_string(yes) {
        if (yes) return "Y";
        return "N";
    }

    function get_features_string() {
        var ret = "";
        ret += "AVCODEC:" + yes_or_no_as_string(QOPENHD_ENABLE_VIDEO_VIA_AVCODEC) + ", ";
        ret += "MMAL:" + yes_or_no_as_string(QOPENHD_HAVE_MMAL) + ", ";
        ret += "GSTREAMER_QMLGLSINK:" + yes_or_no_as_string(QOPENHD_ENABLE_GSTREAMER_QMLGLSINK) + ", ";
        return ret;
    }

    // TabBar for consistent style
    TabBar {
        id: selectItemInStackLayoutBar
        width: parent.width
        TabButton {
            text: qsTr("Developer Menu")
        }
    }

    // Main layout with scrollable content
    ScrollView {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: selectItemInStackLayoutBar.bottom
        anchors.bottom: parent.bottom
        anchors.margins: 10

        Column {
            id: main_column_layout
            width: parent.width

            Text {
                height: 23
                text: qsTr("QOpenHD version: " + _qopenhd.version_string + " for " + Qt.platform.os)
            }
            Text {
                height: 23
                text: qsTr("FEATURES:  " + get_features_string())
            }
            Text {
                height: 23
                text: qsTr("Resolution: " + " Screen " + _qrenderstats.screen_width_height_str + " ADJ:" + _qrenderstats.display_width_height_str + " Window: " + _qrenderstats.window_width + "x" + _qrenderstats.window_height)
            }
            Text {
                height: 23
                text: qsTr("Art Horizon mavlink update rate:" + _fcMavlinkSystem.curr_update_rate_mavlink_message_attitude + " Hz")
            }
            Text {
                height: 23
                text: qsTr("Tele in " + _mavlinkTelemetry.telemetry_pps_in + " pps")
            }

            // Buttons for actions
            GridLayout {
                id: buttonGrid
                columns: 3
                rowSpacing: 10
                columnSpacing: 10

                Button {
                    width: 400
                    text: "Restart local OHD service"
                    onClicked: {
                        _qopenhd.restart_local_oenhd_service();
                    }
                }
                Button {
                    id: local_ip_button
                    text: "Show local IP"
                    onClicked: {
                        var text = _qopenhd.show_local_ip();
                        local_ip_button.text = text;
                    }
                }
                Button {
                    id: write_local_log
                    text: "Write GND log to SD"
                    onClicked: {
                        var text = _qopenhd.write_local_log();
                        write_local_log.text = text;
                    }
                }
                Button {
                    id: write_air_log
                    text: "Write AIR log to SD"
                    onClicked: {
                        var text = "Not implemented yet";
                        write_air_log.text = text;
                    }
                }
                Button {
                    text: "Set Tele rates"
                    onClicked: {
                        _mavlinkTelemetry.re_apply_rates();
                    }
                }
                Button {
                    visible: _qopenhd.is_linux()
                    text: "Start openhd"
                    onClicked: {
                        _qopenhd.sysctl_openhd(0);
                    }
                }
                Button {
                    visible: _qopenhd.is_linux()
                    text: "Stop openhd"
                    onClicked: {
                        _qopenhd.sysctl_openhd(1);
                    }
                }
                Button {
                    visible: _qopenhd.is_linux()
                    text: "Enable openhd"
                    onClicked: {
                        _qopenhd.sysctl_openhd(2);
                    }
                }
                Button {
                    visible: _qopenhd.is_linux()
                    text: "Disable openhd"
                    onClicked: {
                        _qopenhd.sysctl_openhd(3);
                    }
                }
                Button {
                    id: sdbut
                    text: "Self Destruct"
                    onClicked: {
                        sdbut.text = "just kidding";
                    }
                }
                Button {
                    font.capitalization: Font.MixedCase
                    text: qsTr("Restart QOpenHD")
                    onPressed: {
                        qopenhdservicedialoque.open_dialoque(0);
                    }
                }
                Button {
                    font.capitalization: Font.MixedCase
                    text: qsTr("Cancel QOpenHD")
                    onPressed: {
                        qopenhdservicedialoque.open_dialoque(1);
                    }
                }
            }

            // Terminal-like Scrollable Text Display
            Rectangle {
                id: terminalContainer
                visible: false
                width: parent.width
                height: 200
                color: "black" // Background color

                ScrollView {
                    id: terminalScrollView
                    anchors.fill: parent
                    ScrollBar.vertical.interactive: true
                    ScrollBar.vertical.policy: ScrollBar.AlwaysOn

                    TextArea {
                        id: terminalTextArea
                        readOnly: true
                        wrapMode: TextArea.WrapAtWordBoundaryOrAnywhere
                        text: ""
                        font.pixelSize: 14
                        color: "white" // Text color
                        background: Rectangle { color: "black" }
                    }
                }
            }

            Row {
                spacing: 10

                Button {
                    text: "Show Air Info"
                    onClicked: {
                        showSystemInfo(_ohdSystemAir, "Air");
                        terminalContainer.visible = true;
                    }
                }
                Button {
                    text: "Show Ground Info"
                    onClicked: {
                        showSystemInfo(_ohdSystemGround, "Ground");
                        terminalContainer.visible = true;
                    }
                }
                Button {
                    text: "Show Mavlink Info"
                    onClicked: {
                        showSystemInfo(_fcMavlinkSystem, "Mavlink");
                        terminalContainer.visible = true;
                    }
                }
                Button {
                    text: "\uf2ed"
                    onClicked: {
                        terminalTextArea.text = " \n";
                    }
                }
            }
        }
    }

    function showSystemInfo(systemObject, title) {
        if (systemObject) {
            terminalTextArea.text += title + " System Information:\n\n";
            for (var key in systemObject) {
                if (systemObject.hasOwnProperty(key) && typeof systemObject[key] !== "function") {
                    terminalTextArea.text += key + ": " + systemObject[key] + "\n";
                }
            }
        } else {
            terminalTextArea.text += title + " System Information unavailable.\n";
        }
    }


    QOpenHDServiceDialoque {
        id: qopenhdservicedialoque
    }
}
