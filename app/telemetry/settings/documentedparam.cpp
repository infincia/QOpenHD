#include "documentedparam.h"
#include <map>
#include <memory>

#include "param_names.h"
//
// Here we have the mapping (if available) and documentation for all parameters
//
// WAY BIGGER file than one would like, but there is no easy alternative

using namespace DocumentedParam;

// These are util methods for the most common cases for adding parameters to the stored parameters set
static void append_int(std::vector<std::shared_ptr<XParam>>& list,std::string param_name,ImprovedIntSetting improved_int,std::string description,bool requires_reboot=false){
    auto tmp=std::make_shared<XParam>(param_name,improved_int,std::nullopt,description,requires_reboot,false);
    list.push_back(tmp);
}
static void append_string(std::vector<std::shared_ptr<XParam>>& list,std::string param_name,ImprovedStringSetting improved_string,std::string description,bool requires_reboot=false){
    auto tmp=std::make_shared<XParam>(param_name,std::nullopt,improved_string,description,requires_reboot,false);
    list.push_back(tmp);
}
static void append_only_documented(std::vector<std::shared_ptr<XParam>>& list,std::string param_name,std::string description,bool requires_reboot=false){
    auto tmp=std::make_shared<XParam>(param_name,std::nullopt,std::nullopt,description,requires_reboot,false);
    list.push_back(tmp);
}
static void append_documented_read_only(std::vector<std::shared_ptr<XParam>>& list,std::string param_name,std::string description){
    auto tmp=std::make_shared<XParam>(param_name,std::nullopt,std::nullopt,description,false,true);
    list.push_back(tmp);
}

static std::vector<std::shared_ptr<XParam>> get_parameters_list(){
    std::vector<std::shared_ptr<XParam>> ret;
    // These params do not exist, they are only for testing
    append_int(ret,"TEST_INT_0",
               ImprovedIntSetting::createEnumEnableDisable(),
               "test"
               );
    {
        std::vector<std::string> values{};
        values.push_back("enum0");
        values.push_back("enum1");
        values.push_back("enum2");
        values.push_back("enum3");
        append_int(ret,"TEST_INT_1", ImprovedIntSetting::createEnum(values),"test");
    }

    // -------------------------------------------------------------------------------------------------------------------------------------------------------
    // Wifibroadcast parameters
    // -------------------------------------------------------------------------------------------------------------------------------------------------------
    append_int(ret,openhd::WB_VIDEO_VARIABLE_BITRATE,
               ImprovedIntSetting::createEnumEnableDisable(),
               "Recommend a matching bitrate to the encoder depending on the selected link parameters,and reduce bitrate on TX errors (failed injections). On by default, but only works on select cameras. On Cameras that"
               "don't support changing the bitrate / are bad at targeting a given bitrate, you have to adjust your link according to your camera needs."
               );
    {
        std::vector<std::string> values{};
        values.push_back("Disable");
        values.push_back("+1 (2 antennas)");
        //values.push_back("+2 Spatial streams");
        //values.push_back("+3 Spatial streams");
        append_int(ret,openhd::WB_ENABLE_STBC,
                   ImprovedIntSetting::createEnum(values),
                   "!! Advanced users only, changing this param can result in no connection !!. This param is not automatically synchronized between air/ground. "
                   "Specifies the difference between the number of space-time streams and the number of spatial streams. Use +1 only if you have 2 antennas."
                   );
    }
    append_int(ret,openhd::WB_ENABLE_LDPC,
               ImprovedIntSetting::createEnumEnableDisable(),
               "!! Advanced users only, changing this param can result in no connection !!. This param is not automatically synchronized between air/ground. "
               "Enable Low density parity check. Needs to be supported by both your tx and rxes."
               );
    append_int(ret,openhd::WB_ENABLE_SHORT_GUARD,
               ImprovedIntSetting::createEnum({"LONG_GI (default)","SHORT_GI"}),
               "!! Advanced users only !!. This param is not automatically synchronized between air/ground. A short guard intervall increases throughput, "
               "but increases packet collisions."
               );
    append_int(ret,openhd::WB_PASSIVE_MODE,
               ImprovedIntSetting::createEnumEnableDisable(),
               "Enable passive mode if you want to use your GCS as a passive listener to an existing openhd air-ground link. E.g. if you want to tune into"
               "someone elses openhd link (if encryption is enabled, you need his encryption key) but not interfere with any RC/MAVLINK control."
               );
    append_int(ret,openhd::WB_DEV_AIR_SET_HIGH_RETRANSMIT_COUNT,
               ImprovedIntSetting::createEnumEnableDisable(),
               "DEV ONLY - DO NOT TOUCH (LEAVE DISABLED). Sets a wifi param that needs to be investigated."
               );
    {
// ---------- TX POWER - only as placeholders, wizzard is recommended !
        // Measurements of @Marcel Essers:
        //19: 10-12 mW
        //25: 25-30 mW
        //30: 45-50 mW
        //35: 70-80 mW
        //37: 100-110 mW
        //40: 120-140 mW
        //45: 200-230 mW
        //50: 280- 320 mW
        //55: 380-400 mW
        //58: 420-450 mW
        // NOTE: We now have the tx power wizzard !
        auto values_WB_TX_PWR_INDEX=std::vector<ImprovedIntSetting::Item>{
            {"Unitless [1]",1},
            {"Unitless [3]",3},
            {"Unitless [6]",6},
            {"Unitless [10]",10},
            {"Unitless [20]",20},
            {"Unitless [26]",26},
            {"Unitless [40]",40},
            {"Unitless [60]",60},
            {"Unitless [63]",63},                                                                   };
        auto values_WB_TX_PWR_INDEX_ARMED=std::vector<ImprovedIntSetting::Item>{
            {"Disabled [0]",0},
            {"Unitless [1]",1},
            {"Unitless [3]",3},
            {"Unitless [6]",6},
            {"Unitless [10]",10},
            {"Unitless [20]",20},
            {"Unitless [26]",26},
            {"Unitless [40]",40},
            {"Unitless [60]",60},
            {"Unitless [63]",63},
        };
        append_int(ret,openhd::WB_RTL8812AU_TX_PWR_IDX_OVERRIDE,
                   ImprovedIntSetting(0,63,values_WB_TX_PWR_INDEX),
                   "Please use the TX POWER wizzard from WB Link to avoid destroying your card ! Unitless Tx power index value applied when the FC is disarmed and no specific armed"
                   "tx power value is set."
                   );
        append_int(ret,openhd::WB_RTL8812AU_TX_PWR_IDX_ARMED,
                   ImprovedIntSetting(0,63,values_WB_TX_PWR_INDEX_ARMED),
                   "Please use the TX POWER wizzard from WB Link to avoid destroying your card ! Unitless Tx power index value applied when the FC is armed, off by default"
                   );
        append_only_documented(ret,openhd::WB_TX_POWER_MILLI_WATT,
                               "Please use the TX POWER wizzard from WB Link to avoid destroying your card ! tx power in mW when FC is disarmed and no specific armed tx power value is set. Actual tx power depends on the manufacturer.");
        append_only_documented(ret,openhd::WB_TX_POWER_MILLI_WATT_ARMED,
                               "Please use the TX POWER wizzard from WB Link to avoid destroying your card ! tx power in mW when FC is armed, off by default. Actual tx power depends on the manufacturer.");
    }
// -----------------------------------------------------------------------------------------------------------
    {
        auto default_values=std::vector<ImprovedIntSetting::Item>{
                                                                    {"AUTO (Default)",0},
                                                                    {"FEC_K=8",8},
                                                                    {"FEC_K=10",10},
                                                                    {"FEC_K=12",12},
                                                                    {"FEC_K=16",16},
                                                                    {"FEC_K=20",20},
                                                                    {"FEC_K=50",50},
                                                                    };
        append_int(ret,openhd::WB_VIDEO_FEC_BLOCK_LENGTH,
                   ImprovedIntSetting(0,128,default_values),
                   "Default AUTO (Uses biggest block sizes possible while not adding any latency).Otherwise: WB Video FEC block length, previous FEC_K. "
                   "Increasing this value can improve link stability for free, but can create additional latency.");
    }
    {
        auto default_values=std::vector<ImprovedIntSetting::Item>{
                                                                    {"70% (lower)",70},
                                                                    {"80% (lower)",80},
                                                                    {"90% (slightly lower)",90},
                                                                    {"DEFAULT",100},
                                                                    {"110% (slightly higher)",110},
                                                                    {"120% (higher)",120},
                                                                    {"130% (higher)",130},
                                                                    };
        append_int(ret,openhd::WB_VIDEO_RATE_FOR_MCS_ADJUSTMENT_PERC,
                   ImprovedIntSetting(1,500,default_values),
                   "Reduce used data rate per mcs index by fixed value (not needed in most cases)");
    }
    {
        auto default_values=std::vector<ImprovedIntSetting::Item>{
                                                                    {"10%",10},
                                                                    {"20% (low interf)",20},
                                                                    {"30%",30},
                                                                    {"40%",40},
                                                                    {"50% (high interf)",50},
                                                                    {"100%",100},
                                                                    };
        append_int(ret,openhd::WB_VIDEO_FEC_PERCENTAGE,
                   ImprovedIntSetting(0,100,default_values),
                   "WB Video FEC overhead, in percent. Increases link stability, but also the required link bandwidth (watch out for tx errors). "
                   "The best value depends on your rf environment - recommended is about ~20% in low rf enironemnts(e.g. nature), and ~50% in high rf noise environments(populated areas)."
                   "Note that your rf interference/ packet loss will increase during flight, it is not recommended to decrase this value below 20%.");
    }

    {
        std::pair<std::string,int> val1{"20Mhz",20};
        std::pair<std::string,int> val2{"40Mhz",40};
        append_int(ret,openhd::WB_CHANNEL_WIDTH,
                   ImprovedIntSetting::createEnumSimple({val1,val2}),
                   "!!!Editing this param manually without care will result in a broken link!!!"
                   );
    }
    append_int(ret,openhd::WB_MCS_INDEX,
               ImprovedIntSetting::createEnum({"MCS0","MCS1","MCS2","MCS3","MCS4(not rec)","MCS5(not rec)","MCS6(not rec)","MCS7(not rec)",
                                               "MCS8 (VHT0+2SS)", "MCS9 (VHT1+2SS)", "MCS10 (VHT2+2SS)", "MCS11 (VHT3+2SS)",
                                               }),
               "!!!Editing this param manually without care will result in a broken link!!!"
               );
    {
        std::vector<std::string> disable_or_channels{"Disable","Channel 1","CHannel 2","Channel 3","Channel 4","Channel 5",
                                                          "Channel 6","Channel 7","Channel 8","Channel 9","Channel 10"};
        append_int(ret,openhd::WB_MCS_INDEX_VIA_RC_CHANNEL,
                   ImprovedIntSetting::createEnum(disable_or_channels),
                   "Dynamically change the MCS Index (Trade range <-> image quality (bitrate)) during flight using your RC and a specific channel "
                   "(similar to how flight modes work)."
                   );
        append_int(ret,openhd::WB_BW_VIA_RC_CHANNEL,
                   ImprovedIntSetting::createEnum(disable_or_channels),
                   "Dynamically change the BW via RC. NOT ALWAYS SAFE TO USE !"
                   );
        append_int(ret,openhd::WB_POWER_VIA_RC_CHANNEL,
                   ImprovedIntSetting::createEnum(disable_or_channels),
                   "Dynamically change the POWER during flight using your RC and a specific channel "
                   "(similar to how flight modes work)."
                   );

    }

    append_only_documented(ret,openhd::WB_FREQUENCY,"!!!Editing this param manually without care will result in a broken link!!!");
    {
        auto default_values=std::vector<ImprovedIntSetting::Item>{
                                                                    {"AUTO -1",-1},
                                                                    {"LOW 20",20},
                                                                    {"30",30},
                                                                    {"40",40},
                                                                    {"50",50},
                                                                    {"100",100},
                                                                    {"128",128},
                                                                    };
        append_int(ret,openhd::WB_MAX_FEC_BLOCK_SIZE_FOR_PLATFORM,
                   ImprovedIntSetting(-1,128,default_values),
                   "Developer only, max FEC block size.");
    }
    const auto descr_wifi_card="Detected wifi card type used for wifibroadcast.";
    append_documented_read_only(ret,"WIFI_CARD0",descr_wifi_card);
    append_documented_read_only(ret,"WIFI_CARD1",descr_wifi_card);
    append_documented_read_only(ret,"WIFI_CARD2",descr_wifi_card);
    append_documented_read_only(ret,"WIFI_CARD3",descr_wifi_card);
    append_documented_read_only(ret,"HOTSPOT_CARD","Detected card for wifi hotspot");
    append_documented_read_only(ret,"WB_N_RX_CARDS","TODO");
    append_only_documented(ret,"FC_BATT_N_CELLS","Stored locally on your air unit. If set to a nonzero value, overwrites the batt n cells of any connected GS (QOpenHD)"
                                                 "such that you don't have to change the value when swapping around planes.");
    // -------------------------------------------------------------------------------------------------------------------------------------------------------
    // video / camera parameters
    // -------------------------------------------------------------------------------------------------------------------------------------------------------
    {
        {
            const auto choices_video_res_framerate=std::vector<std::string>{
                "640x480@30",
                "640x480@60",
                "640x480@90",
                //"960x720@30",
                //"960x720@60",
                "1280x720@30",
                "1280x720@49",
                "1280x720@60",
                //"1440x1080@30",
                "1920x1080@30",
            };
            append_string(ret,"RESOLUTION_FPS",ImprovedStringSetting::create_from_keys_only(choices_video_res_framerate),
                          "Video WIDTHxHEIGHT@FPS. You can enter any value you want here, but if you select a video format that is not supported by your camera, the video stream will stop");
        }
        append_int(ret,"VIDEO_CODEC",
                   //NOTE: MJPEG has been removed intentionally, since we are going to eventually remove support for it in
                   //favour of h264
                   // ,"mjpeg"
                   ImprovedIntSetting::createEnum( std::vector<std::string>{"h264","h265"}),
                   "Video codec. If your camera/ground station does not support HW accelerated encoding/decoding of the selected codec,it'l default to SW encode/decode. "
                   "A reboot (air&ground) is recommended after changing this parameter."
                   );
        append_int(ret,"AIR_RECORDING_E",
                   ImprovedIntSetting::createEnum( std::vector<std::string>{"DISABLE","ENABLE","AUTO(armed)"}),
                   "Record video data locally on your air unit. You can find the files under /home/openhd/Videos on the SD card and/or download them via the web ui."
                   "When AUTO is set, air recording automatically starts (and stops) when you arm/disarm your drone (requires inav / ardupilot FC)."
                   );
        append_int(ret,"STREAMING_E",
                   ImprovedIntSetting::createEnumEnableDisable(),
                   "Enable / disable streaming for this camera. Note that this setting is persistent at run time - once you disable streaming for a camera, you won't have video"
                   " until you re-enable streaming or reboot your air unit. On by default"
                   );
        {
            auto default_values=std::vector<ImprovedIntSetting::Item>{
                                                                        {"2MBit/s",2},
                                                                        {"4MBit/s",4},
                                                                        {"6MBit/s",6},
                                                                        {"8MBit/s (default)",8},
                                                                        {"10MBit/s (high)",10},
                                                                        {"14MBit/s (high)",14},
                                                                        {"18MBit/s (high)",18},
                                                                        };
            append_int(ret,"BITRATE_MBITS",
                       ImprovedIntSetting(1,100,default_values),
                       "Camera encoder bitrate, does not include FEC overhead. "
                       "!! If variable bitrate is enabled (recommended), this value is ignored.!! Otherwise, you can manually set a fixed camera/encoder bitrate here. "
                       "NOTE: If you are using a camera not listed on the OpenHD recommended cameras list, the bitrate might be fixed by the vendor and not changeable."
                       );
        }
        {
            auto default_values=std::vector<ImprovedIntSetting::Item>{
                                                                        {"2  (best recovery)",2},
                                                                        {"3  (best recovery)",3},
                                                                        {"5  (good recovery)",5},
                                                                        {"8  (good recovery)",8},
                                                                        {"10 (medium recovery)",10},
                                                                        {"15 (medium recovery)",15},
                                                                        {"20 (bad recovery)",20},
                                                                        };
            append_int(ret,"KEYFRAME_I",
                       ImprovedIntSetting(0,100,default_values),
                       "Keyframe / instantaneous decode refresh interval, in frames. E.g. if set to 15, every 15th frame will be a key frame. Higher values result in better image compression, but increase the likeliness of microfreezes."
                       );
        }
        append_int(ret,"FORCE_SW_ENC",
                   ImprovedIntSetting::createEnumEnableDisable(),
                   "Force SW encode for the given camera, only enable if your camera supports outputting an appropriate raw format."
                   );
        append_int(ret,"V_SWITCH_CAM",
                   ImprovedIntSetting::createEnumEnableDisable(),
                   "Requires reboot. Switch primary and secondary camera.");
        append_only_documented(ret,"V_PRIMARY_PERC",
                               "If Variable bitrate is enabled,your primary camera is given that much percentage of the total available link bandwidth. "
                               "The rest is given to the secondary camera. Default to 60% (60:40 split).");

        append_int(ret,"ROTATION_FLIP",
                    ImprovedIntSetting(-1,2130706433,{
                       ImprovedIntSetting::Item{"NONE",0},
                       ImprovedIntSetting::Item{"VFLIP°",1},
                       ImprovedIntSetting::Item{"HFLIP",2},
                       ImprovedIntSetting::Item{"BOTH",3}
                   }),
                   "Flip video vertically / horizontally (ROTATE)"
                   );

        append_int(ret,"ROTATION_DEG",
                   ImprovedIntSetting(0,270,{
                                               ImprovedIntSetting::Item{"0°(disable)",0},
                                               //ImprovedIntSetting::Item{"90 (mmal only)°",90},
                                               ImprovedIntSetting::Item{"180°",180},
                                               //ImprovedIntSetting::Item{"270°(mmal only)",270}
                                      }),
                   "Rotate video"
                   );
        append_int(ret,"INTRA_REFRESH",
                   ImprovedIntSetting(-1,2130706433,{
                                                          ImprovedIntSetting::Item{"NONE",-1},
                                                          ImprovedIntSetting::Item{"CYCLIC",0},
                                                          ImprovedIntSetting::Item{"ADAPTIVE",1},
                                                          ImprovedIntSetting::Item{"BOTH",2},
                                                          ImprovedIntSetting::Item{"CYCLIC_ROWS",2130706433}
                                                      }),
                   "Experimental,Default NONE, Type of Intra Refresh to use"
                   );
        append_only_documented(ret,"BRIGHTNESS","Image capture brightness, [0..200], default 100. Increase for a brighter Image. However, if available, it is recommended to tune AWB or EXP instead.");
        append_only_documented(ret,"ISO","ISO value to use (0 = Auto)");

        {
            // rpicamsrc only for now
            auto gst_awb_modes=std::vector<std::string>{
                "OFF",
                "AUTO",
                "SUNLIGHT",
                "CLOUDY",
                "SHADE",
                "TUNGSTEN",
                "FLUORESCENT",
                "INCANDESCENT",
                "FLASH",
                "HORIZON"
            };
            auto gst_exposure_modes=std::vector<std::string>{
                "OFF",
                "AUTO",
                "NIGHT",
                "NIGHTPREVIEW",
                "BACKLIGHT",
                "SPOTLIGHT",
                "SPORTS",
                "SNOW",
                "BEACH",
                "VERYLONG",
                "FIXEDFPS",
                "ANTISHAKE",
                "FIREWORKS",
            };
            auto values_metering_mode=std::vector<std::string>{
                "AVERAGE","SPOT","BACKLIST","MATRIX"
            };
            append_int(ret,"AWB_MODE",
                       ImprovedIntSetting::createEnum(gst_awb_modes),
                       "AWB Automatic white balance mode"
                       );
            append_int(ret,"EXP_MODE",
                       ImprovedIntSetting::createEnum(gst_exposure_modes),
                       "EXP Exposure mode"
                       );
            append_int(ret,"METERING_MODE",
                       ImprovedIntSetting::createEnum(values_metering_mode),
                       "Camera exposure metering mode to use. Default average."
                       );
        }
        {
            // libcamera only
            auto denoise_modes=std::vector<std::string>{
                "AUTO",
                "OFF",
                "CDN_OFF",
                "CDN_FAST",
                "CDN_HQ",
            };
            append_int(ret,"DENOISE_INDEX_LC",
                       ImprovedIntSetting::createEnum(denoise_modes),
                       "Setting this to off reduces latency by ~1 Frame on the cost of slightly reduced image quality in dark situations."
                       );

            append_int(ret,"METERING_MODE_LC",
                       ImprovedIntSetting::createEnum(std::vector<std::string>{
                           "centre (default)", "spot", "average", //crashes libcamera "custom"
                       }), "Libcamera Metering mode.")
                ;
            append_int(ret,"AWB_MODE_LC",
                       ImprovedIntSetting::createEnum(std::vector<std::string>{
                           "auto (default)", "incandescent", "tungsten", "fluorescent", "indoor", "daylight",
                           "cloudy", "custom"
                       }), "Libcamera AWB mode.")
                ;
            append_int(ret,"EXPOSURE_MODE_LC",
                       ImprovedIntSetting::createEnum(std::vector<std::string>{
                           "normal (default)", "sport"
                       }), "Libcamera exposure mode.")
                ;
            append_int(ret,"SHUTTER_US_LC",
                       ImprovedIntSetting::createEnumSimple(
                           std::vector<std::pair<std::string,int>>{
                                                                    {"auto",0},
                                                                    {"example1 (1000us)",1000},
                                                                    {"example2 (5000us)",5000},
                                                                    {"example3 (16666us)",16666},
                                                                    {"example4 (33333us)",33333},
                                                                    }),"Libcamera shutter in microseconds. Normally seleceted automatically, but you can overwrite this value for more control.");
            append_int(ret,"CONTRAST_LC",
                       ImprovedIntSetting::createEnumSimple(
                           std::vector<std::pair<std::string,int>>{
                                                                    {"default(100)",100},
                                                                    {"higher (120)",120},
                                                                    {"higher (150)",150},
                                                                    {"lower(80)",80},
                                                                    {"lower (50)",50},
                                                                    }),
                       "Libcamera contrast");
            append_int(ret,"SATURATION_LC",
                       ImprovedIntSetting::createEnumSimple(
                           std::vector<std::pair<std::string,int>>{
                                                                    {"default(100)",100},
                                                                    {"higher (120)",120},
                                                                    {"higher (150)",150},
                                                                    {"lower(80)",80},
                                                                    {"lower (50)",50},
                                                                    }),
                       "Libcamera saturation");
            append_int(ret,"SHARPNESS_LC",
                       ImprovedIntSetting::createEnumSimple(
                           std::vector<std::pair<std::string,int>>{
                                                                    {"default(100)",100},
                                                                    {"higher (120)",120},
                                                                    {"higher (150)",150},
                                                                    {"lower(80)",80},
                                                                    {"lower (50)",50},
                                                                    }),
                       "Libcamera sharpness");
            append_int(ret,"EXPOSURE_LC",
                       ImprovedIntSetting::createEnumSimple(
                           std::vector<std::pair<std::string,int>>{
                                                                    {"default(0)",0},
                                                                    {"higher (5)",5},
                                                                    {"higher (10)",10},
                                                                    {"lower(-5)",-5},
                                                                    {"lower (-10)",-10},
                                                                    }),
                       "Libcamera exposure value, [-10,10]");
        }
        append_int(ret,"HIGH_ENCRYPTION",
                   ImprovedIntSetting::createEnumEnableDisable(),
                   "Enable ultra secure - but expensive - video encryption - by default, video is not encrypted (only validated) to save CPU performance (Telemetry is always encrypted though)."
                   "It is recommended to leave video encryption off unless you are using at least RPI 4 on air and are TOTALLY worried about someone listening to your video"
                   " - even with encryption disabled, it is not easy for an attacker to listen in on your openhd video "
                   "(and impossible to attack your video due to always on secure packet validation)."
                   );
    }
    // -------------------------------------------------------------------------------------------------------------------------------------------------------
    // Other stuff
    // -------------------------------------------------------------------------------------------------------------------------------------------------------
    {
        append_documented_read_only(ret,"BOARD_TYPE","TODO");
        append_int(ret,"ENABLE_JOY_RC",ImprovedIntSetting::createEnumEnableDisable(),
                   "Only enable joystick rc if you actually use it to save cpu / bandwidth. If enabled, you can connect a joystick to your ground station for RC. After enabling,"
                   "a reboot is recommended, but not neccessary.");
        append_int(ret,"FC_UART_FLWCTL",ImprovedIntSetting::createEnumEnableDisable(),
                   "Leave disabled, for setups with an additional 4th cable for uart flow control");

        {
            auto baud_rate_items=std::vector<ImprovedIntSetting::Item>{
                                                                         {"9600",9600},
                                                                         {"19200",19200},
                                                                         {"38400",38400},
                                                                         {"57600",57600},
                                                                         {"115200",115200},
                                                                         {"230400",230400},
                                                                         {"460800",460800},
                                                                         {"500000",500000},
                                                                         {"576000",576000},
                                                                         {"921600",921600},
                                                                         {"1000000",1000000},
                                                                         };
            append_int(ret,"FC_UART_BAUD",ImprovedIntSetting(0,1000000,baud_rate_items),
                       "RPI HW UART baud rate, needs to match the UART baud rate set on your FC");
        }
        append_int(ret,"CONFIG_BOOT_AIR",ImprovedIntSetting::createEnumEnableDisable(),"DEV, change boot as air / ground",true);
        append_int(ret,"WIFI_HOTSPOT_E",ImprovedIntSetting::createEnum({"AUTO","ALWAYS_OFF","ALWAYS_ON"}),
                   "Enable/Disable WiFi hotspot such that you can connect to your air/ground unit via normal WiFi. Frequency is always the opposite of your WB link, e.g. "
                   "2.4G if your wb link is 5.8G and opposite. In AUTO (default), the wifi hotspot is automatically disarmed when you arm your FC (to avoid interference).");

        append_int(ret,"ETH_HOTSPOT_E",ImprovedIntSetting::createEnumEnableDisable(),
                   "Enable/Disable ethernet hotspot. When enabled, your rpi becomes a DHCPD server and starts forwarding video & telemetry if you connect a device via ethernet."
                   "However, this doesn't allow you to share your other device(s) internet with the RPI (e.g android) and in general, it is recommended to use ETH_PASSIVE_F instead."
                   "Cannot be used simultanoeusly with ETH_PASSIVE_F. Requires reboot.",true);
        append_int(ret,"ETH_PASSIVE_F",ImprovedIntSetting::createEnumEnableDisable(),
                   "Enable automatic video & telemetry forwarding via ethernet if EITH_HOTSPOT_E is false (Recommended)."
                   "Cannot be used simultaneously with ETH_HOTSPOT_E. Doesn't require reboot.",false);
        {
            auto values=std::vector<std::string>{"UNMANAGED","HOTSPOT","FORWARD+INTERNET"};
            append_int(ret,"ETHERNET",ImprovedIntSetting::createEnum(values),
                       "Specify the intended usage of the ground station ethernet port (for connecting external devices). A reboot might be required after changing this param.");
        }
        {
            auto values=std::vector<std::string>{"untouched","high","low"};
            append_int(ret,"GPIO_2",ImprovedIntSetting::createEnum(values),
                       "Experimental, allows manually controlling a rpi gpio for special uses like a LED, landing gear, ...");
            append_int(ret,"GPIO_26",ImprovedIntSetting::createEnum(values),
                       "Experimental, allows manually controlling a rpi gpio for special uses like a LED, landing gear, ...");
        }
        //
        {
            auto fc_uart_conn_values=std::vector<ImprovedStringSetting::Item>{
                {"DISABLE",""},
                {"DEFAULT","DEFAULT"},
                {"/dev/serial0","/dev/serial0"},
                {"/dev/serial1","/dev/serial1"},
                {"/dev/ttyS1","/dev/ttyS1"},
                {"/dev/ttyS2","/dev/ttyS2"},
                {"/dev/ttyUSB0","/dev/ttyUSB0"},
                {"/dev/ttyACM0","/dev/ttyACM0"},
                {"/dev/ttyACM1","/dev/ttyACM1"},
                {"/dev/ttyS7","/dev/ttyS7"},
                {"/dev/ttyAMA4","//dev/ttyAMA4"}
            };
            append_string(ret,"FC_UART_CONN",ImprovedStringSetting{fc_uart_conn_values},
                          "Telemetry FC<->Air unit. Make sure FC_UART_BAUD matches your FC. DEFAULT - primary telemetry serial of this platform (see wiki)."
                          "Otherwise, any linux serial fd filename (dev/testing).");
            //same for ground uart out
            append_string(ret,"TRACKER_UART_OUT",ImprovedStringSetting{fc_uart_conn_values},
                          "Enable mavlink telemetry out via UART on the ground station for connecting a tracker or even an RC with mavlink lua script.");
        }
        // Channel mapping presets for device(s)
        {
            /*auto values=std::vector<ImprovedStringSetting::Item>{
            {"default"       ,"0,1,2,3,4,5,6,7"},
            {"Steamdeck AETR","3,4,0,1,2,5,6,7"},
            {"Steamdeck EXP","0,1,3,4,2,5,6,7"},
            {"EX1",           "0,1,3,2,4,5,6,7"},
            {"EX2",           "0,1,4,5,2,3,6,7"},
            {"EX3",           "3,2,1,0,4,5,6,7"},
        };*/
            auto values=std::vector<ImprovedStringSetting::Item>{ // 4,5,1,2,3
                                                                   {"default"       ,"1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18"},
                                                                   {"Steamdeck AETR","4,5,1,2,3,6,7,8,9,10,11,12,13,14,15,16,17,18"},
                                                                   {"EX1"           ,"2,1,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18"},
                                                                   {"EX2"           ,"2,1,4,3,5,6,7,8,9,10,11,12,13,14,15,16,17,18"},
                                                                   };
            append_string(ret,"RC_CHAN_MAP",ImprovedStringSetting{values},
                          "Change which joystick 'channel' is taken for each RC channel. This is a list of numbers, where each number X at position N means take joystick input nr X for channel N."
                          " For example, 1,4,... means take channel number 1 for the first channel, and channel number 4 for the second channel. Needs to have ! all! 18 channel elements seperated by a ','");
        }
        {
            auto infiray_colorpalete_items=std::vector<ImprovedIntSetting::Item>{
                    {"CMD Shutter calibration",32768},
                    {"CMD YUYV output",32773},
                    {"CMD save configurations",33022},
                    // Color palete
                    {"White Hot",34816},
                    {"Black Hot",34817},
                    {"Iron Rainbow",34818},
                    {"Lava",34819},
                    {"Rainbow2",34820},
                    {"Rainbow3",34821},
                    {"Red Hot",34822},
                    {"Iron gray",34823},
                    {"HCR1",34824},
                    {"HCR2",34825},
                    {"Black hot 2",34826},
                                                                         };
            append_int(ret,"COLOR_PALETE",ImprovedIntSetting(34816,34826,infiray_colorpalete_items),
                       "Infiray thermal colors");
        }
        append_only_documented(ret,"RC_UPDATE_HZ",
                               "Specify the update rate of RC over wifibroadcast. A higher update rate gives lower RC latency, but takes more bandwidth away from the downlink."
                               "No effect if joy rc is disabled.");
    }
    {
        /*append_int(ret,"AUDIO_ENABLE",
                   ImprovedIntSetting::createEnumEnableDisable(),
                   "enables / disables audio streaming from air to ground. In development. Enabling automatically restarts the air unit !"
                   );*/
        auto audio_items=std::vector<ImprovedIntSetting::Item>{
         {"DISABLE",0},
         {"ENABLE",1},
         {"TEST",100},
         };
        append_int(ret,"AUDIO_ENABLE",ImprovedIntSetting(0,1000000,audio_items),
                "enables / disables audio streaming from air to ground. In development. Use test mode to validate your ground audio output."
                   );
    }
    return ret;
}

static std::map<std::string,std::shared_ptr<XParam>> create_param_map(){
    //qWarning("Create param map");
    auto param_list=get_parameters_list();
    //qWarning("X");
    std::map<std::string,std::shared_ptr<XParam>> ret;
    for(auto param:param_list){
        //qWarning("Y %s",param.param_name.c_str());
        if(ret.find(param->param_name)!=ret.end()){
            //qWarning("Param %s already exists !",param.param_name.c_str());
            assert(false);
        }
        ret[param->param_name]=param;
        //qDebug()<<"YY"<<param.param_name.c_str();
        //qWarning("Z");
    }
    //qDebug()<<"create_param_map()"<<tmp.size()<<", "<<tmp.size();
    return ret;
}

std::optional<DocumentedParam::XParam> DocumentedParam::find_param(const std::string &param_name)
{
    //qDebug()<<"find_param"<<param_name.c_str();
    // we use static to create a cache - we only read from the map anyways
    static std::map<std::string,std::shared_ptr<XParam>> cached=create_param_map();
    if(cached.find(param_name)!=cached.end()){
        auto ret=cached.at(param_name);
        //qDebug()<<"XXX Found "<<param_name.c_str()<<" "<<ret->param_name.c_str();
        return *ret;
    }else{
        //qDebug()<<"XXX Didn't find "<<param_name.c_str();
    }
    return std::nullopt;
}



bool DocumentedParam::read_only(const std::string &param_name)
{
    bool ret=false;
    const auto tmp=find_param(param_name);
    if(tmp.has_value()){
        ret=tmp.value().is_read_only;
    }
    //qDebug()<<"Param"<<param_id.c_str()<<"Read-only:"<<(ret==false ? "N":"Y");
    return ret;
}

std::optional<ImprovedIntSetting> DocumentedParam::get_improved_for_int(const std::string &param_id)
{
    const auto tmp=DocumentedParam::find_param(param_id);
    if(tmp.has_value()){
        const auto& param=tmp.value();
        if(param.improved_int.has_value()){
            return param.improved_int.value();
        }
    }
    return std::nullopt;
}

std::optional<ImprovedStringSetting> DocumentedParam::get_improved_for_string(const std::string param_id)
{
    const auto tmp=DocumentedParam::find_param(param_id);
    if(tmp.has_value()){
        const auto& param=tmp.value();
        if(param.improved_string.has_value()){
            return param.improved_string.value();
        }
    }
    return std::nullopt;
}

std::optional<std::string> DocumentedParam::int_param_to_enum_string_if_known(const std::string param_id, int value){
    const auto improved_opt=get_improved_for_int(param_id);
    if(improved_opt.has_value()){
        const auto& improved=improved_opt.value();
        if(improved.has_enum_mapping()){
            return improved.value_to_string(value);
        }
    }
    return std::nullopt;
}

std::optional<std::string> DocumentedParam::string_param_to_enum_string_if_known(const std::string param_id, std::string value){
    const auto improved_opt=get_improved_for_string(param_id);
    if(improved_opt.has_value()){
        const auto& improved=improved_opt.value();
        return improved.value_to_key(value);
    }
    return std::nullopt;
}

bool DocumentedParam::requires_reboot(const std::string &param_name)
{
    const auto tmp=DocumentedParam::find_param(param_name);
    if(tmp.has_value()){
        return tmp.value().requires_reboot;
    }
    return false;
}

std::string DocumentedParam::get_short_description(const std::string &param_name)
{
    const auto tmp=DocumentedParam::find_param(param_name);
    if(tmp.has_value()){
        return tmp.value().description.c_str();
    }
    return "TODO";
}

// ----------
static std::map<std::string, void *> get_whitelisted_params()
{
    std::map<std::string,void*> ret{};
    ret[openhd::WB_FREQUENCY]=nullptr;
    ret[openhd::WB_CHANNEL_WIDTH]=nullptr;
    ret[openhd::WB_MCS_INDEX]=nullptr;
    ret["CONFIG_BOOT_AIR"]=nullptr;
    ret[openhd::WB_MAX_FEC_BLOCK_SIZE_FOR_PLATFORM]=nullptr;
    ret[openhd::WB_DEV_AIR_SET_HIGH_RETRANSMIT_COUNT]=nullptr;
    // TX POWER Whitelisted, since they should be changed from the quick wizzard
    ret[openhd::WB_TX_POWER_MILLI_WATT]=nullptr;
    ret[openhd::WB_TX_POWER_MILLI_WATT_ARMED]=nullptr;
    ret[openhd::WB_RTL8812AU_TX_PWR_IDX_OVERRIDE]=nullptr;
    ret[openhd::WB_RTL8812AU_TX_PWR_IDX_ARMED]=nullptr;
    // STBC / LDPC - Whitelisted, since they should be changed from the quick wizzard
    ret[openhd::WB_ENABLE_LDPC]=nullptr;
    ret[openhd::WB_ENABLE_STBC]=nullptr;
    // Whitelisted since normally it should not be changed / has no effect anyways
    ret[openhd::WB_ENABLE_SHORT_GUARD]=nullptr;
    //
    ret["WB_V_FEC_PERC"]=nullptr;
    ret["WB_V_RATE_PERC"]=nullptr;
    ret["VARIABLE_BITRATE"]=nullptr;
    //
    ret["TYPE_CAM0"]=nullptr;
    ret["TYPE_CAM1"]=nullptr;
    // The actual 'camera' parameters
    // Whitelisted since r.n we don't really know
    // what happens on a platform if h265 is selected
    ret["VIDEO_CODEC"]=nullptr;
    ret["AIR_RECORDING"]=nullptr;
    ret["FORCE_SW_ENC"]=nullptr;
    ret["BITRATE_MBITS"]=nullptr;
    ret["KEYFRAME_I"]=nullptr;
    ret["INTRA_REFRESH"]=nullptr;
    ret["N_SLICES"]=nullptr;
    //

    ret["STREAMING_E"]=nullptr;


    ret[""]=nullptr;
    ret[""]=nullptr;
    ret[""]=nullptr;
    ret[""]=nullptr;
    //ret[""]=nullptr;
    return ret;
}

bool DocumentedParam::is_param_whitelisted(const std::string &param_id)
{
    const auto tmp=get_whitelisted_params();
    if(tmp.find(param_id)!=tmp.end()){
        return true;
    }
    return false;
}
