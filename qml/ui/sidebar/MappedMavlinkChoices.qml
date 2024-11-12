import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

import Qt.labs.settings 1.0

import OpenHD 1.0

import "../../ui" as Ui
import "../elements"

Item {
    id: mappedMavlinkChoices


    ListModel{
        id: elements_model_brightness
        //ListElement {value: 0; verbose:"0%"}
        //ListElement {value: 25; verbose:"25%"}
        //ListElement {value: 50; verbose:"50%"}
        ListElement {value: 75; verbose:"75%"}
        ListElement {value: 95; verbose:"95%"}
        ListElement {value: 98; verbose:"98%"}
        ListElement {value: 100; verbose:"100%\n(Default)"}
        ListElement {value: 102; verbose:"102%"}
        ListElement {value: 105; verbose:"105%"}
        ListElement {value: 125; verbose:"125%"}
        //ListElement {value: 150; verbose:"150%"}
        //ListElement {value: 175; verbose:"175%"}
        //ListElement {value: 200; verbose:"200%"}
    }
    ListModel{
        id: elements_model_saturation
        ListElement {value: 50; verbose:"50%"}
        ListElement {value: 75; verbose:"75%"}
        ListElement {value: 90; verbose:"90%"}
        ListElement {value: 100; verbose:"100%\n(Default)"}
        ListElement {value: 110; verbose:"110%"}
        ListElement {value: 125; verbose:"125%"}
        ListElement {value: 150; verbose:"150%"}
    }
    ListModel{
        id: elements_model_contrast
        ListElement {value: 50; verbose:"50%"}
        ListElement {value: 75; verbose:"75%"}
         ListElement {value: 90; verbose:"90%"}
        ListElement {value: 100; verbose:"100%\n(Default)"}
        ListElement {value: 125; verbose:"125%"}
        ListElement {value: 150; verbose:"150%"}
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

    // Built dynamically depending on the settings
    // 5700,5745,5785,5825,5865,5260,5280
    ListModel{
        id: frequencies_model
        ListElement {value: 5700; verbose:"5700Mhz\nOHD 1"}
        ListElement {value: 5745; verbose:"5745Mhz\nOHD 2"}
        ListElement {value: 5785; verbose:"5785Mhz\nOHD 3"}
        ListElement {value: 5825; verbose:"5825Mhz\nOHD 4"}
        ListElement {value: 5865; verbose:"5865Mhz\nOHD 5"}
        ListElement {value: 5260; verbose:"5260Mhz\nOHD 6"}
        ListElement {value: 5280; verbose:"5280Mhz\nOHD 7"}
    }
    ListModel{
        id: frequencies_model_with_5180mhz_lowband
        ListElement {value: 5180; verbose:"5180Mhz\n{CUSTOM LB}"}
        ListElement {value: 5220; verbose:"5220Mhz\n{CUSTOM LB}"}
        ListElement {value: 5260; verbose:"5260Mhz\n{CUSTOM LB}"}
        ListElement {value: 5300; verbose:"5300Mhz\n{CUSTOM LB}"}
        //
        ListElement {value: 5700; verbose:"5700Mhz\nOHD 1"}
        ListElement {value: 5745; verbose:"5745Mhz\nOHD 2"}
        ListElement {value: 5785; verbose:"5785Mhz\nOHD 3"}
        ListElement {value: 5825; verbose:"5825Mhz\nOHD 4"}
        ListElement {value: 5865; verbose:"5865Mhz\nOHD 5"}
        ListElement {value: 5260; verbose:"5260Mhz\nOHD 6"}
        ListElement {value: 5280; verbose:"5280Mhz\nOHD 7"}
    }
    ListModel{
        id: elements_model_channel_width
        ListElement {value: 20; verbose:"20Mhz"}
        ListElement {value: 40; verbose:"40Mhz\n(HIGH BW)"}
    }
    ListModel{
        id: elements_frequency_scan
        ListElement {value: 0; verbose:"OHD"}
        ListElement {value: 1; verbose:"5.8"}
        ListElement {value: 2; verbose:"2.4"}
        ListElement {value: 3; verbose:"ALL"}
    }
    ListModel{
        id: elements_model_rate
        ListElement {value: 0; verbose:"MCS0\n(LONG RANGE)"}
        ListElement {value: 1; verbose:"MCS1\n(RANGE)"}
        ListElement {value: 2; verbose:"MCS2\n(QUALITY)"}
    }


    function get_model(param_id){
        if(param_id=="BRIGHTNESS"){
            return elements_model_brightness;
        }else if(param_id=="SATURATION"){
            return elements_model_saturation;
        }else if(param_id=="CONTRAST"){
            return elements_model_contrast;
        }else if(param_id=="SHARPNESS"){
            return elements_model_sharpness;
        }else if(param_id=="ROTATION"){
            return elements_model_rotation;
        }else if(param_id=="ENABLE_JOY_RC"){
            return elements_model_on_off;
        }else if(param_id=="AIR_RECORDING_E"){
            return elements_model_air_recording;
        }else if(param_id=="WIFI_HOTSPOT_E"){
            return elements_model_hotspot
        }else if(param_id=="ROTATION_FLIP"){
            return elements_model_camera_rotation_flip
        }else if(param_id=="RESOLUTION_FPS"){
            return get_camera_resolution_model();
        }else if(param_id=="FREQUENCY"){
            if(settings.dev_show_5180mhz_lowband){
                return frequencies_model_with_5180mhz_lowband
            }
            return frequencies_model;
        }else if(param_id=="FREQUENCY_SCAN"){
            return elements_frequency_scan;
        }else if(param_id=="CHANNEL_WIDTH"){
            return elements_model_channel_width;
        }else if(param_id=="RATE"){
            return elements_model_rate;
        }
        return elements_model_undefined;
    }

}
