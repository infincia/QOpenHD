#ifndef QOPENHDVIDEOHELPER_H
#define QOPENHDVIDEOHELPER_H

#include <QSettings>
#include <qqmlapplicationengine.h>
#include <qquickitem.h>
#include <qquickwindow.h>

#include <sstream>
#include <fstream>
#include <qfileinfo.h>

namespace QOpenHDVideoHelper{

// Must be in sync with OpenHD
static constexpr auto kDefault_udp_rtp_input_ip_address="127.0.0.1";
static constexpr auto kDefault_udp_rtp_input_port_primary=5600;
static constexpr auto kDefault_udp_rtp_input_port_secondary=5601;

// OpenHD supported video codecs
typedef enum VideoCodec {
  VideoCodecH264=0,
  VideoCodecH265=1,
  VideoCodecMJPEG=2
} VideoCodec;
static VideoCodec intToVideoCodec(int videoCodec){
    if(videoCodec==0)return VideoCodecH264;
    if(videoCodec==1)return VideoCodecH265;
    if(videoCodec==2)return VideoCodecMJPEG;
    qDebug() << "VideoCodec::intToVideoCodec::somethingWrong,using H264 as default";
    return VideoCodecH264;
}
static std::string video_codec_to_string(const VideoCodec& codec){
    if(codec==VideoCodecH264)return "h264";
    if(codec==VideoCodecH265)return "h265";
    return "mjpeg";
}

enum class VideoTestMode{
    DISABLED, // disabled
    RAW_VIDEO, // raw video into qmlglsink. Doesn't check video decoding capabilities, only qmlsink capabilities
    RAW_VIDEO_ENCODE_DECODE // encode raw video, then decode it and then into qmlsink. When this passes, platform can do video decoding
};
static VideoTestMode videoTestModeFromInt(int value){
    if(value==1)return VideoTestMode::RAW_VIDEO;
    if(value==2)return VideoTestMode::RAW_VIDEO_ENCODE_DECODE;
    return VideoTestMode::DISABLED;
}

/**
 * Dirty - settings mostly for developing / handling differen platforms
 * Not seperated for primary / secondary stream
 */
struct GenericVideoSettings{
    // used for testing and development.
    VideoTestMode dev_test_video_mode = VideoTestMode::DISABLED;
    //
    bool dev_enable_custom_pipeline=false;
    std::string dev_custom_pipeline="";
    //
    int dev_limit_fps_on_test_file=-1;
    bool dev_use_low_latency_parser_when_possible=true;
    // feed incomplete frame(s) to the decoder, in contrast to only only feeding intact frames,
    // but not caring about missing previous frames
    bool dev_feed_incomplete_frames_to_decoder=false;
    // argh, is the only thing I say here
    bool dev_rpi_use_external_omx_decode_service=true;
    // platforms other than RPI
    bool dev_always_use_generic_external_decode_service=false;
    // On embedded devices, video is commonly rendered on a special surface, independent of QOpenHD
    // r.n only the rpi mmal impl. supports proper video rotation
    int extra_screen_rotation=0;
    bool qopenhd_switch_primary_secondary=false;

    // 2 configs are equal if all members are exactly the same.
    bool operator==(const GenericVideoSettings &o) const {
       return this->dev_test_video_mode == o.dev_test_video_mode &&
               this->dev_enable_custom_pipeline==o.dev_enable_custom_pipeline &&
               this->dev_custom_pipeline==o.dev_custom_pipeline &&
               this->dev_limit_fps_on_test_file == o.dev_limit_fps_on_test_file &&
               this->dev_use_low_latency_parser_when_possible == o.dev_use_low_latency_parser_when_possible &&
               this->dev_feed_incomplete_frames_to_decoder == o.dev_feed_incomplete_frames_to_decoder &&
               this->dev_rpi_use_external_omx_decode_service==o.dev_rpi_use_external_omx_decode_service &&
               this->dev_always_use_generic_external_decode_service==o.dev_always_use_generic_external_decode_service &&
               this->extra_screen_rotation == o.extra_screen_rotation &&
               this->qopenhd_switch_primary_secondary == o.qopenhd_switch_primary_secondary;
     }
    bool operator !=(const GenericVideoSettings &o) const {
        return !(*this==o);
    }
};

/**
 * @brief Settings for the primary / secondary video stream
 */
struct VideoStreamConfigXX{
    // the ip address where we receive udp rtp video data from
    std::string udp_rtp_input_ip_address=kDefault_udp_rtp_input_ip_address;
    // the port where to receive udp rtp video data from
    int udp_rtp_input_port = kDefault_udp_rtp_input_port_primary;
    // the video codec the received rtp data should be intepreted as.
    VideoCodec video_codec=VideoCodecH264;
    // force sw decoding (only makes a difference if on this platform/compile-time configuration a HW decoder is chosen by default)
    bool enable_software_video_decoder=false;

    // 2 configs are equal if all members are exactly the same.
    bool operator==(const VideoStreamConfigXX &o) const {
        return this->udp_rtp_input_port == o.udp_rtp_input_port && this->video_codec== o.video_codec
               && this->enable_software_video_decoder==o.enable_software_video_decoder &&
               this->udp_rtp_input_ip_address==o.udp_rtp_input_ip_address;
    }
    bool operator !=(const VideoStreamConfigXX &o) const {
        return !(*this==o);
    }
};

struct VideoStreamConfig{
    GenericVideoSettings generic;
    VideoStreamConfigXX primary_stream_config;
    VideoStreamConfigXX secondary_stream_config;
    bool operator==(const VideoStreamConfig &o) const {
        return this->generic == o.generic &&
               this->primary_stream_config==o.primary_stream_config &&
               this->secondary_stream_config==o.secondary_stream_config;
    }
    bool operator !=(const VideoStreamConfig &o) const {
        return !(*this==o);
    }
};


static VideoStreamConfigXX read_from_settingsXX(bool is_primary){
    QSettings settings;
    QOpenHDVideoHelper::VideoStreamConfigXX _videoStreamConfig;
    if(is_primary){
        _videoStreamConfig.udp_rtp_input_port=settings.value("qopenhd_primary_video_rtp_input_port", kDefault_udp_rtp_input_port_primary).toInt();
        _videoStreamConfig.udp_rtp_input_ip_address=settings.value("qopenhd_primary_video_rtp_input_ip",kDefault_udp_rtp_input_ip_address).toString().toStdString();
        const int tmp_video_codec = settings.value("qopenhd_primary_video_codec", 0).toInt();
        _videoStreamConfig.video_codec=QOpenHDVideoHelper::intToVideoCodec(tmp_video_codec);
        _videoStreamConfig.enable_software_video_decoder=settings.value(" qopenhd_primary_video_force_sw", 0).toBool();
    }else{
        _videoStreamConfig.udp_rtp_input_port=settings.value("qopenhd_secondary_video_rtp_input_port", kDefault_udp_rtp_input_port_secondary).toInt();
        _videoStreamConfig.udp_rtp_input_ip_address=settings.value("qopenhd_secondary_video_rtp_input_ip",kDefault_udp_rtp_input_ip_address).toString().toStdString();
        const int tmp_video_codec = settings.value("qopenhd_secondary_video_codec", 0).toInt();
        _videoStreamConfig.video_codec=QOpenHDVideoHelper::intToVideoCodec(tmp_video_codec);
        _videoStreamConfig.enable_software_video_decoder=settings.value(" qopenhd_secondary_video_force_sw", 0).toBool();
    }
    return _videoStreamConfig;
}

// Kinda UI, kinda video related
static int get_display_rotation(){
    QSettings settings{};
    return settings.value("general_screen_rotation", 0).toInt();
}
// See setting description
// do not preserve aspect ratio of primary video
// default false (do preserve video aspect ratio)
static bool get_primary_video_scale_to_fit(){
    QSettings settings{};
    return settings.value("primary_video_scale_to_fit", false).toBool();
}

static GenericVideoSettings read_generic_from_settings(){
    QSettings settings;
    GenericVideoSettings _videoStreamConfig;
    _videoStreamConfig.dev_test_video_mode=QOpenHDVideoHelper::videoTestModeFromInt(settings.value("dev_test_video_mode", 0).toInt());

    _videoStreamConfig.dev_enable_custom_pipeline=settings.value("dev_enable_custom_pipeline",false).toBool();
    _videoStreamConfig.dev_limit_fps_on_test_file=settings.value("dev_limit_fps_on_test_file",-1).toInt();
    _videoStreamConfig.dev_use_low_latency_parser_when_possible=settings.value("dev_use_low_latency_parser_when_possible",true).toBool();
    //
    _videoStreamConfig.dev_rpi_use_external_omx_decode_service=settings.value("dev_rpi_use_external_omx_decode_service", true).toBool();
    _videoStreamConfig.dev_always_use_generic_external_decode_service=settings.value("dev_always_use_generic_external_decode_service", false).toBool();
    _videoStreamConfig.qopenhd_switch_primary_secondary=settings.value("qopenhd_switch_primary_secondary", false).toBool();
    _videoStreamConfig.extra_screen_rotation=get_display_rotation();
    // QML text input sucks, so we read a file. Not ideal, but for testing only anyways
    {
        _videoStreamConfig.dev_custom_pipeline="";
        if(_videoStreamConfig.dev_enable_custom_pipeline){
            std::ifstream file("/usr/local/share/qopenhd/custom_pipeline.txt");
            if(file.is_open()){
                std::stringstream buffer;
                buffer << file.rdbuf();
                _videoStreamConfig.dev_custom_pipeline=buffer.str();
            }else{
                qDebug()<<"dev_enable_custom_pipeline but no file";
            }
        }
    }
    //_videoStreamConfig.dev_custom_pipeline=settings.value("dev_custom_pipeline","").toString().toStdString();
    return _videoStreamConfig;
}
static VideoStreamConfig read_config_from_settings(){
    VideoStreamConfig ret;
    ret.generic=read_generic_from_settings();
    ret.primary_stream_config=read_from_settingsXX(true);
    ret.secondary_stream_config=read_from_settingsXX(false);
    return ret;
}

// For OpenHD images, these files are copied over by the image builder and therefore can
// be used for testing / validation
static std::string get_default_openhd_test_file(const VideoCodec video_codec) {
    std::stringstream in_filename;
#ifdef _WIN32
    // Use Windows path
    in_filename << "C:\\\\testvideos\\\\";
#else
    // Use Linux path
    in_filename << "/usr/local/share/testvideos/";
#endif

    if (video_codec == QOpenHDVideoHelper::VideoCodecH264) {
        in_filename << "rpi_1080.h264";
    } else if (video_codec == QOpenHDVideoHelper::VideoCodecH265) {
        in_filename << "jetson_test.h265";
    } else {
        in_filename << "uv_640x480.mjpeg";
    }

    return in_filename.str();
}

// FFMPEG needs a ".sdp" file to do rtp udp h264,h265 or mjpeg
// For MJPEG we map mjpeg to 26 (mjpeg), for h264/5 we map h264/5 to 96 (general)
static std::string create_udp_rtp_sdp_file(const VideoStreamConfigXX& video_stream_config){
    std::stringstream ss;
    ss<<"c=IN IP4 "<<video_stream_config.udp_rtp_input_ip_address<<"\n";
    //ss<<"v=0\n";
    //ss<<"t=0 0\n";
    //ss<<"width=1280\n";
    //ss<<"height=720\n";
    if(video_stream_config.video_codec==QOpenHDVideoHelper::VideoCodec::VideoCodecMJPEG){
         ss<<"m=video "<<video_stream_config.udp_rtp_input_port<<" RTP/UDP 26\n";
        ss<<"a=rtpmap:26 JPEG/90000\n";
    }else{
        ss<<"m=video "<<video_stream_config.udp_rtp_input_port<<" RTP/UDP 96\n";
        if(video_stream_config.video_codec==QOpenHDVideoHelper::VideoCodec::VideoCodecH264){
            ss<<"a=rtpmap:96 H264/90000\n";
        }else{
            assert(video_stream_config.video_codec==QOpenHDVideoHelper::VideoCodec::VideoCodecH265);
            ss<<"a=rtpmap:96 H265/90000\n";
        }
    }
    return ss.str();
}
static void write_file_to_tmp(const std::string filename,const std::string content){
    std::ofstream _t(filename);
    _t << content;
    _t.close();
}
static constexpr auto kRTP_FILENAME="/tmp/rtp_custom.sdp";

static void write_udp_rtp_sdp_file_to_tmp(const VideoStreamConfigXX& video_stream_config){
    write_file_to_tmp(kRTP_FILENAME,create_udp_rtp_sdp_file(video_stream_config));
}
static std::string get_udp_rtp_sdp_filename(const VideoStreamConfigXX& video_stream_config){
    write_udp_rtp_sdp_file_to_tmp(video_stream_config);
    return kRTP_FILENAME;
}

static int get_qopenhd_n_cameras(){
    QSettings settings;
    const int tmp = settings.value("dev_qopenhd_n_cameras", 1).toInt();
    return tmp;
}

// We autmatically (over) write the video codec once we get camera telemetry data
static int get_qopenhd_camera_video_codec(bool secondary){
    QSettings settings;
    int codec_in_qopenhd=0;
    if(secondary){
        codec_in_qopenhd = settings.value("qopenhd_secondary_video_codec", 0).toInt();
    }else{
        codec_in_qopenhd = settings.value("qopenhd_primary_video_codec", 0).toInt();
    }
    return codec_in_qopenhd;
}
static void set_qopenhd_camera_video_codec(bool secondary,int codec){
    QSettings settings;
    int codec_in_qopenhd=0;
    if(secondary){
        settings.setValue("qopenhd_secondary_video_codec",(int)codec);
    }else{
        settings.setValue("qopenhd_primary_video_codec",(int)codec);
    }
}
// We set qopenhd switch primary and secondary to false when started -
// This way, we avoid confusion where an user might have switched, then disabled dualcam,
// and is now getting "no video" since there is no secondary camera
static void reset_qopenhd_switch_primary_secondary(){
    QSettings settings;
    settings.setValue("qopenhd_switch_primary_secondary",false);

}

}

#endif // QOPENHDVIDEOHELPER_H
