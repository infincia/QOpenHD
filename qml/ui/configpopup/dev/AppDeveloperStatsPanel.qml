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
                    text: "Restart local OHD service"
                    onClicked: {
                        _qopenhd.restart_local_oenhd_service()
                    }
                }
                Button {
                    text: "Show local IP"
                    onClicked: {
                        console.log("Local IP: 192.168.1.1")
                    }
                }
                Button {
                    text: "Write GND log to SD"
                    onClicked: {
                        console.log("GND log written")
                    }
                }
                Button {
                    text: "Write AIR log to SD"
                    onClicked: {
                        console.log("AIR log written")
                    }
                }
                Button {
                    text: "Set Tele rates"
                    onClicked: {
                        console.log("Telemetry rates updated")
                    }
                }
                Button {
                    text: "Start openhd"
                    visible: _qopenhd.is_linux()
                    onClicked: {
                        console.log("OpenHD started")
                    }
                }
                Button {
                    text: "Stop openhd"
                    visible: _qopenhd.is_linux()
                    onClicked: {
                        console.log("OpenHD stopped")
                    }
                }
                Button {
                    text: "Enable openhd"
                    visible: _qopenhd.is_linux()
                    onClicked: {
                        console.log("OpenHD enabled")
                    }
                }
                Button {
                    text: "Disable openhd"
                    visible: _qopenhd.is_linux()
                    onClicked: {
                        console.log("OpenHD disabled")
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
