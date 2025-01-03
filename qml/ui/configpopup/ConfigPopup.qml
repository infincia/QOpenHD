import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.12
import Qt.labs.settings 1.0

import OpenHD 1.0

import "connect"
import "credits"
import "dev"
import "log"
import "qopenhd_settings"
import "openhd_settings"
import "rc"
import "status"


// Contains the selector on the left and a stack view for the panels on the right
Rectangle {
    id: settings_form
    //width: 1280
    //height: 720
    anchors.fill: parent
    //width: parent.width * settings.screen_settings_overlay_size_percent / 100;
    //height: parent.height * settings.screen_settings_overlay_size_percent / 100;
    z: 4.0
    color: "transparent"

    // size of the elements in the left bar - e.g. what allows switching between all the tabs
    property int left_sidebar_elements_height: 46

    function openSettings() {
        settings_form.visible=true
        settings_form.focus=true;
        sidebar.focus=true
    }

    function close_all(){
        visible = false;
        focus=false;
        hudOverlayGrid.regain_focus();
    }

    function side_bar_regain_focus(){
        sidebar.focus = true;
    }

    function user_quidance_open_connect_screen(){
        openSettings();
        mainStackLayout.currentIndex=5;
    }

    function user_guidance_open_openhd_settings_find_air_unit_location(){
        openSettings();
        mainStackLayout.currentIndex=1;
        ohdSettingsPanel.user_quidance_animate_channel_scan();
    }

    function gain_focus(){
        sidebar.focus=true;
    }

    /*Keys.onPressed: (event)=> {
        console.log("ConfigPopup Key was pressed:"+event);
        if (event.key == Qt.Key_Return) {
            console.log("enter was pressed");
            event.accepted = true;
            close_all()
            //hudOverlayGrid.settingsButtonClicked();
            //settingsButton.focus=false;
        }
    }*/

    Rectangle {
        id: spacerTopSpacer
        width: 132
        visible: true
        anchors.left: parent.left
        anchors.leftMargin: -1
        anchors.bottom: parent.bottom
        anchors.bottomMargin: -1
        anchors.top: parent.top
        anchors.topMargin: -1
        color: "#fafafa"

        clip: true

        Column {
            width: parent.width
            anchors.top: parent.top

            // Close
            Item {
                height: 48
                width: parent.width

                Button{
                    id: closeButton
                    text: "\uf060"
                    font.family: "Font Awesome 5 Free"
                    font.pixelSize: 22
                    height: 35
                    width: 35
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter

                    background: Rectangle {
                        opacity: .5
                        radius: 5
                        //later this can be changed to focus
                        color: closeButton.hovered ? "grey" : "white" // I update background color by this
                    }
                    onClicked: {
                        close_all();
                    }
                }
            }
        }
    }

    // This is the settings row on the left. When an item is clicked, the stack layout index is changed which
    // means a different category is displayed.
    // NOTE: when changing the n of elements in the panel, remember to change the indices
    Rectangle {
        id: sidebar
        width: 132
        visible: true
        anchors.left: parent.left
        anchors.leftMargin: -1
        anchors.bottom: parent.bottom
        anchors.bottomMargin: -1
        anchors.top: parent.top
        anchors.topMargin: 48
        color: "#333c4c"
        radius: 0
        border.width: 1
        border.color: "#4c000000"

        clip: true

        Keys.onPressed: (event)=> {
                // console.log("Sidebar Key was pressed:"+event);
                var tmp_index=mainStackLayout.currentIndex;
                if(event.key==Qt.Key_Up){
                    tmp_index--;
                    if(tmp_index<0)tmp_index=0;
                    mainStackLayout.currentIndex=tmp_index;
                }else if (event.key == Qt.Key_Down) {
                    console.log("Down arrow");
                    tmp_index++;
                    if(tmp_index>=mainStackLayout.count)tmp_index=0;
                    mainStackLayout.currentIndex=tmp_index;
                }else if(event.key == Qt.Key_Left){
                    close_all()
                }else if(event.key == Qt.Key_Right){
                    // Right now we only have joystick navigation for the QUICK panel
                    /*if(mainStackLayout.currentIndex==0){
                         // hand over navigation to the panel
                         quickPanel.gain_focus();
                         return;
                     }*/
                    //mainStackLayout.childAt(mainStackLayout.currentIndex).focus=true
                    _qopenhd.show_toast("No joystick navigation for this panel");
                }
            }
        ScrollView{
            width: parent.width
            height: parent.height
            contentHeight: navigation_buttons_column.height
            Column {
                id: navigation_buttons_column
                width: parent.width
                anchors.top: parent.top

                //
                /*ConfigPopupSidebarButton{
                    id:  quick
                    m_icon_text: "\uf0fb"
                    m_description_text: "Quick"
                    m_selection_index: 0
                }*/

                // Status
                ConfigPopupSidebarButton{
                    id:  power
                    m_icon_text: "\uf21e" //"\uf011"
                    m_description_text: "Status"
                    m_selection_index: 0
                }

                // OpenHD OHD Settings
                ConfigPopupSidebarButton{
                    id:  openhd_button
                    m_icon_text: "\uf085"
                    m_description_text: "OpenHD"
                    m_selection_index: 1
                }

                // (QOpenHD Settings - AppSettingsPanel)
                // OSD
                ConfigPopupSidebarButton{
                    id:  qopenhd_button
                    m_icon_text: "\uf013"
                    m_description_text: "OSD"
                    m_selection_index: 2
                }

                // Log
                ConfigPopupSidebarButton{
                    id:  log_button
                    m_icon_text: "\uf0c9"
                    m_description_text: "Log"
                    m_selection_index: 3
                }

                // RC
                ConfigPopupSidebarButton{
                    id:  rc
                    m_icon_text: "\uf11b"
                    m_description_text: "RC"
                    m_selection_index: 4
                }

                // We only need the connect panel on android (external device)
                // On localhost, QOpenHD "automatically" connects due to udp localhost method
                ConfigPopupSidebarButton{
                    id:  connect_button
                    m_icon_text: "\uf6ff"
                    m_description_text: "Connect"
                    m_selection_index: 5
                }

                // Credits and copyright
                ConfigPopupSidebarButton{
                    id:  credits
                    m_icon_text: "\uf005"
                    m_description_text: "Credits"
                    m_selection_index: 6
                }

                // Developer stats
                ConfigPopupSidebarButton{
                    id:  developerstats
                    m_icon_text: "\uf0ad"
                    m_description_text: "DEV"
                    m_selection_index: 7
                }
            }
        }
    }




    StackLayout {
        id: mainStackLayout
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.left: sidebar.right
        anchors.leftMargin: 0
        anchors.top: parent.top
        anchors.topMargin: 0

        // default index
        currentIndex: 0

        PanelStatus {
            id: statusPanel
        }

         //this is "openhd" menu
        OHDSettingsPanel {
            id:  ohdSettingsPanel
        }

        AppSettingsPanel {
            id: appSettingsPanel
        }

        LogMessagesStatusView{
            id: logMessagesStatusView
        }

        RcInfoPanel {
            id: rcInfoPanel
        }

        ConnectPanel{
            id: connectPanel
        }

        Credits {
            id: creditspanel
        }

        AppDeveloperStatsPanel {
            id: appDeveloperStatsPanel
        }

    }
}

