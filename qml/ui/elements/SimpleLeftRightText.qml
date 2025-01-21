import QtQuick 2.12

Item {
    width: parent.width
    //height: 24
    height: 20

    property string m_left_text: "FILL ME"
    property string m_right_text: "FILL ME"

    property color m_right_text_color: "white"

    Text {
        text: m_left_text
        color: "white"
        font.bold: true
        height: parent.height
        font.pixelSize: detailPanelFontPixels
        anchors.left: parent.left
        verticalAlignment: Text.AlignVCenter
    }
    Text {
        text: m_right_text
        color: m_right_text_color;
        font.bold: false;
        height: parent.height
        font.pixelSize: detailPanelFontPixels;
        anchors.right: parent.right
        verticalAlignment: Text.AlignVCenter
    }
}
