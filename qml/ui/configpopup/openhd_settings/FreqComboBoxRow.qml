import QtQuick 2.0

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import QtQuick.Controls.Material 2.12
 

import Qt.labs.settings 1.0

import OpenHD 1.0

import "../../../ui" as Ui
import "../../elements"

Rectangle{
    property int m_background_color_type: 0

    property string m_main_text: "[] XXXXMhz"

    // 0: this row matches current openhd selected frequency
    // 1: otherwise
    property int m_selection_tpye: 0

    property bool m_is_2G: false
    property bool m_show_radar: false

    property int m_openhd_race_band: -1
    property int m_openhd_licensed_band: -1


    property int m_pollution_pps: -1


    color: "transparent"
    anchors.fill: parent
    anchors.margins: 6

    // Background
    Rectangle{
        anchors.fill: parent
        //color: "transparent"
        color: m_selection_tpye===0 ? "transparent" : "#c5c3c7" // light gray
        //border.color: "red"
        //color: "red"
    }
    // The acual row, contains text and icons
    RowLayout{
        //width:parent.width
        //height:parent.height
        anchors.fill: parent
        Text {
            Layout.alignment: Qt.AlignLeft
            Layout.preferredWidth: 110
            text: m_main_text
            color: "black"
            font: comboBoxFreq.font
        }
        Text{
            Layout.alignment: Qt.AlignLeft
            text: "2.4G"
            //color: value > 3000 ? "green" : "#ff8c00" //"orange"
            color: "#706F1D" // dark green
            visible: m_is_2G
            font.pixelSize: comboBoxFreq.font.pixelSize
        }
        Text { // Radar icon
            Layout.alignment: Qt.AlignLeft
            text: qsTr("\uf7c0");
            font.family: "Font Awesome 5 Free"
            color: "red"
            visible: m_show_radar
            font.pixelSize: comboBoxFreq.font.pixelSize
        }
        Item{
            Layout.fillWidth: true
            // filler
        }
        /*Text{ // smiley icon - indicates good channel
            Layout.alignment: Qt.AlignRight
            Layout.preferredWidth: 30
            text: qsTr("\uf585")
            visible: m_show_good_channel
            font.family: "Font Awesome 5 Free"
            color: "green"
        }*/
        Text{
            Layout.alignment: Qt.AlignLeft
            Layout.preferredWidth: 60
            text: "Default"
            font.family: "Font Awesome 5 Free"
            color: "green"
            visible: m_openhd_race_band==2
            font.pixelSize: comboBoxFreq.font.pixelSize
        }
        Text{ // recommended / openhd race band channel
            Layout.alignment: Qt.AlignRight
            Layout.preferredWidth: 60
            text: {
                qsTr("OHD "+m_openhd_race_band)
            }
            visible: m_openhd_race_band>=0
            font.family: "Font Awesome 5 Free"
            color: "green"
            font.pixelSize: comboBoxFreq.font.pixelSize
        }
        // Channel occupancy estimate
        Item{
            Layout.preferredWidth: parent.height
            Layout.preferredHeight: parent.height
            height: parent.height
            width: height
            Layout.alignment: Qt.AlignRight
            Rectangle{// Channel occupancy estimate
                height: parent.height * 2/3;
                width: height
                radius: width
                anchors.centerIn: parent
                color: {
                    if(m_pollution_pps<0){
                        return "gray"
                    }
                    if(m_pollution_pps<1)return "green";
                    if(m_pollution_pps<20) return "orange";
                    return "red";
                }
                border.width: 2
                border.color: "black"
            }
        }
    }
}
