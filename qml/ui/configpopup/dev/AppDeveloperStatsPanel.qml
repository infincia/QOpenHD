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

            // Example information text
            Text {
                text: qsTr("QOpenHD version: 1.0.0 for Linux")
                font.pixelSize: 14
            }
            Text {
                text: qsTr("Features: AVCODEC (Enabled), MMAL (Enabled), GStreamer QMLGLSink (Disabled)")
                font.pixelSize: 14
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
                        _qopenhd.restart_local_oenhd_service()
                    }
                }
                Button {
                    id: local_ip_button
                    text: "Show local IP"
                    onClicked: {
                        var text = _qopenhd.show_local_ip()
                        local_ip_button.text = text
                    }
                }
                Button {
                    id: write_local_log
                    text: "Write GND log to SD"
                    onClicked: {
                        var text = _qopenhd.write_local_log()
                        write_local_log.text = text
                    }
                }
                Button {
                    id: write_air_log
                    text: "Write AIR log to SD"
                    onClicked: {
                        var text = "Not implemented yet"
                        write_air_log.text = text
                    }
                }
                Button {
                    text: "Set Tele rates"
                    onClicked: {
                        _mavlinkTelemetry.re_apply_rates()
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
                    id:sdbut
                    text: "Self Distruct"
                    onClicked: {
                        sdbut.text="just kidding";
                    }
                }
                Button {
                    font.capitalization: Font.MixedCase
                    text: qsTr("Restart QOpenHD")
                    onPressed: {
                        qopenhdservicedialoque.open_dialoque(0)
                    }
                }
                Button {
                    font.capitalization: Font.MixedCase
                    text: qsTr("Cancel QOpenHD")
                    onPressed: {
                        qopenhdservicedialoque.open_dialoque(1)
                    }
                }
            }

            // Terminal-like Scrollable Text Display
            Rectangle {
                id: terminalContainer
                width: parent.width
                height: 200
                color: "black" // Background color
                border.color: "gray"
                border.width: 1

                ScrollView {
                    id: terminalScrollView
                    anchors.fill: parent
                    ScrollBar.vertical.interactive: true

                    TextArea {
                        id: terminalTextArea
                        readOnly: true
                        wrapMode: TextArea.WrapAtWordBoundaryOrAnywhere
                        text: "Initializing...\n" +
                              "Feature AVCODEC: Enabled\n" +
                              "Feature MMAL: Enabled\n" +
                              "GStreamer QMLGLSink: Disabled\n" +
                              "Mavlink update rate: 50Hz\n" +
                              "Local IP: 192.168.1.1\n" +
                              "Logs updating...\n"
                        font.pixelSize: 14
                        color: "white" // Text color
                        background: Rectangle { color: "black" } // Ensures full black background
                    }
                }
            }
        }
    }
}
