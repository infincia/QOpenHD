import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import QtQuick.Controls.Material 2.12

import OpenHD 1.0


// Element for showing the user a (big) settings text that should be clicked to be edited
Button{
    id: button
    width:180
    height: 64
    font.bold: true
    font.pixelSize: 14

    background: Rectangle {
        anchors.right: parent.right
        anchors.rightMargin: 1
        anchors.left: parent.left
        anchors.leftMargin: 1
        opacity: 1.0
        color: button.hovered ? "lightgrey" : "transparent"
    }

}
