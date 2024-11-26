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
BaseJoyEditElement{

    // Needs to contain the unique ID of the param inside the param set
    property string m_param_id: "FILL ME"
    // Set to air / ground /air cam 1 / air cam 2 depending on where the param is stored
    property var m_settings_model: _ohdSystemGroundSettings
    // Int param is much more common, but string param is also possible
    property bool override_takes_string_param: false

    ListModel{
        id: elements_model_brightness
        ListElement {value: 0; verbose:"0%"}
        ListElement {value: 25; verbose:"25%"}
        ListElement {value: 50; verbose:"50%"}
        ListElement {value: 75; verbose:"75%"}
        ListElement {value: 95; verbose:"95%"}
        ListElement {value: 98; verbose:"98%"}
        ListElement {value: 100; verbose:"100%\n(Default)"}
        ListElement {value: 102; verbose:"102%"}
        ListElement {value: 105; verbose:"105%"}
        ListElement {value: 125; verbose:"125%"}
        ListElement {value: 150; verbose:"150%"}
        ListElement {value: 175; verbose:"175%"}
        ListElement {value: 200; verbose:"200%"}
    }
    ListModel{
        id: elements_model_exposure
        ListElement {value: -3; verbose:"-3"}
        ListElement {value: -2; verbose:"-2"}
        ListElement {value: -1; verbose:"-1"}
        ListElement {value: 0; verbose:"0"}
        ListElement {value: 1; verbose:"+1%"}
        ListElement {value: 2; verbose:"+2"}
        ListElement {value: 3; verbose:"+3"}
    }
    ListModel{
        id: elements_model_metering
        ListElement {value: "centre"; verbose:"center"}
        ListElement {value: "spot"; verbose:"spot"}
        ListElement {value: "matrix"; verbose:"matrix"}
 
    }
    ListModel{
        id: elements_model_sharpness
        ListElement {value: 50; verbose:"50%"}
        ListElement {value: 75; verbose:"75%"}
        ListElement {value: 90; verbose:"90%"}
        ListElement {value: 100; verbose:"100%\n(Default)"}
        ListElement {value: 110; verbose:"110%"}
        ListElement {value: 125; verbose:"125%"}
        ListElement {value: 150; verbose:"150%"}
    }
    ListModel{
        id: elements_model_rotation
        ListElement {value: 50; verbose:"50%"}
        ListElement {value: 75; verbose:"75%"}
        ListElement {value: 90; verbose:"90%"}
        ListElement {value: 100; verbose:"100%\n(Default)"}
        ListElement {value: 110; verbose:"110%"}
        ListElement {value: 125; verbose:"125%"}
        ListElement {value: 150; verbose:"150%"}
    }
    ListModel{
        id: elements_model_on_off
        ListElement {value: 0; verbose:"OFF"}
        ListElement {value: 1; verbose:"ON"}
    }
    ListModel{
        id: elements_model_air_recording
        ListElement {value: 0; verbose:"ALWAYS\nOFF"}
        ListElement {value: 1; verbose:"ALWAYS\nON"}
         ListElement {value: 2; verbose:"AUTO\n(WHEN ARMED)"}
    }
    ListModel{
        id: elements_model_hotspot
        ListElement {value: 0; verbose:"AUTO\n(WHEN DISARMED)"}
        ListElement {value: 1; verbose:"ALWAYS\nOFF"}
        ListElement {value: 2; verbose:"ALWAYS\nON"}
    }
    ListModel{
        id: elements_model_camera_rotation_degree
        ListElement {value: 0; verbose:"0°\n(Default)"}
        ListElement {value: 1; verbose:"180°\n"}
    }
    ListModel{
        id: elements_model_camera_rotation_flip
        ListElement {value: 0; verbose:"0°\n(Default)"}
        ListElement {value: 1; verbose:"VFLIP\n(mirror)\n"}
        ListElement {value: 2; verbose:"HFLIP\n(180°)"}
        ListElement {value: 3; verbose:"BOTH\n"}
    }

    ListModel{
        id: elements_model_undefined
        ListElement {value: 0; verbose:"0\nUNDEFINED"}
        ListElement {value: 1; verbose:"1\nUNDEFINED"}
        ListElement {value: 2; verbose:"2\nUNDEFINED"}
    }

    // NOTE: This is for a string param !
    ListModel{
        id: elements_model_camera_resolution_bup
        ListElement {value: "640x480@60"; verbose:"VGA 4:3\n60fps"}
        ListElement {value: "1280x720@60"; verbose:"HD 16:9\n60fps"}
        ListElement {value: "1920x1080@30"; verbose:"FHD 16:9\n30fps"}
    }
    ListModel{
        id: elements_model_camera_resolution_dynamic
    }
    // Built dynamically depending on the used camera
    function get_camera_resolution_model(){
        if(_cameraStreamModelPrimary.camera_type<0){
            return elements_model_camera_resolution_bup;
        }
        var supported_resolutions=_cameraStreamModelPrimary.get_supported_resolutions();
        elements_model_camera_resolution_dynamic.clear()
        for(var i=0; i<supported_resolutions.length; i++){
            var tmp=supported_resolutions[i];
            var verbose_str=_cameraStreamModelPrimary.make_resolution_fps_verbose(tmp);
            //console.log("Supported:["+tmp+"]");
            elements_model_camera_resolution_dynamic.append({value: tmp, verbose: verbose_str});
        }
        return elements_model_camera_resolution_dynamic;
    }

    property int m_model_index: -1;
    property int m_actual_value_int: -1;
    property string m_actual_value_string: ""

    function get_model(){
        if(m_param_id=="BRIGHTNESS"){
            return elements_model_brightness;
        }else if(m_param_id=="EXPOSURE"){
            return elements_model_exposure;
        }else if(m_param_id=="METERING"){
            return elements_model_metering;
        }else if(m_param_id=="SHARPNESS"){
            return elements_model_sharpness;
        }else if(m_param_id=="ROTATION"){
            return elements_model_rotation;
        }else if(m_param_id=="ENABLE_JOY_RC"){
            return elements_model_on_off;
        }else if(m_param_id=="AIR_RECORDING_E"){
            return elements_model_air_recording;
        }else if(m_param_id=="WIFI_HOTSPOT_E"){
            return elements_model_hotspot
        }else if(m_param_id=="ROTATION_FLIP"){
            return elements_model_camera_rotation_flip
        }else if(m_param_id=="RESOLUTION_FPS"){
            return get_camera_resolution_model();
        }
        return elements_model_undefined;
    }

    onVisibleChanged: {
        if(visible){
            populate();
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
    property bool populate_enable_left:false
    property bool populate_enable_right:false
    property string populate_display_text:"I SHOULD NEVER APPEAR"

    function populate(){
        // First, check if the system is alive
        if(!m_settings_model.system_is_alive()){
            // Do not enable the elements, system is not alive
            m_model_index=-1;
            populate_enable_left=false;
            populate_enable_right=false;
            populate_display_text="N/A";
            return;
        }
        if(!m_settings_model.has_params_fetched){
            m_model_index=-1;
            populate_enable_left=false;
            populate_enable_right=false;
            populate_display_text="N/A";
            return;
        }
        var param_exists;
        if(override_takes_string_param){
            param_exists=m_settings_model.param_string_exists(m_param_id)
        }else{
            param_exists=m_settings_model.param_int_exists(m_param_id)
        }
        if(!param_exists){
            console.log("Param "+m_param_id+" does not exist");
            m_model_index=-1;
            populate_enable_left=false;
            populate_enable_right=false;
            populate_display_text="NOT\nAVAILABLE";
            return;
        }
        if(override_takes_string_param){
            var actual_value_string=m_settings_model.get_cached_string(m_param_id);
            update_model_index(actual_value_string);
        }else{
            var actual_value_int=m_settings_model.get_cached_int(m_param_id);
            update_model_index(actual_value_int);
        }
    }


    function update_model_index(value){
        const m_elements_model=get_model();
        m_model_index=-1;
        for(var i=0;i<m_elements_model.count;i++){
            const tmp=m_elements_model.get(i).value;
            if(value===tmp){
                m_model_index=i;
                break;
            }
        }
        if(m_model_index==-1){
            // The current value does not exist inside the model
            var tmp="{";
            tmp+=value;
            tmp+="}\nCUSTOM";
            populate_display_text=tmp
            populate_enable_left=false;
            populate_enable_right=false;
        }else{
            // The current value exists inside the mdel, activate the up/down as long as
            // that does not go out of the bounds of the model
            if(m_model_index>0){
                populate_enable_left=true;
            }else{
                populate_enable_left=false;
            }
            if(m_model_index<m_elements_model.count-1){
                populate_enable_right=true;
            }else{
                populate_enable_right=false;
            }
            populate_display_text=m_elements_model.get(m_model_index).verbose;
        }
    }

    m_button_left_activated: {
        return populate_enable_left && !m_settings_model.ui_is_busy;
    }

    m_button_right_activated: {
        return populate_enable_right && !m_settings_model.ui_is_busy;
    }

    m_displayed_value: {
        return populate_display_text;
    }

    function try_update_value_async(increment){
        if(m_model_index==-1)return;
        var new_model_index=m_model_index;
        if(increment){
            new_model_index+=1;
        }else{
            new_model_index-=1;
        }
        const value_new=get_model().get(new_model_index).value;
        if(override_takes_string_param){
             m_settings_model.try_set_param_string_async(m_param_id,value_new);
        }else{
             m_settings_model.try_set_param_int_async(m_param_id,value_new);
        }
    }

    onChoice_left: {
        try_update_value_async(false);
    }

    onChoice_right: {
        try_update_value_async(true);
    }


}
