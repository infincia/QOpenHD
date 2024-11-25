#include "camerastreammodel.h"
#include "qdebug.h"
#include "../tutil/telemetryutil.hpp"
#include "../videostreaming/vscommon/QOpenHDVideoHelper.hpp"

#include <qsettings.h>
#include <regex>

#include <logging/hudlogmessagesmodel.h>
#include <logging/logmessagesmodel.h>

#include "openhd_core/camera.hpp"

static std::string video_codec_to_string(int value){
    if(value==0)return "h264";
    if(value==1)return "h265";
    return "Unknown";
}

CameraStreamModel::CameraStreamModel(int m_camera_index,QObject *parent)
    : QObject{parent},
      m_camera_index(m_camera_index)
{

}

CameraStreamModel &CameraStreamModel::instance(int cam_index)
{
    if(cam_index==0){
        static CameraStreamModel instancePrimary{0};
        return instancePrimary;
    }else if(cam_index==1){
        static CameraStreamModel instanceSecondary{1};
        return instanceSecondary;
    }
    assert(false);
}

QString CameraStreamModel::camera_type_to_string(int camera_type)
{
    if(camera_type<0)return "N/A";
    auto tmp=XCamera{camera_type,0,0};
    return tmp.cam_type_as_verbose_string().c_str();
}

QString CameraStreamModel::camera_status_to_string(int camera_status)
{
    if(camera_status<0)return "N/A";
    if(camera_status==0)return "unknown";
    if(camera_status==1)return "streaming";
    if(camera_status==2)return "restarting";
    return "ERROR";
}

QString CameraStreamModel::camera_codec_to_string(int camera_codec)
{
    if(camera_codec<0)return "ERROR";
    if(camera_codec==0)return "h264";
    if(camera_codec==1)return "h265";
    if(camera_codec==2)return "mjpeg";
    return "ERROR";
}

QString CameraStreamModel::camera_recording_mode_to_string(int recording_mode)
{
    if(recording_mode<0)return "                  n/a ";
    if(recording_mode==0)return "                 inactive ";
    if(recording_mode==1)return "                 active ";
    return "error";
}

bool CameraStreamModel::is_valid_resolution_fps_string(QString input)
{
    auto parsed=parse_video_format(input.toStdString());
    if(parsed.has_value())return true;
    return false;
}

QString CameraStreamModel::get_default_resolution()
{
    auto tmp=XCamera{m_camera_type,0,0};
    auto default_res_fps=tmp.get_default_resolution_fps();
    return default_res_fps.as_string().c_str();
}

QStringList CameraStreamModel::get_supported_resolutions()
{
    QStringList ret;
    auto tmp_cam=XCamera{m_camera_type,0,0};
    auto tmp=tmp_cam.get_supported_resolutions();
    for(auto& element:tmp){
        ret.push_back(element.as_string().c_str());
    }
    return ret;
}

QString CameraStreamModel::make_resolution_fps_verbose(QString input)
{
    auto parsed=parse_video_format(input.toStdString());
    if(parsed.has_value()){
        return get_verbose_string_of_resolution(parsed.value()).c_str();
    }
    qDebug()<<"WEIRD RESOLUTION"<<input;
    return input;
}

void CameraStreamModel::update_mavlink_openhd_stats_wb_video_air(const mavlink_openhd_stats_wb_video_air_t &msg)
{
    const auto curr_recommended_bitrate_kbits=msg.curr_recommended_bitrate;
    set_curr_recommended_bitrate_from_message(curr_recommended_bitrate_kbits);
    set_curr_video_measured_encoder_bitrate(Telemetryutil::bitrate_bps_to_qstring(msg.curr_measured_encoder_bitrate));
    set_curr_video_injected_bitrate(Telemetryutil::bitrate_bps_to_qstring(msg.curr_injected_bitrate));
    set_curr_video_injected_pps(Telemetryutil::pps_to_string(msg.curr_injected_pps));
    set_total_n_tx_dropped_frames(msg.curr_dropped_frames);
    if(m_last_tx_frame_drop_calculation_count<0){
        m_last_tx_frame_drop_calculation_count=msg.curr_dropped_frames;
    }else{
        const auto elapsed=std::chrono::steady_clock::now()-m_last_tx_frame_drop_calculation;
        if(elapsed>std::chrono::seconds(1)){
            const int diff=msg.curr_dropped_frames-m_last_tx_frame_drop_calculation_count;
            m_last_tx_frame_drop_calculation_count=msg.curr_dropped_frames;
            set_curr_delta_tx_dropped_frames(diff);
        }
    }

    if(msg.curr_recommended_bitrate>1 && msg.curr_measured_encoder_bitrate>1 ){ //check for valid measured / set values
        const double recommended_kbits=static_cast<float>(msg.curr_recommended_bitrate);
        // Measured and set encoder bitrate should match on a 20% basis
        const double max_perc_allowed=0.2;
        const auto max_kbit_okay=recommended_kbits * (1.0+max_perc_allowed);
        const auto min_kbit_okay=recommended_kbits * (1.0-max_perc_allowed);
        const double measured_kbits=msg.curr_measured_encoder_bitrate/1000.0;
        bool mismatch=false;
        if(measured_kbits <=min_kbit_okay){
           set_curr_set_and_measured_bitrate_mismatch(1);
           mismatch=true;
        }else if(measured_kbits >=max_kbit_okay){
           set_curr_set_and_measured_bitrate_mismatch(2);
           mismatch = true;
        }else{
           set_curr_set_and_measured_bitrate_mismatch(0);
        }
        if(mismatch){
           if(m_n_mismatch_has_been_logged<10){
               std::stringstream ss;
               ss<<"Set/ Measured encoder bitrate mismatch >"<<max_perc_allowed<<"%";
               LogMessagesModel::instanceOHDAir().add_message_warn("CAM",ss.str().c_str());
               m_n_mismatch_has_been_logged++;
           }
        }
    }
    set_curr_curr_fec_percentage(msg.curr_fec_percentage);
    set_air_tx_packets_per_second_and_bits_per_second(StringHelper::bitrate_and_pps_to_string(msg.curr_injected_bitrate,msg.curr_injected_pps).c_str());
}

void CameraStreamModel::update_mavlink_openhd_camera_status_air(const mavlink_openhd_camera_status_air_t &msg)
{
    //qDebug()<<"X:"<<(int)msg.cam_type;
    set_curr_curr_keyframe_interval(msg.encoding_keyframe_interval);
    set_air_recording_active(msg.air_recording_active);
    set_camera_type(msg.cam_type);
    set_encoding_codec(msg.encoding_format);
    {
        std::stringstream ss;
        ss<<(int)msg.stream_w<<"x"<<(int)msg.stream_h<<"@"<<msg.stream_fps;
        set_curr_set_video_format(ss.str().c_str());
        ss<<", "<<video_codec_to_string(msg.encoding_format);
        set_curr_set_video_format_and_codec(ss.str().c_str());
    }
    auto new_res_fps=ResolutionFramerate{msg.stream_w,msg.stream_h,msg.stream_fps};
    if(new_res_fps.is_any_invalid()){
        qDebug()<<"Invalid data from air unit:"<<resolution_framerate_to_string(new_res_fps).c_str();
    }else{
        if(m_curr_res_framerate.is_any_invalid()){
           // First time we ever got the current resolution / fps of the camera
           m_curr_res_framerate=new_res_fps;
        }else{
           if(m_curr_res_framerate!=new_res_fps){
               m_curr_res_framerate=new_res_fps;
               qDebug()<<"Res/Framerate changed:"<<resolution_framerate_to_string(m_curr_res_framerate).c_str();
           }
        }
    }
    // Feature - automatically set the right codec in qopenhd
    const bool secondary=m_camera_index==1;
    const int codec_in_qopenhd=QOpenHDVideoHelper::get_qopenhd_camera_video_codec(secondary);
    const int codec_in_openhd=msg.encoding_format;
    //qDebug()<<"Cam index:"<<(int)msg.cam_index<<" Codec: "<<codec_in_openhd;
    if(codec_in_openhd==0 || codec_in_openhd==1 || codec_in_openhd==2){
        if(codec_in_qopenhd!=codec_in_openhd){
           QOpenHDVideoHelper::set_qopenhd_camera_video_codec(secondary,codec_in_openhd);
           std::stringstream log;
           log<<"QOpenHD- set "<<(secondary ? "CAM2" : "CAM1")<<" to "<<video_codec_to_string(msg.encoding_format);
           HUDLogMessagesModel::instance().add_message_info(log.str().c_str());
        }
    }else{
        qDebug()<<"Invalid video codec: "<<codec_in_openhd;
    }
    set_camera_status(msg.cam_status);
}

void CameraStreamModel::update_mavlink_openhd_stats_wb_video_air_fec_performance(const mavlink_openhd_stats_wb_video_air_fec_performance_t &msg)
{
    set_curr_fec_encode_time_avg_min_max(
        Telemetryutil::us_min_max_avg_to_string(msg.curr_fec_encode_time_min_us,msg.curr_fec_encode_time_max_us,msg.curr_fec_encode_time_avg_us));
    set_curr_fec_block_length_min_max_avg(
        Telemetryutil::min_max_avg_to_string(msg.curr_fec_block_size_min,msg.curr_fec_block_size_max,msg.curr_fec_block_size_avg));
    set_curr_time_until_tx_min_max_avg(
        Telemetryutil::us_min_max_avg_to_string(msg.curr_tx_delay_min_us,msg.curr_tx_delay_max_us,msg.curr_tx_delay_avg_us));
}

void CameraStreamModel::update_mavlink_openhd_stats_wb_video_ground(const mavlink_openhd_stats_wb_video_ground_t &msg)
{
    set_curr_received_bitrate_with_fec(Telemetryutil::bitrate_bps_to_qstring(msg.curr_incoming_bitrate));
    set_count_blocks_lost(msg.count_blocks_lost);
    set_count_blocks_recovered(msg.count_blocks_recovered);
    set_count_fragments_recovered(msg.count_fragments_recovered);
    set_count_blocks_total(msg.count_blocks_total);
    //set_gnd_rx_packets_per_second_and_bits_per_second(StringHelper::bitrate_and_pps_to_string(msg.curr_incoming_bitrate,msg.cur_).c_str());
}

void CameraStreamModel::update_mavlink_openhd_stats_wb_video_ground_fec_performance(const mavlink_openhd_stats_wb_video_ground_fec_performance_t &msg)
{
    set_curr_fec_decode_time_avg_min_max(
        Telemetryutil::us_min_max_avg_to_string(msg.curr_fec_decode_time_min_us,msg.curr_fec_decode_time_max_us,msg.curr_fec_decode_time_avg_us));
}

void CameraStreamModel::set_curr_recommended_bitrate_from_message(const int64_t curr_recommended_bitrate_kbits)
{
    if(m_curr_recomended_video_bitrate_kbits!= 0 && curr_recommended_bitrate_kbits!=0 && m_curr_recomended_video_bitrate_kbits != curr_recommended_bitrate_kbits){
        QString message=m_camera_index==0 ? "Cam1 encoder:" : "Cam2 encoder:";
        message+=Telemetryutil::bitrate_kbits_to_qstring(curr_recommended_bitrate_kbits);
        HUDLogMessagesModel::instance().add_message_info(message);
    }
    set_curr_recomended_video_bitrate_kbits(curr_recommended_bitrate_kbits);
    if(curr_recommended_bitrate_kbits>0){
        set_curr_recomended_video_bitrate_string(Telemetryutil::bitrate_kbits_to_qstring(curr_recommended_bitrate_kbits));
    }else{
        set_curr_recomended_video_bitrate_string("N/A");
    }
}

std::string CameraStreamModel::resolution_framerate_to_string(const ResolutionFramerate &data)
{
    std::stringstream ss;
    ss<<(int)data.width<<"x"<<(int)data.height<<"@"<<data.framerate;
    return ss.str();
}

QVariantList CameraStreamModel::get_camera_choices(int platform_type)
{
    QList<QCameraNameAndType> list{
        QCameraNameAndType{"XLOL",1}
    };
    auto tmp= QManufacturerForPlatform{
        "LOL",
        list
    };
    QVariantList ret;
    return ret;
}

QStringList CameraStreamModel::get_manufacturer_choices(int platform_type)
{
    auto all_choices=get_camera_choices_for_platform(platform_type,m_camera_index!=0);
    QStringList ret;
    for(auto& choice: all_choices){
        ret.push_back(choice.manufacturer_name.c_str());
    }
    return ret;
}

QList<int> CameraStreamModel::get_manufacturer_cameras_type(int platform_type, int index)
{
    auto all_choices=get_camera_choices_for_platform(platform_type,m_camera_index!=0);
    QList<int> ret;
    if(index>=all_choices.size())index=0;
    for(auto& choice: all_choices[index].cameras){
        ret.push_back(choice.type);
    }
    return ret;
}

QStringList CameraStreamModel::get_manufacturer_cameras_names(int platform_type, int index)
{
    auto all_choices=get_camera_choices_for_platform(platform_type,m_camera_index!=0);
    QStringList ret;
    if(index>=all_choices.size())index=0;
    for(auto& choice: all_choices[index].cameras){
        ret.push_back(choice.name.c_str());
    }
    return ret;
}
