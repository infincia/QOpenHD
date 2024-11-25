import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

import Qt.labs.settings 1.0

import OpenHD 1.0

import "../../ui" as Ui
import "../elements"


//
// Complex ;)
// Way more javascript code than i'd like, but as long as you:
// 1) Specify the right param_id
// 2) Specify the right settings model instance
// 3) Declare a matching choices-model here
// This handles all the edge cases that might arise ;)
BaseJoyEditElement2{
    id: mavlinkChoiceElement2
    // Needs to contain the unique ID of the param inside the param set
    property string m_param_id: "FILL ME"
    // Set to air / ground /air cam 1 / air cam 2 depending on where the param is stored
    property var m_settings_model: _ohdSystemGroundSettings
    // Int param is much more common, but string param is also possible
    property bool override_takes_string_param: false
    // contains the actual, current param
    property int m_actual_value_int: -1;
    property string m_actual_value_string: ""
    // set to true if the value exists inside the param set
    property bool m_param_exists: false

    // EXTRA
    property string mPARAM_ID_CHANNEL_WIDTH: "CHANNEL_WIDTH"
    property string mPARAM_ID_FREQUENCY: "FREQUENCY"
    property string mPARAM_ID_FREQUENCY_SCAN: "FREQUENCY_SCAN"
    property string mPARAM_ID_RATE: "RATE"


    signal goto_previous();
    signal goto_next();

    MappedMavlinkChoices{
        id: mappedMavlinkChoices
    }

    Keys.onPressed: (event)=> {
                        console.log("BaseJoyElement"+m_title+" key was pressed:"+event);
                        if(event.key == Qt.Key_Left){
                            sidebar.regain_control_on_sidebar_stack();
                            event.accepted=true;
                        }else if(event.key == Qt.Key_Right){
                            open_choices_menu(false);
                            event.accepted=true;
                        }else if(event.key==Qt.Key_Up){
                            choiceSelector.discard_and_close();
                            goto_previous()
                            event.accepted=true;
                        }else if(event.key==Qt.Key_Down){
                            choiceSelector.discard_and_close();
                            goto_next();
                            event.accepted=true;
                        }else if(event.key==Qt.Key_Enter || event.key==Qt.Key_Return){
                            open_choices_menu(false);
                            event.accepted=true;
                        }
                    }

    onBase_joy_edit_element_clicked: {
        if(choiceSelector.visible){
            choiceSelector.close_choices();
        }else{
            open_choices_menu(true);
        }
    }

    function open_choices_menu(clickable){
        if(!m_param_exists){
            _qopenhd.show_toast("N/A");
            return;
        }
        if(m_settings_model.ui_is_busy){
            _qopenhd.show_toast("Busy, please try again later");
            return;
        }
        const m_elements_model=mappedMavlinkChoices.get_model(m_param_id);
        if(override_takes_string_param){
            choiceSelector.open_choices(m_elements_model,m_actual_value_string,mavlinkChoiceElement2)
        }else{
            choiceSelector.open_choices(m_elements_model,m_actual_value_int,mavlinkChoiceElement2)
        }
        choiceSelector.set_clickable(clickable)
    }

    onVisibleChanged: {
        if(visible){
            populate();
            extra_populate();
        }
    }

    property int m_update_count: m_settings_model.update_count
    onM_update_countChanged: {
        if(visible){
            if(m_settings_model.last_updated_param_id===m_param_id){
                populate();
            }
        }
    }
    property bool m_has_params_fetched: m_settings_model.has_params_fetched
    onM_has_params_fetchedChanged: {
        if(visible){
            populate();
        }
    }

    // Avoid binding loop
    property string populate_display_text:"I SHOULD NEVER APPEAR"

    function populate(){
        // Don't mind those 3
        if(m_param_id==mPARAM_ID_CHANNEL_WIDTH || m_param_id==mPARAM_ID_FREQUENCY || m_param_id==mPARAM_ID_RATE){
            return;
        }
        // First, check if the system is alive
        if(!m_settings_model.system_is_alive()){
            // Do not enable the elements, system is not alive
            m_param_exists=false;
            populate_display_text="N/A";
            return;
        }
        if(!m_settings_model.has_params_fetched){
            m_param_exists=false;
            populate_display_text="N/A";
            return;
        }
        if(override_takes_string_param){
            m_param_exists=m_settings_model.param_string_exists(m_param_id)
        }else{
            m_param_exists=m_settings_model.param_int_exists(m_param_id)
        }
        if(!m_param_exists){
            console.log("Param "+m_param_id+" does not exist");
            populate_display_text="NOT\nAVAILABLE";
            return;
        }
        if(override_takes_string_param){
            var actual_value_string=m_settings_model.get_cached_string(m_param_id);
            update_display_text(actual_value_string);
        }else{
            var actual_value_int=m_settings_model.get_cached_int(m_param_id);
            update_display_text(actual_value_int);
        }
        m_param_exists=true;
    }


    function update_display_text(value){
        if(override_takes_string_param){
            m_actual_value_string=value
        }else{
            m_actual_value_int=value;
        }
        const m_elements_model=mappedMavlinkChoices.get_model(m_param_id);
        var model_index=-1;
        for(var i=0;i<m_elements_model.count;i++){
            const tmp=m_elements_model.get(i).value;
            if(value===tmp){
                model_index=i;
                break;
            }
        }
        if(model_index==-1){
            // The current value does not exist inside the model
            var tmp="{";
            tmp+=value;
            tmp+="}\nCUSTOM";
            populate_display_text=tmp
        }else{
            populate_display_text=m_elements_model.get(model_index).verbose;
        }
    }

    m_displayed_value: {
        return populate_display_text;
    }


    function user_selected_value(value_new){
        // A few need to be handled specially
         if(m_param_id==mPARAM_ID_FREQUENCY){
            if(_fcMavlinkSystem.armed){
                if(settings.dev_allow_freq_change_when_armed){
                    // okay
                }else{
                    _qopenhd.show_toast("ARMED, change not possible");
                    return;
                }
            }
        }
        if(m_param_id==mPARAM_ID_FREQUENCY){
            const new_frequency=value_new;
            _qopenhd.set_busy_for_milliseconds(2000,"CHANGING FREQUENCY");
            _wbLinkSettingsHelper.change_param_air_and_ground_frequency(value_new)
            return;
        }else if(m_param_id==mPARAM_ID_CHANNEL_WIDTH){
            const channel_width_mhz=value_new;
            if(!_ohdSystemAir.is_alive){
                _hudLogMessagesModel.add_message_warning("Cannot change BW:"+channel_width_mhz+"Mhz, AIR not alive");
                return;
            }
            _qopenhd.set_busy_for_milliseconds(2000,"CHANGING BW");
            _wbLinkSettingsHelper.change_param_air_channel_width_async(channel_width_mhz,true);
            return;
        }else if(m_param_id==mPARAM_ID_RATE){
            const mcs_index=value_new;
            _qopenhd.set_busy_for_milliseconds(2000,"CHANGING RATE");
           _wbLinkSettingsHelper.set_param_air_only_mcs_async(mcs_index)
            return;
        }else{
            // 'normal' params
            if(override_takes_string_param){
                m_settings_model.try_set_param_string_async(m_param_id,value_new);
            }else{
                m_settings_model.try_set_param_int_async(m_param_id,value_new);
            }
        }
    }


    // -------------- EXTRA --------------
    property int curr_channel_mhz: _ohdSystemAir.curr_channel_mhz
    onCurr_channel_mhzChanged: {
        extra_populate();
    }
    property int curr_mcs_index:_ohdSystemAir.curr_mcs_index;
    onCurr_mcs_indexChanged: {
        extra_populate();
    }
    property int curr_bandwidth_mhz: _ohdSystemAir.curr_channel_width_mhz
    onCurr_bandwidth_mhzChanged: {
        extra_populate();
    }
    function extra_populate(){
        if(!(m_param_id==mPARAM_ID_CHANNEL_WIDTH || m_param_id==mPARAM_ID_FREQUENCY || m_param_id==mPARAM_ID_RATE)){
            return;
        }
        // First, check if the system is alive
        if(!m_settings_model.system_is_alive()){
            // Do not enable the elements, system is not alive
            m_param_exists=false;
            populate_display_text="N/A";
            return;
        }
        if(m_param_id==mPARAM_ID_FREQUENCY_SCAN){
                m_param_exists=true;
            }
        if(m_param_id==mPARAM_ID_FREQUENCY){
            if(curr_channel_mhz<=0){
                m_param_exists=false;
                populate_display_text="N/A";
                return;
            }
            update_display_text(curr_channel_mhz);
            m_param_exists=true;
        }else if(m_param_id==mPARAM_ID_RATE){
            if(curr_mcs_index<0){
                m_param_exists=false;
                populate_display_text="N/A";
                return;
            }
            update_display_text(curr_mcs_index);
            m_param_exists=true;
        }else if(m_param_id==mPARAM_ID_CHANNEL_WIDTH){
            if(curr_bandwidth_mhz<=0){
                m_param_exists=false;
                populate_display_text="N/A";
                return;
            }
            update_display_text(curr_bandwidth_mhz);
            m_param_exists=true;
        }
    }

}
