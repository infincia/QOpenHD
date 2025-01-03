import QtQuick 2.0


Item{
    id: go_back_element
    width: parent.width
    height: 50

    function close_and_go_back(){
        // Go back to the
         //settings_form.gain_focus();
         quickPanelX.visible=false;
         hudOverlayGrid.regain_focus();
    }

    Rectangle{
        width: parent.width
        height: parent.height
        border.color: "black"
        border.width: go_back_element.focus ? 3 : 0;
        color: "#333c4c"
        opacity: go_back_element.focus ? 1.0 : 0.3;
    }
    Text{
         text: "BACK"
         color: "white"
         verticalAlignment: Qt.AlignVCenter
         horizontalAlignment: Qt.AlignHCenter
         anchors.fill: parent
    }
    MouseArea{
        anchors.fill: parent
        onClicked: {
            close_and_go_back()
        }
    }

    Keys.onPressed: (event)=> {
                        // console.log("Go back element key was pressed:"+event);
                        if(event.key == Qt.Key_Left || event.key == Qt.Key_Right || event.key == Qt.Key_Return){
                           close_and_go_back()
                        }
                    }
}


