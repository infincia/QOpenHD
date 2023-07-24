#ifndef EXTERNALDECODESERVICE_H
#define EXTERNALDECODESERVICE_H

#include "QOpenHDVideoHelper.hpp"
#include "common/openhd-util.hpp"
#include "common/util_fs.h"
#include "decodingstatistcs.h"

#include <../logging/logmessagesmodel.h>
#include <../logging/hudlogmessagesmodel.h>
#include <thread>


//
// On some platforms, it is easiest to just start and stop a service that does the video decode (QOpenHD is then transparently layered on top)
// On rpi, this also gives by far the best performance / latency (but requires openhd mmal hacks)
// On ubuntu / x86, it is also quite usefully
//
namespace qopenhd::decode::service {

static void write_service_rotation_file(int rotation){
    qDebug()<<"Writing "<<rotation<<"° to video service file";
        FILE *f = fopen("/tmp/video_service_rotation.txt", "w");
    if (f == NULL){
        qDebug()<<"Error opening file!";
        return;
    }
    fprintf(f, "%d",rotation);
    fclose(f);
}

static constexpr auto GENERIC_H264_DECODE_SERVICE="h264_decode";
static constexpr auto GENERIC_H265_DECODE_SERVICE="h265_decode";
static constexpr auto GENERIC_MJPEG_DECODE_SERVICE="mjpeg_decode";
static std::string get_service_name_for_codec(const QOpenHDVideoHelper::VideoCodec& codec){
    if(codec==QOpenHDVideoHelper::VideoCodecH264)return GENERIC_H264_DECODE_SERVICE;
    if(codec==QOpenHDVideoHelper::VideoCodecH264)return GENERIC_H265_DECODE_SERVICE;
    return GENERIC_MJPEG_DECODE_SERVICE;
}

static void stop_service_if_exists(const std::string& name){
    if(util::fs::service_file_exists(name)){
        OHDUtil::run_command("systemctl stop",{name});
    }
}

static void start_service_if_exists(const std::string& name){
    if(util::fs::service_file_exists(name)){
        OHDUtil::run_command("systemctl start",{name});
    }else{
        std::stringstream message;
        message<<"Cannot start video decode service "<<name;
        LogMessagesModel::instanceOHD().add_message_warn("QOpenHD",message.str().c_str());
        HUDLogMessagesModel::instance().add_message_warning(message.str().c_str());
        qDebug()<<message.str().c_str();
    }
}

static void stop_all_services(){
    std::vector<std::string> services{GENERIC_H264_DECODE_SERVICE,GENERIC_H265_DECODE_SERVICE,GENERIC_MJPEG_DECODE_SERVICE};
    for(const auto& service:services){
        stop_service_if_exists(service);
    }
}

static bool qopenhd_is_background_transparent(){
    QSettings settings;
    return settings.value("app_background_transparent",true).toBool();

}

void decode_via_external_decode_service(const QOpenHDVideoHelper::VideoStreamConfig& settings,std::atomic<bool>& request_restart){
    qDebug()<<"dirty_generic_decode_via_external_decode_service begin";
    // this is always for primary video, unless switching is enabled
    auto stream_config=settings.primary_stream_config;
    if(settings.generic.qopenhd_switch_primary_secondary){
        stream_config=settings.secondary_stream_config;
    }
    // Stop any still running service (just in case there is one)
    stop_all_services();
    // QRS are not available with this implementation
    DecodingStatistcs::instance().reset_all_to_default();
    {
        std::stringstream type;
        type<<"External "<<QOpenHDVideoHelper::video_codec_to_string(settings.primary_stream_config.video_codec);
        DecodingStatistcs::instance().set_decoding_type(type.str().c_str());
    }
    // Background should be transparent
    if(!qopenhd_is_background_transparent()){
        HUDLogMessagesModel::instance().add_message_warning("Background not transparent !");
        HUDLogMessagesModel::instance().add_message_warning("Make transparent (Screen Settings) and restart !");
    }

    // Dirty way we communicate with the service / executable
    const auto rotation=QOpenHDVideoHelper::get_display_rotation();
    write_service_rotation_file(rotation);
    // Start service
    const auto service_name=get_service_name_for_codec(settings.primary_stream_config.video_codec);
    start_service_if_exists(service_name);
    // We follow the same pattern here as the decode flows that don't use an "external service"
    while(true){
        if(request_restart){
            request_restart=false;
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    // Stop service
    OHDUtil::run_command("systemctl stop",{service_name});
    qDebug()<<"dirty_generic_decode_via_external_decode_service end";
}

}

#endif // EXTERNALDECODESERVICE_H
