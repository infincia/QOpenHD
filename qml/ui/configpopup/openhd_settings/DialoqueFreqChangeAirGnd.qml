import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import QtQuick.Controls.Material 2.12

import "../../elements"

// Dialoque that is shown to the user when the FC is armed and he wants to change frequency
// Once the user clicks the warning away, proceed as if FC is not armed
Card {
    id: dialoqueFreqChangeAirGnd
    width: 360
    height: 340
    z: 5.0
    anchors.centerIn: parent
    cardName: get_card_title_string()
    cardNameColor: "black"
    visible: false

    property int m_wanted_frequency: -1

    property bool m_fc_is_armed: true

    function close(){
        visible=false;
        enabled=false;
    }

    function initialize_and_show_frequency(frequency){
        m_wanted_frequency=frequency
        visible=true;
        enabled=true;
        // Show warning if
        if(_fcMavlinkSystem.is_alive && _fcMavlinkSystem.armed){
            m_fc_is_armed=true;
        }else{
            m_fc_is_armed=false;
        }
    }

    function get_card_title_string(){
        return "Frequency "+m_wanted_frequency+"Mhz"
    }

    function get_card_body_string(){
        const channel_nr=_frequencyHelper.get_frequency_channel_nr(m_wanted_frequency);
        const channel_nr_openhd=_frequencyHelper.get_frequency_openhd_race_band(m_wanted_frequency);
        const channel_nr_openhd_licensed=_frequencyHelper.get_frequency_openhd_licensed_band(m_wanted_frequency);
        return "Set AIR and GROUND to CHANNEL ["+channel_nr+"]\n"+
                "("+m_wanted_frequency+" Mhz) ?";

    }

    cardBody: Item{
        height: 200
        width: 320

        Text{
            id: description
            width: parent.width
            height: parent.height /2;
            leftPadding: 12
            rightPadding: 12
            wrapMode: Text.WordWrap
            text: get_card_body_string();
        }
        Text{
            anchors.top: description.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            leftPadding: 12
            rightPadding: 12
            wrapMode: Text.WordWrap
            text: "WARNING !\n CHANGING FREQUENCY WHILE ARMED IS NOT RECOMMENDED !";
            color: "red"
            visible: m_fc_is_armed
        }
    }
    hasFooter: true
    cardFooter: Item {
        anchors.fill: parent
        RowLayout{
            anchors.fill: parent

            Button{
                Layout.preferredWidth: 150
                text: "CANCEL"
                onPressed: {
                    dialoqueFreqChangeAirGnd.visible=false;
                }
            }

            Button{
                Layout.preferredWidth: 150
                text: "YES";
                Material.accent: m_fc_is_armed ?  Material.Red : Material.Grey
                highlighted: m_fc_is_armed ? true : false;

                onPressed: {
                    var result= _wbLinkSettingsHelper.change_param_air_and_ground_frequency(m_wanted_frequency);
                    if(result==0){
                        _qopenhd.show_toast("SUCCESS");
                        dialoqueFreqChangeAirGnd.visible=false;
                        return;
                    }else if(result==-1){
                        // Air unit rejected
                        _qopenhd.show_toast("ERROR - AIR REJECTED");
                        return;
                    }else if(result==-2){
                        // Couldn't reach air unit
                        var error_message_not_reachable="Couldn't reach air unit -";
                        _qopenhd.show_toast("ERROR -COULDN'T REACH AIR");
                        return;
                    }
                    // Really really bad
                    _messageBoxInstance.set_text_and_show("Something went wrong - please use 'FIND AIR UNIT' to fix");
                    dialoqueFreqChangeAirGnd.visible=false;
                }
            }
        }
    }
}

