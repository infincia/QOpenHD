import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

import Qt.labs.settings 1.0

import OpenHD 1.0

import "../../ui" as Ui
import "../elements"

//
// Base for an UI element that has the following properties:
// left / right: Increment / Decrement (model) value (If increment / decrement is available)
// top / bottom : Go to next / previous element
// MIDDLE: Show the current value
//
// It looks like this:
// |-------------------------------|
// |             TITLE             |
// | {ARROW L} {CONTENT} {ARROW R} |
// |------------------------------ |
//
// Joystick navigatable (focus) but also supports touch.
Item{
    id: base_joy_edit_element
    width: 320
    height: 50+30

    property string m_title: "FILL ME"

    property bool m_is_selected: false

    property string m_displayed_value: "VALUE T"

    property bool m_is_enabled: true

    // Set to true if the "select element left" button should be activated
    property bool m_button_left_activated: false
     // Set to true if the "select element right" button should be activated
    property bool  m_button_right_activated: false

    // Allows the parent to override the text color
    // Right now only used by the change frequency element
    property bool override_show_red_text: false

    // Emitted if the button left is clicked
    signal choice_left();
    // Emitted if the button right is clicked
    signal choice_right();

    // Emitted if the button up is clicked (navigate to previous eelement)
    signal goto_previous();
    signal goto_next();

    Keys.onPressed: (event)=> {
                        // console.log("BaseJoyElement"+m_title+" key was pressed:"+event);
                        if(event.key == Qt.Key_Left){
                           if(m_button_left_activated){
                                choice_left()
                            }else{
                                _qopenhd.show_toast("NOT AVAILABLE");
                            }
                            event.accepted=true;
                        }else if(event.key == Qt.Key_Right){
                            if(m_button_right_activated){
                                 choice_right()
                             }else{
                                 _qopenhd.show_toast("NOT AVAILABLE");
                             }
                            event.accepted=true;
                        }else if(event.key==Qt.Key_Up){
                            goto_previous()
                            event.accepted=true;
                        }else if(event.key==Qt.Key_Down){
                            goto_next();
                            event.accepted=true;
                        }
                    }

    function takeover_control(){
        focus=true;
    }

    Rectangle{
        width: parent.width
        height:parent.height
        border.color: "white"
        border.width: base_joy_edit_element.focus ? 3 : 0;
        color: "#333c4c"
        opacity: base_joy_edit_element.focus ? 1.0 : 0.3;
    }

    Text {
        id: title_str
        text: qsTr(m_title)
        width: parent.width
        height: 30
        verticalAlignment: Qt.AlignVCenter
        horizontalAlignment: Qt.AlignHCenter
        font.pixelSize: 18
        anchors.top: parent.top
        color: "white"
    }

    Item{
        id: middle_element_holder
        width: parent.width
        height: 50
        anchors.top: title_str.bottom
        Rectangle{
            id: main_selector
            color: "transparent"
            width: parent.width /2;
            height: parent.height
            //border.color: "white"
            //border.width: 2
            anchors.centerIn: parent
        //Item{
            //id: main_selector
            //width: parent.width /2;
            //height: parent.height
            //anchors.centerIn: parent
            Text{
                id: middle_element
                width: parent.width
                height: parent.height
                text: m_displayed_value
                verticalAlignment: Qt.AlignVCenter
                horizontalAlignment: Qt.AlignHCenter
                font.pixelSize: 15
                color: override_show_red_text? "red" :  "white"
            }
        }
        TriangleButton{
            m_point_right: false
            id: button_left
            anchors.right:  main_selector.left
            height: parent.height
            width: height
            onClicked: choice_left()
            enabled: m_button_left_activated
            m_is_enabled: enabled
        }

        TriangleButton{
            m_point_right: true
            id: button_right
            anchors.left:  main_selector.right
            height: parent.height
            width: height
            onClicked: choice_right()
            enabled: m_button_right_activated
            m_is_enabled: enabled
        }
    }

    Rectangle{
        width: parent.width
        height: parent.height
        color: "gray"
        opacity: 0.7
        visible: !m_is_enabled
    }

}
