import QtQuick 2.12

Rectangle{
    id: choiceSelector
    width: 200
    height: secondaryUiHeight - (secondaryUiHeight/8)
    //color: "green"
    color: "transparent"
    visible: false

    // Index of the element that is selected when the selector is opened (current value)
    property int m_initial_index: -1
    // Changed as the user navigates around
    property int m_current_user_selected_index: -1;
    // We are only 'clickable' if no joystick usage
    property bool m_allow_clickable: false

    property var m_parent

    function open_choices(x_model,current_value,parent){
        // First, find the current selection inside the model
        availableChociesModel.clear();
        var current_value_index=0
        var current_value_found_in_model=false;
        for(var i=0;i<x_model.count;i++){
            const value=x_model.get(i).value;
            const verbose=x_model.get(i).verbose;
            availableChociesModel.insert(i,{'value': (""+value),'verbose': verbose});
            if(!current_value_found_in_model && value==current_value){
                current_value_index=i;
                current_value_found_in_model=true;
            }
        }
        if(!current_value_found_in_model){
            // If the current value is not 'known' to us (not inside the model) we add an extra element
            // to the model containing exactly this value (and a dummy description)
            console.log("Current value not found in model");
            const value=current_value;
            const verbose="{"+value+"}\n"+"custom";
            console.log(value+""+verbose)
            availableChociesModel.insert(0,{'value': value,'verbose': verbose});
            current_value_index=0;
        }
        m_initial_index=current_value_index;
        m_current_user_selected_index=m_initial_index;
        visible=true;
        focus=true;
        m_parent=parent
        const ui_required_height=availableChociesModel.count*73;
        availableChociesListView.height=ui_required_height;
    }

    function set_clickable(clickable){
        m_allow_clickable=clickable;
    }

    // Calls the parent's update method if there is any change and closes
    function close_choices(){
        if(m_current_user_selected_index!=m_initial_index){
            const value_old=availableChociesModel.get(m_initial_index).value;
            const value_new=availableChociesModel.get(m_current_user_selected_index).value;
            console.log("Changed from ["+m_initial_index+":"+value_old+"] to ["+m_current_user_selected_index+":"+value_new+"]");
            m_parent.user_selected_value(value_new);
        }
        m_current_user_selected_index=-1;
        m_initial_index=-1;
        visible=false;
        focus=false;
        m_parent.takeover_control();
    }

    // Discards a change selected by the user (if there is any) and closes
    function discard_and_close(){
        m_current_user_selected_index=-1;
        m_initial_index=-1;
        visible=false;
        focus=false;
    }


    Keys.onPressed: (event)=> {
                        // console.log("ChoiceSelector key was pressed:"+event);
                        if(event.key == Qt.Key_Left){
                            // Either close immediately or go back to the initial choice (don't save)
                            if(m_current_user_selected_index==m_initial_index){
                                close_choices();
                            }else{
                                m_current_user_selected_index=m_initial_index;
                            }
                            event.accepted=true;
                        }else if(event.key == Qt.Key_Right){
                            // Save and close or do nothing
                            if(m_current_user_selected_index==m_initial_index){
                                // Do nothing
                            }else{
                                close_choices();
                            }
                            event.accepted=true;
                        }else if(event.key==Qt.Key_Up){
                            if(m_current_user_selected_index>0){
                                m_current_user_selected_index--;
                            }else{
                                // Out of bounds
                            }
                            event.accepted=true;
                        }else if(event.key==Qt.Key_Down){
                            if(m_current_user_selected_index+1<availableChociesModel.count){
                                m_current_user_selected_index++;
                            }else{
                                // Out of bounds
                            }
                            event.accepted=true;
                        }else if(event.key==Qt.Key_Enter || event.key==Qt.Key_Return){
                            close_choices();
                            event.accepted=true;
                        }
                    }


    ListModel {
        id: availableChociesModel
        //ListElement { value:0; verbose:"0 def"}
        //ListElement { value:1; verbose:"1 def"}
        //ListElement { value:2; verbose:"2 def"}
        //ListElement { value:3; verbose:"3 def"}
    }

    Component {
        id: availableChociesDelegate
        Item {
            width: 200
            height: 70
            id: choice
            // Background
            Rectangle{
                anchors.fill: parent
                //color: index==m_initial_index ? "yellow" : "orange"
                color: "#333c4c"
                opacity: index==m_initial_index ? 1.0 : 0.7
            }
            Text {
                text: verbose
                anchors.fill: parent
                verticalAlignment: Qt.AlignVCenter
                horizontalAlignment: Qt.AlignHCenter
                color: "white"
                font.pixelSize: 15
            }
            // Outline current value
            Rectangle{
                anchors.fill: parent
                color: "transparent"
                border.color: "white"
                border.width: index==m_initial_index ? 2 : 0;
            }
            // Outline current selection by user
            Rectangle{
                anchors.fill: parent
                color: "transparent"
                border.color: "white"
                border.width: (index==m_current_user_selected_index) ? 3 : 0
            }
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    if(m_allow_clickable){
                        m_current_user_selected_index=index;
                        close_choices();
                    }
                }
            }
        }
    }

    Rectangle{
        implicitWidth: availableChociesListView.width
        implicitHeight: availableChociesListView.height
        anchors.left: availableChociesListView.left
        anchors.top: availableChociesListView.top
        color: secondaryUiColor
        opacity: secondaryUiOpacity
    }

    ListView {
        id: availableChociesListView
        model: availableChociesModel
        delegate: availableChociesDelegate
        anchors.top: parent.top
        anchors.left: parent.left
        width: parent.width
        height: parent.height*2
        //clip: false
        spacing: 3
    }
}
