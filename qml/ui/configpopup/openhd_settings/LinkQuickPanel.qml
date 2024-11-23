import QtQuick 2.0

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import QtQuick.Controls.Material 2.12


import Qt.labs.settings 1.0

import OpenHD 1.0

import "../../../ui" as Ui
import "../../elements"

// This is an extra screen for changing the frequency / channel width -
// They both need to match !
Rectangle{
    id: main_background
    width: parent.width
    height: parent.height
    //property color m_background_color: "#8cbfd7f3"
    //property color m_background_color: "#ADD8E6"
    property color m_background_color: "#8cbfd7f3"

    property int m_small_height: 50
    property int m_small_width: 120

    function user_quidance_animate_channel_scan(){
        console.log("User guidance animate channel scan");
        anim_find_air_unit.start()
    }

    // https://stackoverflow.com/questions/41991438/how-do-i-find-a-particular-listelement-inside-a-listmodel-in-qml
    // For the models above (model with value) try to find the index of the first  item where model[i].value===value
    function find_index(model,value){
        for(var i = 0; i < model.count; ++i) if (model.get(i).value===value) return i
        return -1
    }

    function fc_is_armed(){
        return _fcMavlinkSystem.armed
    }

    ListModel{
        id: frequencies_model
        ListElement {title: "Unknown"; value_frequency_mhz:-1}
    }

    function create_list_models_frequency(){
        frequencies_model.clear();
        const filter = filter_tab_bar.currentIndex;
        const frequencies_all=_frequencyHelper.get_frequencies(filter);
        for(var i=0;i<frequencies_all.length;i++){
            const frequency=frequencies_all[i];
            const text=_frequencyHelper.get_frequency_description(frequency)
            frequencies_model.append({title: text, value_frequency_mhz: frequency});
        }
    }

    function update_frequency_combobox(){
        create_list_models_frequency();
        comboBoxFreq.model=frequencies_model;
        if(_wbLinkSettingsHelper.curr_channel_mhz>0){
            var index=find_index(comboBoxFreq.model,_wbLinkSettingsHelper.curr_channel_mhz);
            if(index>=0){
                comboBoxFreq.currentIndex=index;
            }else{
                comboBoxFreq.currentIndex=0;
                console.log("Seems not to be a valid channel "+_wbLinkSettingsHelper.curr_channel_mhz)
            }
        }else{
            comboBoxFreq.currentIndex=-1;
        }
    }

    // We get notified every time we should re-build the model(s) and their current selection
    property int m_ui_rebuild_models : _wbLinkSettingsHelper.ui_rebuild_models
    onM_ui_rebuild_modelsChanged: {
        console.log(" onM_ui_rebuild_modelsChanged: "+_wbLinkSettingsHelper.ui_rebuild_models);
        function_rebuild_ui()
    }

    function function_rebuild_ui(){
        update_frequency_combobox();
        console.log("function_rebuild_ui:"+_wbLinkSettingsHelper.ui_rebuild_models);
        if(_wbLinkSettingsHelper.ui_rebuild_models<=0)return
        popup_analyze_channels.update();
    }

    //
    function close_all_dialoques(){
        popup_analyze_channels.close()
        popup_scan_channels.close();
        popup_change_tx_power.close();
        dialoqueFreqChangeGndOnly.close();
        dialoqueFreqChangeAirGnd.close();
        popup_enable_stbc_ldpc.close();
    }

    function get_text_stbc_ldpc(air){
        if(air){
            if(!_ohdSystemAir.is_alive)return "N/A";
            return ""+(_ohdSystemAir.wb_stbc_enabled ? "YES" : "NO")+"/"+(_ohdSystemAir.wb_lpdc_enabled ? "YES" : "NO");
        }
        if(!_ohdSystemGround.is_alive)return "N/A";
        return ""+(_ohdSystemGround.wb_stbc_enabled ? "YES" : "NO")+"/"+(_ohdSystemGround.wb_lpdc_enabled ? "YES" : "NO");
    }

    Component.onCompleted: {
        close_all_dialoques();
        create_list_models_frequency();
        update_frequency_combobox();
    }

    function get_text_wifi_tx_power(air){
        if(air){
            if(!_wifi_card_air.alive) return "N/A";
            return ""+_wifi_card_air.tx_power+" "+_wifi_card_air.tx_power_unit;
        }
        if(!_wifi_card_gnd0.alive) return "N/A";
        return ""+_wifi_card_gnd0.tx_power+" "+_wifi_card_gnd0.tx_power_unit;
    }

    ScrollView {
        id: main_scroll_view
        width: parent.width
        height: parent.height
        anchors.centerIn: parent
        contentHeight: main_column_layout.height
        //contentWidth: main_column_layout.width
        //ScrollBar.vertical.policy: ScrollBar.AlwaysOn
        ScrollBar.vertical.interactive: true
        visible: (!popup_analyze_channels.visible && !popup_enable_stbc_ldpc.visible && !popup_change_tx_power.visible && !popup_scan_channels.visible)
        clip: true

        Item {
            anchors.fill: parent

            Column{
                id: main_column_layout
                anchors.left: parent.left
                anchors.right: parent.right

                SettingsCategory{
                    m_description: "FREQUENCY / TOOLKIT"
                    spacing: 1
                    m_hide_elements: false;

                    Row{
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 8
                        ComboBox {
                            width: elementComboBoxWidth
                            id: comboBoxFreq
                            model: frequencies_model
                            textRole: "title"
                            implicitWidth:  elementComboBoxWidth
                            currentIndex: 0
                            delegate: ItemDelegate {
                                width: comboBoxFreq.width
                                contentItem: FreqComboBoxRow{
                                    m_main_text: title
                                    m_selection_tpye: (value_frequency_mhz===_wbLinkSettingsHelper.curr_channel_mhz) ? 1 : 0
                                    m_is_2G: value_frequency_mhz < 3000 && value_frequency_mhz > 100
                                    m_show_radar: _frequencyHelper.get_frequency_radar(value_frequency_mhz)
                                    m_openhd_race_band: _frequencyHelper.get_frequency_openhd_race_band(value_frequency_mhz)
                                    m_openhd_licensed_band: _frequencyHelper.get_frequency_openhd_licensed_band(value_frequency_mhz)
                                    m_pollution_pps: _pollutionHelper.pollution_get_last_scan_pollution_for_frequency(value_frequency_mhz)
                                }
                                highlighted: comboBoxFreq.highlightedIndex === index
                            }
                            displayText: {
                                if(!_ohdSystemGround.is_alive)return "GND NOT ALIVE";
                                if(_ohdSystemGround.wb_gnd_operating_mode==1){
                                    return "SCANNING";
                                }
                                if(_ohdSystemGround.wb_gnd_operating_mode==2){
                                    return "ANALYZING";
                                }
                                if(!_ohdSystemAir.is_alive){
                                    return _wbLinkSettingsHelper.curr_channel_mhz+"@"+"N/A"+" Mhz (NO AIR)";
                                }
                                return _wbLinkSettingsHelper.curr_channel_mhz+"@"+_wbLinkSettingsHelper.curr_channel_width_mhz+" Mhz";
                            }
                            onActivated: {
                                console.log("onActivated:"+currentIndex);
                                if(currentIndex<0)return;
                                const frequency_mhz=comboBoxFreq.model.get(currentIndex).value_frequency_mhz
                                console.log("Selected frequency: "+frequency_mhz);
                                if(!_frequencyHelper.hw_supports_frequency_threadsafe(frequency_mhz)){
                                    _qopenhd.show_toast("your HW does not support "+frequency_mhz+" Mhz");
                                    return;
                                }
                                if(_wbLinkSettingsHelper.curr_channel_mhz==frequency_mhz){
                                    console.log("Already at frequency "+frequency_mhz);
                                    return;
                                }
                                if(!_ohdSystemAir.is_alive){
                                    var error_message_not_alive="AIR Unit not alive -"
                                    dialoqueFreqChangeGndOnly.initialize_and_show_frequency(frequency_mhz,error_message_not_alive);
                                    return;
                                }
                                // Change the freuquency
                                dialoqueFreqChangeAirGnd.initialize_and_show_frequency(frequency_mhz);
                            }
                            enabled: _ohdSystemGround.is_alive && _ohdSystemGround.wb_gnd_operating_mode==0;
                        }
                        TabBar {
                            id: filter_tab_bar
                            width: 350
                            currentIndex: settings.qopenhd_frequency_filter_selection
                            onCurrentIndexChanged: {
                                if (currentIndex != settings.qopenhd_frequency_filter_selection) {
                                    settings.qopenhd_frequency_filter_selection = currentIndex;
                                    console.log("Tab changed to index:", currentIndex);
                                    function_rebuild_ui();
                                }
                            }
                            TabButton {
                                text: "OpenHD"
                                font.capitalization: Font.MixedCase
                            }
                            TabButton {
                                text: "2.4G"
                                enabled: {
                                    if (_ohdSystemAir.is_alive && _ohdSystemAir.ohd_platform_type == 30) {
                                        // X20 does not support 2.4G
                                        return false;
                                    }
                                    return true;
                                }
                            }
                            TabButton {
                                text: "5.8G"
                            }
                            enabled: comboBoxFreq.enabled
                            TabButton {
                                text: "Custom"
                                font.capitalization: Font.MixedCase
                                visible: settings.dev_show_5180mhz_lowband
                            }
                        }

                        /*ButtonIconInfo2{
                    Layout.alignment: Qt.AlignRight
                    visible:false
                    onClicked: {
                        var text="Please select a channel / frequency free of noise and interference. The current loss / pollution stats below can help,"+
                                "as well as the analyze channels feature or a frequency analyzer on your phone. DEF: Show OpenHD standard channels [1-7] only - they "+
                                " often are free of wifi pollution and should be used."
                        _messageBoxInstance.set_text_and_show(text)
                    }
                }*/
                    }

                    Row{
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 8
                        Button{
                            width:  150
                            id: b_find_air_unit
                            text: "SCAN"
                            enabled: _ohdSystemGround.is_alive
                            onClicked: {
                                close_all_dialoques();
                                popup_scan_channels.open()
                            }
                            SequentialAnimation {
                                running: false
                                loops: 4
                                id: anim_find_air_unit
                                // Expand the button
                                PropertyAnimation {
                                    target: b_find_air_unit
                                    property: "scale"
                                    to: 1.5
                                    duration: 200
                                    easing.type: Easing.InOutQuad
                                }
                                // Shrink back to normal
                                PropertyAnimation {
                                    target: b_find_air_unit
                                    property: "scale"
                                    to: 1.0
                                    duration: 200
                                    easing.type: Easing.InOutQuad
                                }
                            }
                        }
                        Button{
                            width:  150
                            text: "ANALYZE"
                            enabled: _ohdSystemGround.is_alive
                            onClicked: {
                                close_all_dialoques();
                                popup_analyze_channels.open()
                            }
                        }
                    }
                    Row{
                        anchors.horizontalCenter: parent.horizontalCenter
                        Text{
                            width:  m_small_width
                            height: m_small_height
                            text:{
                                "LOSS:\n"+_ohdSystemGround.curr_rx_packet_loss_perc+"%"
                            }
                            color: _ohdSystemGround.curr_rx_packet_loss_perc > 5 ? "red" : "black"
                            verticalAlignment: Qt.AlignVCenter
                            horizontalAlignment: Qt.AlignHCenter
                            font.bold: false
                            font.pixelSize: settings.qopenhd_general_font_pixel_size
                        }
                        Text{
                            width:  m_small_width
                            height: m_small_height
                            text: {
                                return "POLLUTION:\n"+_ohdSystemGround.wb_link_curr_foreign_pps+"pps"
                            }
                            color: _ohdSystemGround.wb_link_curr_foreign_pps > 20 ? "red" : "black"
                            verticalAlignment: Qt.AlignVCenter
                            horizontalAlignment: Qt.AlignHCenter
                            font.bold: false
                            font.pixelSize: settings.qopenhd_general_font_pixel_size
                        }
                        Text{
                            width:  m_small_width
                            height: m_small_height
                            text: {
                                return "TX ERRORS:\n"+_cameraStreamModelPrimary.total_n_tx_dropped_frames
                            }
                            color: _ohdSystemGround.wb_link_curr_foreign_pps > 20 ? "red" : "black"
                            verticalAlignment: Qt.AlignVCenter
                            horizontalAlignment: Qt.AlignHCenter
                            font.bold: false
                            font.pixelSize: settings.qopenhd_general_font_pixel_size
                        }
                        /*Text{
                            width:  m_small_width
                            height: m_small_height
                            text: {
                                var ret="THROTTLE:\n";
                                if(_ohdSystemAir.curr_n_rate_adjustments<=-1){
                                    ret+="N/A";
                                }else if(_ohdSystemAir.curr_n_rate_adjustments==0){
                                    ret+="NONE";
                                }else{
                                    ret+=("ACTIVE:"+_ohdSystemAir.curr_n_rate_adjustments+"x");
                                }
                                return ret;
                            }
                            color: _ohdSystemAir.curr_n_rate_adjustments > 0 ? "red" : "black"
                            verticalAlignment: Qt.AlignVCenter
                            horizontalAlignment: Qt.AlignHCenter
                            font.bold: false
                            font.pixelSize: settings.qopenhd_general_font_pixel_size
                        }*/
                    }
                }

                SettingsCategory{
                    m_description: "TX POWER";
                    m_hide_elements: false;
                    Row{
                        anchors.horizontalCenter: parent.horizontalCenter
                        Text{
                            width:  m_small_width
                            height: m_small_height
                            text: "AIR:\n "+get_text_wifi_tx_power(true)
                            verticalAlignment: Qt.AlignVCenter
                            horizontalAlignment: Qt.AlignHCenter
                            font.bold: false
                            font.pixelSize: settings.qopenhd_general_font_pixel_size
                        }
                        Button{
                            text: "EDIT"
                            enabled: _ohdSystemAir.is_alive
                            //enabled: true
                            onClicked: {
                                close_all_dialoques();
                                popup_change_tx_power.m_is_air=true;
                                popup_change_tx_power.open()
                            }
                        }
                        Text{
                            width:  m_small_width
                            height: m_small_height
                            text: "GND:\n"+get_text_wifi_tx_power(false)
                            verticalAlignment: Qt.AlignVCenter
                            horizontalAlignment: Qt.AlignHCenter
                            font.bold: false
                            font.pixelSize: settings.qopenhd_general_font_pixel_size
                        }
                        Button{
                            text: "EDIT"
                            enabled: _ohdSystemGround.is_alive
                            //enabled: true
                            onClicked: {
                                close_all_dialoques();
                                popup_change_tx_power.m_is_air=false;
                                popup_change_tx_power.open()
                            }
                        }

                    }
                }

                SettingsCategory{
                    m_description: "ADVANCED (STBC,LDPC)"
                    m_hide_elements: false;
                    Row{
                        anchors.horizontalCenter: parent.horizontalCenter
                        Text{
                            width:  m_small_width
                            height: m_small_height
                            text: "AIR:\n"+get_text_stbc_ldpc(true);
                            verticalAlignment: Qt.AlignVCenter
                            horizontalAlignment: Qt.AlignHCenter
                            font.bold: false
                            font.pixelSize: settings.qopenhd_general_font_pixel_size
                        }
                        Text{
                            width:  m_small_width
                            height: m_small_height
                            text: "GND:\n"+get_text_stbc_ldpc(false);
                            verticalAlignment: Qt.AlignVCenter
                            horizontalAlignment: Qt.AlignHCenter
                            font.bold: false
                            font.pixelSize: settings.qopenhd_general_font_pixel_size
                        }
                        Button{
                            text: "EDIT";
                            //enabled: true
                            enabled: _ohdSystemAir.is_alive && _ohdSystemGround.is_alive && (_wbLinkSettingsHelper.ui_rebuild_models>=0) &&
                                     (_ohdSystemGround.wb_stbc_enabled!=true || _ohdSystemGround.wb_lpdc_enabled!=true || _ohdSystemAir.wb_stbc_enabled!=true || _ohdSystemAir.wb_lpdc_enabled!=true);
                            onClicked: {
                                close_all_dialoques();
                                popup_enable_stbc_ldpc.open()
                            }
                        }
                    }
                }
            }
        }
    }
    PopupAnalyzeChannels{
        id: popup_analyze_channels
    }

    PopupScanChannels{
        id: popup_scan_channels
    }

    PopupTxPowerEditor{
        id: popup_change_tx_power
    }
    PopupEnableSTBCLDPC{
        id: popup_enable_stbc_ldpc
    }

    DialoqueFreqChangeGndOnly{
        id: dialoqueFreqChangeGndOnly
    }
    DialoqueFreqChangeAirGnd{
        id: dialoqueFreqChangeAirGnd
    }
}

