import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import QtQuick.Controls.Material 2.12

import Qt.labs.settings 1.0

import OpenHD 1.0

import "../../../ui" as Ui
import "../../elements"

// Hard-coded layouting:
// 1/3 for left text (For example VERSION)
// 2/3 for value text
// value text can be color coded red,
// in which case it is clickable
// The general idea behind this element is to be green / pretty when everything is okay
// and red if something is wrong (in which case the user should click on it for more info)
Item {
    //Layout.fillWidth: true
    //Layout.minimumWidth: 50
    //Layout.minimumHeight: 30
    //
    //Layout.preferredWidth: 250
    //Layout.preferredHeight: 30
    //Layout.minimumWidth: 250
    width: parent.width
    height: 48

    id: main_item
    property string m_left_text: "LEFT"

    property string m_right_text: "RIFGHT"

    property bool m_has_error: false

    property color m_right_text_color: "green"
    property color m_right_text_color_error: "red"

    property string m_error_text: "NONE"

    property bool m_error_view: false

    // For debugging
    /*Rectangle{
        implicitWidth: main_item.width
        implicitHeight: main_item.height
        color: "blue"
        border.width: 3
        border.color: "black"
    }*/
    /*Rectangle{
        implicitWidth: main_item.width
        implicitHeight: main_item.height
        color: "transparent"
        border.width: 3
        border.color: "red"
    }*/
    // Actual item,centered
    Item{
        width: parent.width > 400 ? 400 : parent.width
        height: main_item.height
        anchors.centerIn: parent

        Text{
            id: left_part
            width: parent.width*0.5
            height: parent.height
            anchors.left: parent.left
            anchors.top: parent.top
            text: m_left_text
            verticalAlignment: Qt.AlignVCenter
            horizontalAlignment: Qt.AlignRight
            font.bold: true
            font.pixelSize: 18
            clip: true
            elide: Text.ElideRight
            padding: 4
        }

        Text{
            id: right_part
            width: parent.width-left_part.width
            height: parent.height
            anchors.left: left_part.right
            anchors.top: left_part.top
            text: m_right_text
            verticalAlignment: Qt.AlignVCenter
            horizontalAlignment: Qt.AlignLeft
            color: m_has_error ? m_right_text_color_error : m_right_text_color
            visible: !right_part_button.visible
            font.pixelSize: 18
            clip: true
            elide: Text.ElideRight
            padding: 4
        }
        MouseArea {
            enabled: m_has_error && !m_error_view
            anchors.fill: parent
            onClicked: {
                _messageBoxInstance.set_text_and_show(m_error_text)
            }
        }
        Button{
            id: right_part_button
            width: parent.width-left_part.width
            height: parent.height
            anchors.left: left_part.right
            anchors.leftMargin: -10
            anchors.top: left_part.top
            text: m_right_text
            background: Rectangle {
                        anchors.right: parent.right
                        anchors.rightMargin: 5
                        anchors.left: parent.left
                        anchors.leftMargin: 5
                        opacity: .3
                        color: right_part_button.hovered ? "lightgrey" : "transparent"
                    }
            //Material.accent: Material.Red
            //highlighted: true
            visible: m_has_error && m_error_view
            onClicked: {
                _messageBoxInstance.set_text_and_show(m_error_text)
            }
            //palette.buttonText: "red"
            Material.foreground: "red"
        }
    }

}
