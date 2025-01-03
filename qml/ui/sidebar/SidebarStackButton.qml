import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import QtQuick.Shapes 1.0
import QtQuick.Controls.Material 2.0

import Qt.labs.settings 1.0

import OpenHD 1.0

import "../elements"

//
// Joystick controllable (override the events)
// (But also clickable)
//
Item {
    id: sidebar_stack_button
    height: secondaryUiHeight / 8 //number of items
    width: height

    property string override_text: "OVERRIDE ME"
    property string override_tag: "OVERRIDE ME"
    property int override_index: -100;


    function takeover_control(){
        focus=true;
    }


    // Background
    Rectangle{
        width: parent.width
        height: parent.height
        color: (mouseArea.containsMouse || m_stack_index==override_index) ? highlightColor : mainDarkColor
        border.color: "white"
        border.width: sidebar_stack_button.focus ? 3 : 0;
    }
    // Icon
    Text {
        anchors.fill: parent
        text: override_text
        font.pixelSize: secondaryUiHeight / 16
        opacity: 1.0
        font.family: "Font Awesome 5 Free"
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: {
            console.log("Item clicked: " + override_tag)
            sidebar.update_stack_index(override_index)
        }
    }

    Keys.onPressed: (event)=> {
                        // console.log("SidebarStackButton"+override_tag+" key was pressed:"+event);
                        if(event.key === Qt.Key_Up){
                            m_stack_index--;
                            if(m_stack_index<-1){
                                m_stack_index=-1;
                            }
                            sidebar.handover_joystick_control_to_button(m_stack_index);
                            if(m_stack_index==-1){
                                sidebar.close();
                            }
                            event.accepted=true;
                        }else if(event.key === Qt.Key_Down){
                            m_stack_index++;
                            if(m_stack_index>6){
                                m_stack_index=6;
                            }
                            sidebar.handover_joystick_control_to_button(m_stack_index);
                            event.accepted=true;
                        }else if(event.key==Qt.Key_Right && focus){
                            sidebar.handover_joystick_control_to_panel(sidebar.m_stack_index);
                            event.accepted=true;
                        }
                    }
}
