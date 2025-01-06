#include "avcodec_decoder.h"
#include "qdebug.h"
#include <QFileInfo>
#include <iostream>
#include <sstream>

#include "avcodec_helper.hpp"
#include "common/TimeHelper.hpp"
#include "common/util_fs.h"

#include "texturerenderer.h"
#include "decodingstatistcs.h"
#include "common/SchedulingHelper.hpp"
#include "logging/hudlogmessagesmodel.h"
#include "logging/logmessagesmodel.h"

#include "ExternalDecodeService.hpp"

static int hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type) {
    int err = 0;
    ctx->hw_frames_ctx = NULL;
    if ((err = av_hwdevice_ctx_create(&ctx->hw_device_ctx, type, NULL, NULL, 0)) < 0) {
        char errbuf[128];
        av_strerror(err, errbuf, sizeof(errbuf));
        fprintf(stderr, "Failed to create HW device (%s): %s\n", av_hwdevice_get_type_name(type), errbuf);
        return err;
    }
    qDebug() << "HW device created: " << av_hwdevice_get_type_name(type);
    return err;
}

static enum AVPixelFormat wanted_hw_pix_fmt;
static enum AVPixelFormat get_hw_format(AVCodecContext *ctx,const enum AVPixelFormat *pix_fmts){
    const enum AVPixelFormat *p;
    AVPixelFormat ret=AV_PIX_FMT_NONE;
    std::stringstream supported_formats;
    for (p = pix_fmts; *p != -1; p++) {
        const int tmp=(int)*p;
        supported_formats<<safe_av_get_pix_fmt_name(*p)<<"("<<tmp<<"),";
        if (*p == wanted_hw_pix_fmt){
          // matches what we want
          ret=*p;
        }
    }
    qDebug()<<"Supported (HW) pixel formats: "<<supported_formats.str().c_str();
    if(ret==AV_PIX_FMT_NONE){
      fprintf(stderr, "Failed to get HW surface format. Wanted: %s\n", av_get_pix_fmt_name(wanted_hw_pix_fmt));
    }
    return ret;
}

// For SW decode, we support YUV420 | YUV422 and their (mjpeg) abbreviates since
// we can always copy and render these formats via OpenGL - and when we are doing SW decode
// we most likely are on a (fast) x86 platform where we can copy those formats via CPU
// relatively easily, at least the resolutions common in OpenHD
static enum AVPixelFormat get_sw_format(AVCodecContext *ctx,const enum AVPixelFormat *pix_fmts){
    const enum AVPixelFormat *p;
    qDebug()<<"All (SW) pixel formats:"<<all_formats_to_string(pix_fmts).c_str();
    for (p = pix_fmts; *p != -1; p++) {
        const AVPixelFormat tmp=*p;
        if(tmp==AV_PIX_FMT_YUV420P || tmp==AV_PIX_FMT_YUV422P || tmp==AV_PIX_FMT_YUVJ422P || tmp==AV_PIX_FMT_YUVJ420P){
            return tmp;
        }
    }
    qDebug()<<"Weird, we should be able to do SW decoding on all platforms";
    return AV_PIX_FMT_NONE;
}


AVCodecDecoder::AVCodecDecoder(QObject *parent):
    QObject(parent)
{
    //drm_prime_out=std::make_unique<DRMPrimeOut>(1,false,false);
}

AVCodecDecoder::~AVCodecDecoder()
{
    terminate();
}

void AVCodecDecoder::init(bool primaryStream)
{
    qDebug() << "AVCodecDecoder::init()";
    m_last_video_settings=QOpenHDVideoHelper::read_config_from_settings();
    decode_thread = std::make_unique<std::thread>([this]{this->constant_decode();} );
    timer_check_settings_changed=std::make_unique<QTimer>();
    QObject::connect(timer_check_settings_changed.get(), &QTimer::timeout, this, &AVCodecDecoder::timer_check_settings_changed_callback);
    timer_check_settings_changed->start(1000);
}

void AVCodecDecoder::terminate()
{
    // Stop the timer, which can be done (almost) immediately (it's runnable doesn't block)
    timer_check_settings_changed->stop();
    timer_check_settings_changed=nullptr;
    // This will stop the constant_decode as soon as the current running decode_until_error loop returns
    m_should_terminate=true;
    // This will break out of a running "decode until error" loop if there is one currently running
    request_restart=true;
    if(decode_thread){
        // Wait for everything to cleanup and stop
        decode_thread->join();
    }
    qDebug()<<"Decoding terminated";
}

void AVCodecDecoder::timer_check_settings_changed_callback()
{
    const auto new_settings=QOpenHDVideoHelper::read_config_from_settings();
    if(m_last_video_settings!=new_settings){
        // We just request a restart from the video (break out of the current constant_decode() loop,
        // and restart with the new settings.
        request_restart=true;
        m_last_video_settings=new_settings;
    }
}

void AVCodecDecoder::constant_decode()
{
    while(!m_should_terminate){
        qDebug()<<"Start decode";
        const auto settings = QOpenHDVideoHelper::read_config_from_settings();
        // this is always for primary video, unless switching is enabled
        auto stream_config=settings.primary_stream_config;
        if(settings.generic.qopenhd_switch_primary_secondary){
            stream_config=settings.secondary_stream_config;
        }
         bool do_custom_rtp=settings.generic.dev_use_low_latency_parser_when_possible;
         if(stream_config.video_codec==QOpenHDVideoHelper::VideoCodecMJPEG){
             // we got no support for mjpeg in our custom rtp parser
             do_custom_rtp=false;
        }
        // On a couple of embedded platform(s) we do not do the decoding in qopenhd,
        // but by using a "decode service" that renders / composes the video into a plane behind qopenhd
        // on rpi, this is by far the most performant / low latency option
        bool use_external_decode_service=false;
        // choice - enable regardless of platform, usefull for development
        if(settings.generic.dev_always_use_generic_external_decode_service){
            use_external_decode_service=true;
        }
        bool is_rpi=false;
#ifdef IS_PLATFORM_RPI
        is_rpi=true;
#endif // IS_PLATFORM_RPI
        if(is_rpi && settings.generic.dev_rpi_use_external_omx_decode_service){
            use_external_decode_service=true;
        }
        if(use_external_decode_service){
            dirty_generic_decode_via_external_decode_service(settings);
        }else{
            if(settings.generic.dev_test_video_mode!=QOpenHDVideoHelper::VideoTestMode::DISABLED){
                // file playback always goes via non-custom rtp parser (since it is not rtp)
                do_custom_rtp=false;
            }
            if(do_custom_rtp){
                // Does h264 and h265 custom rtp parse, but uses avcodec for decode
                open_and_decode_until_error_custom_rtp(settings);
            }else{
                open_and_decode_until_error(settings);
            }
        }
        qDebug()<<"Decode stopped,restarting";
        // wait a bit before starting again, to save CPU
        if(!m_should_terminate)std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}


int AVCodecDecoder::decode_and_wait_for_frame(AVPacket *packet,std::optional<std::chrono::steady_clock::time_point> parse_time)
{
    AVFrame *frame = nullptr;
    //qDebug()<<"Decode packet:"<<packet->pos<<" size:"<<packet->size<<" B";
    const auto beforeFeedFrame=std::chrono::steady_clock::now();
    if(parse_time!=std::nullopt){
        const auto delay=beforeFeedFrame-parse_time.value();
        avg_parse_time.add(delay);
        avg_parse_time.custom_print_in_intervals(std::chrono::seconds(3),[](const std::string name,const std::string message){
            //qDebug()<<name.c_str()<<":"<<message.c_str();
            DecodingStatistcs::instance().set_parse_and_enqueue_time(message.c_str());
        });
    }
    const auto beforeFeedFrameUs=getTimeUs();
    packet->pts=beforeFeedFrameUs;
    timestamp_add_fed(packet->pts);

    //m_ffmpeg_dequeue_or_queue_mutex.lock();
    const int ret_avcodec_send_packet = avcodec_send_packet(decoder_ctx, packet);
    //m_ffmpeg_dequeue_or_queue_mutex.unlock();
    if (ret_avcodec_send_packet < 0) {
        fprintf(stderr, "Error during decoding\n");
        return ret_avcodec_send_packet;
    }
    // alloc output frame(s)
    if (!(frame = av_frame_alloc())) {
        // NOTE: It is a common practice to not care about OOM, and this is the best approach in my opinion.
        // but ffmpeg uses malloc and returns error codes, so we keep this practice here.
        qDebug()<<"can not alloc frame";
        av_frame_free(&frame);
        return AVERROR(ENOMEM);
    }
    int ret=0;
    // Poll until we get the frame out
    const auto loopUntilFrameBegin=std::chrono::steady_clock::now();
    bool gotFrame=false;
    int n_times_we_tried_getting_a_frame_this_time=0;
    while (!gotFrame){
        //m_ffmpeg_dequeue_or_queue_mutex.lock();
        ret = avcodec_receive_frame(decoder_ctx, frame);
        //m_ffmpeg_dequeue_or_queue_mutex.unlock();
        if(ret == AVERROR_EOF){
            qDebug()<<"Got EOF";
            break;
        }else if(ret==0){
            //debug_is_valid_timestamp(frame->pts);
            // we got a new frame
            if(!use_frame_timestamps_for_latency){
                const auto x_delay=std::chrono::steady_clock::now()-beforeFeedFrame;
                //qDebug()<<"(True) decode delay(wait):"<<((float)std::chrono::duration_cast<std::chrono::microseconds>(x_delay).count()/1000.0f)<<" ms";
                avg_decode_time.add(x_delay);
            }else{
                const auto now_us=getTimeUs();
                const auto delay_us=now_us-frame->pts;
                //qDebug()<<"(True) decode delay(nowait):"<<((float)delay_us/1000.0f)<<" ms";
                //MLOGD<<"Frame pts:"<<frame->pts<<" Set to:"<<now<<"\n";
                //frame->pts=now;
                avg_decode_time.add(std::chrono::microseconds(delay_us));
            }
            gotFrame=true;
            frame->pts=beforeFeedFrameUs;
            // display frame
            on_new_frame(frame);
            avg_decode_time.custom_print_in_intervals(std::chrono::seconds(3),[](const std::string name,const std::string message){
                //qDebug()<<name.c_str()<<":"<<message.c_str();
                DecodingStatistcs::instance().set_decode_time(message.c_str());
            });
        }else if(ret==AVERROR(EAGAIN)){
            // TODO FIXME REMOVE
            if(true){
                break;
            }
            if(n_no_output_frame_after_x_seconds>=2){
                // note decode latency is now wrong
                //qDebug()<<"Skipping decode lockstep due to no frame for more than X seconds\n";
                DecodingStatistcs::instance().set_doing_wait_for_frame_decode("No");
                if(n_times_we_tried_getting_a_frame_this_time>4){
                    break;
                }
            }
            //std::cout<<"avcodec_receive_frame returned:"<<ret<<"\n";
            // for some video files, the decoder does not output a frame every time a h264 frame has been fed
            // In this case, I unblock after X seconds, but we cannot measure the decode delay by using the before-after
            // approach. We can still measure it using the pts timestamp from av, but this one cannot necessarily be trusted 100%
            if(std::chrono::steady_clock::now()-loopUntilFrameBegin > std::chrono::seconds(2)){
              qDebug()<<"Got no frame after X seconds. Break, but decode delay will be reported wrong";
              n_no_output_frame_after_x_seconds++;
              use_frame_timestamps_for_latency=true;
              break;
            }
            // sleep a bit to not hog the CPU too much
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }else{
            qDebug()<<"Got unlikely / weird error:"<<ret;
            break;
        }
        n_times_we_tried_getting_a_frame_this_time++;
    }
    av_frame_free(&frame);
    return 0;
}

int AVCodecDecoder::decode_config_data(AVPacket *packet)
{
     const int ret_avcodec_send_packet = avcodec_send_packet(decoder_ctx, packet);
     return ret_avcodec_send_packet;
}


bool AVCodecDecoder::feed_rtp_frame_if_available()
{
    auto frame=m_rtp_receiver->get_next_frame();
    if(frame){
        {
            // parsing delay
            const auto delay=std::chrono::steady_clock::now()-frame->get_nal().creationTime;
            avg_parse_time.add(delay);
            avg_parse_time.custom_print_in_intervals(std::chrono::seconds(3),[](const std::string name,const std::string message){
                //qDebug()<<name.c_str()<<":"<<message.c_str();
                DecodingStatistcs::instance().set_parse_and_enqueue_time(message.c_str());
            });
        }
        AVPacket *pkt=av_packet_alloc();
        pkt->data=(uint8_t*)frame->get_nal().getData();
        pkt->size=frame->get_nal().getSize();
        const auto beforeFeedFrameUs=getTimeUs();
        pkt->pts=beforeFeedFrameUs;
        timestamp_add_fed(pkt->pts);
        avcodec_send_packet(decoder_ctx, pkt);
        av_packet_free(&pkt);
        return true;
    }
    return false;
}

void AVCodecDecoder::fetch_frame_or_feed_input_packet(){
    AVPacket *pkt=av_packet_alloc();
    bool keep_fetching_frames_or_input_packets=true;
    while(keep_fetching_frames_or_input_packets){
        if(request_restart){
            keep_fetching_frames_or_input_packets=false;
            request_restart=false;
            continue;
        }
        AVFrame* frame= av_frame_alloc();
        assert(frame);
        const int ret = avcodec_receive_frame(decoder_ctx, frame);
        //m_ffmpeg_dequeue_or_queue_mutex.unlock();
        if(ret == AVERROR_EOF){
            qDebug()<<"Got EOF";
            keep_fetching_frames_or_input_packets=false;
        }else if(ret==0){
            timestamp_debug_valid(frame->pts);
            // we got a new frame
            const auto now_us=getTimeUs();
            const auto delay_us=now_us-frame->pts;
            //qDebug()<<"(True) decode delay(nowait):"<<((float)delay_us/1000.0f)<<" ms";
            //frame->pts=now;
            avg_decode_time.add(std::chrono::microseconds(delay_us));
            // display frame
            on_new_frame(frame);
            avg_decode_time.custom_print_in_intervals(std::chrono::seconds(3),[](const std::string name,const std::string message){
                 //qDebug()<<name.c_str()<<":"<<message.c_str();
                 DecodingStatistcs::instance().set_decode_time(message.c_str());
            });
            av_frame_free(&frame);
            frame= av_frame_alloc();
        }else if(ret==AVERROR(EAGAIN)){
            //qDebug()<<"Needs more data";
            // Get more encoded data
            const bool success=feed_rtp_frame_if_available();
            if(!success){
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }else{
            qDebug()<<"Weird decoder error:"<<ret;
            keep_fetching_frames_or_input_packets=false;
        }
    }
    av_packet_free(&pkt);
}

void AVCodecDecoder::on_new_frame(AVFrame *frame)
{
    {
        std::stringstream ss;
        ss<<safe_av_get_pix_fmt_name((AVPixelFormat)frame->format)<<" "<<frame->width<<"x"<<frame->height;
        DecodingStatistcs::instance().set_primary_stream_frame_format(QString(ss.str().c_str()));
        //qDebug()<<"Got frame:"<<ss.str().c_str();
    }
    // Once we got the first frame, reduce the log level
    av_log_set_level(AV_LOG_WARNING);
    //qDebug()<<debug_frame(frame).c_str();
    TextureRenderer::instance().queue_new_frame_for_display(frame);
    if(last_frame_width==-1 || last_frame_height==-1){
        last_frame_width=frame->width;
        last_frame_height=frame->height;
    }else{
        if(last_frame_width!=frame->width || last_frame_height!=frame->height){
            // PI and SW decoer will just slently start outputting garbage frames
            // if the width/ height changes during RTP streaming
            qDebug()<<"Need to restart the decoder, width / heght changed";
            request_restart=true;
        }
    }
    //drm_prime_out->queue_new_frame_for_display(frame);
}

void AVCodecDecoder::reset_before_decode_start()
{
    n_no_output_frame_after_x_seconds=0;
    last_frame_width=-1;
    last_frame_height=-1;
    avg_decode_time.reset();
    avg_parse_time.reset();
    DecodingStatistcs::instance().reset_all_to_default();
    last_frame_width=-1;
    last_frame_height=-1;
    m_fed_timestamps_queue.clear();
}

int AVCodecDecoder::open_and_decode_until_error(const QOpenHDVideoHelper::VideoStreamConfig settings)
{
    // this is always for primary video, unless switching is enabled
    auto stream_config=settings.primary_stream_config;
    if(settings.generic.qopenhd_switch_primary_secondary){
        stream_config=settings.secondary_stream_config;
    }
    std::string in_filename="";
    if(settings.generic.dev_test_video_mode==QOpenHDVideoHelper::VideoTestMode::DISABLED){
        in_filename=QOpenHDVideoHelper::get_udp_rtp_sdp_filename(stream_config);
        //in_filename="rtp://192.168.0.1:5600";
    }else{
        if(settings.generic.dev_enable_custom_pipeline){
            in_filename=settings.generic.dev_custom_pipeline;
        }else{
            // For testing, I regulary change the filename(s) and recompile
            const bool consti_testing=false;
            if(consti_testing){
                if(stream_config.video_codec==QOpenHDVideoHelper::VideoCodecH264){
                     //in_filename="/tmp/x_raw_h264.h264";
                    in_filename="/home/consti10/Desktop/hello_drmprime/in/rpi_1080.h264";
                    //in_filename="/home/consti10/Desktop/hello_drmprime/in/rv_1280x720_green_white.h264";
                    //in_filename="/home/consti10/Desktop/hello_drmprime/in/Big_Buck_Bunny_1080_10s_1MB_h264.mp4";
                }else if(stream_config.video_codec==QOpenHDVideoHelper::VideoCodecH265){
                      //in_filename="/tmp/x_raw_h265.h265";
                    in_filename="/home/consti10/Desktop/hello_drmprime/in/jetson_test.h265";
                    //in_filename="/home/consti10/Desktop/hello_drmprime/in/Big_Buck_Bunny_1080_10s_1MB_h265.mp4";
                }else{
                   in_filename="/home/consti10/Desktop/hello_drmprime/in/uv_640x480.mjpeg";
                   //in_filename="/home/consti10/Desktop/hello_drmprime/in/Big_Buck_Bunny_1080.mjpeg";
                }
            }else{
                in_filename=QOpenHDVideoHelper::get_default_openhd_test_file(stream_config.video_codec);
            }

        }
    }
    av_log_set_level(AV_LOG_TRACE);

    // These options are needed for using the foo.sdp (rtp streaming)
    // https://stackoverflow.com/questions/20538698/minimum-sdp-for-making-a-h264-rtp-stream
    // https://stackoverflow.com/questions/16658873/how-to-minimize-the-delay-in-a-live-streaming-with-ffmpeg
    AVDictionary* av_dictionary=nullptr;
    av_dict_set(&av_dictionary, "protocol_whitelist", "file,udp,rtp", 0);
    /*av_dict_set(&av_dictionary, "buffer_size", "212992", 0);
    av_dict_set_int(&av_dictionary,"max_delay",0,0);
    av_dict_set(&av_dictionary,"reuse_sockets","1",0);
    av_dict_set_int(&av_dictionary, "reorder_queue_size", 0, 0);
    av_dict_set_int(&av_dictionary,"network-caching",0,0);
    //
    //av_dict_set(&av_dictionary,"sync","ext",0);
    //
    //av_dict_set_int(&av_dictionary, "probesize", 32, 0);
    //av_dict_set_int(&av_dictionary, "analyzeduration", 1000*100, 0); // Is in microseconds

    // I think those values are in seconds ?
    av_dict_set_int(&av_dictionary, "rw_timeout", 1000*100, 0); //microseconds
    av_dict_set_int(&av_dictionary, "stimeout", 1000*100, 0); //microseconds
    //av_dict_set_int(&av_dictionary, "rw_timeout", 0, 0);
    //av_dict_set_int(&av_dictionary, "stimeout",0, 0);
    av_dict_set_int(&av_dictionary,"rtbufsize",0,0);
    av_dict_set_int(&av_dictionary,"max_interleave_delta",1,0); //in microseconds
    //av_dict_set_int(&av_dictionary,"max_streams",1,0);

    av_dict_set(&av_dictionary, "rtsp_transport", "udp", 0);*/

    av_dict_set(&av_dictionary,"timeout",0,0);

    // For decode ?!
    av_dict_set_int(&av_dictionary, "extra_buffers", 10, 0);

    AVFormatContext *input_ctx = nullptr;
    input_ctx=avformat_alloc_context();
    assert(input_ctx);
    /*input_ctx->video_codec_id=AV_CODEC_ID_H264;
    input_ctx->flags |= AVFMT_FLAG_FLUSH_PACKETS;
    input_ctx->flags |= AVFMT_FLAG_NOBUFFER;*/
    //input_ctx->avio_flags = AVIO_FLAG_DIRECT;
    //input_ctx->flags |= AVFMT_FLAG_NOBUFFER;// | AVFMT_FLAG_FLUSH_PACKETS;

    //input_ctx->flags |= AVFMT_FLAG_NOFILLIN;
    //input_ctx->flags |= AVFMT_FLAG_NOPARSE;
    //input_ctx->flags |= AVFMT_FLAG_NONBLOCK;

    // open the input file
    if (avformat_open_input(&input_ctx,in_filename.c_str(), NULL, &av_dictionary) != 0) {
        qDebug()<< "Cannot open input file ["<<in_filename.c_str()<<"]";
        avformat_close_input(&input_ctx);
        // sleep a bit before returning in this case, to not occupy a full CPU thread just trying
        // to open a file/stream that doesn't exist / is ill-formated
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return -1;
    }
    qDebug()<<"done avformat_open_input["<<in_filename.c_str()<<"]";

    if (avformat_find_stream_info(input_ctx, NULL) < 0) {
        qDebug()<< "Cannot find input stream information.";
        avformat_close_input(&input_ctx);
        return -1;
    }
    qDebug()<<"done avformat_find_stream_info";
    int ret=0;
    // find the video stream information
#if LIBAVFORMAT_VERSION_MAJOR < 59 || (LIBAVFORMAT_VERSION_MAJOR == 59 && LIBAVFORMAT_VERSION_MINOR == 0 && LIBAVFORMAT_VERSION_MICRO < 100)
     ret = av_find_best_stream(input_ctx, AVMEDIA_TYPE_VIDEO, -1, -1,(AVCodec**) &decoder, 0);
#else
    ret = av_find_best_stream(input_ctx, AVMEDIA_TYPE_VIDEO, -1, -1,(const AVCodec**) &decoder, 0);
#endif
    if (ret < 0) {
        qDebug()<< "Cannot find a video stream in the input file";
        avformat_close_input(&input_ctx);
        return -1;
    }
    qDebug()<<"done av_find_best_stream:"<<ret;
    const int video_stream = ret;

    if(!(decoder->id==AV_CODEC_ID_H264 || decoder->id==AV_CODEC_ID_H265 || decoder->id==AV_CODEC_ID_MJPEG)){
      qDebug()<<"We only do h264,h265 and mjpeg in this project";
      avformat_close_input(&input_ctx);
      return 0;
    }

    #ifdef _WIN32
        const AVHWDeviceType kAvhwDeviceType = AV_HWDEVICE_TYPE_D3D11VA;
    #else
        const AVHWDeviceType kAvhwDeviceType = AV_HWDEVICE_TYPE_DRM;
    #endif

    //const AVHWDeviceType kAvhwDeviceType = AV_HWDEVICE_TYPE_D3D11VA;
    //const AVHWDeviceType kAvhwDeviceType = AV_HWDEVICE_TYPE_DXVA2;
    //const AVHWDeviceType kAvhwDeviceType = AV_HWDEVICE_TYPE_DRM;
    //const AVHWDeviceType kAvhwDeviceType = AV_HWDEVICE_TYPE_VAAPI;
    //const AVHWDeviceType kAvhwDeviceType = AV_HWDEVICE_TYPE_CUDA;
    //const AVHWDeviceType kAvhwDeviceType = AV_HWDEVICE_TYPE_VDPAU;

    bool is_mjpeg=false;
    if (decoder->id == AV_CODEC_ID_H264) {
        qDebug()<<"H264 decode";
        qDebug()<<all_hw_configs_for_this_codec(decoder).c_str();
        if(!stream_config.enable_software_video_decoder){
            // weird workaround needed for pi + DRM_PRIME
            /*if ((decoder = avcodec_find_decoder_by_name("h264_v4l2m2m")) == NULL) {
                fprintf(stderr, "Cannot find the h264 v4l2m2m decoder\n");
                avformat_close_input(&input_ctx);
                return -1;
            }*/
            auto tmp = avcodec_find_decoder_by_name("h264_mmal");
            if(tmp!=nullptr){
                decoder = tmp;
                 wanted_hw_pix_fmt = AV_PIX_FMT_MMAL;
            }else{
                wanted_hw_pix_fmt = AV_PIX_FMT_YUV420P;
            }
        }else{
            wanted_hw_pix_fmt = AV_PIX_FMT_YUV420P;
            //wanted_hw_pix_fmt = AV_PIX_FMT_DRM_PRIME;
        }
    }
    else if(decoder->id==AV_CODEC_ID_H265){
        qDebug()<<"H265 decode";
        qDebug()<<all_hw_configs_for_this_codec(decoder).c_str();
        wanted_hw_pix_fmt = AV_PIX_FMT_DRM_PRIME;
        //wanted_hw_pix_fmt = AV_PIX_FMT_CUDA;
        //wanted_hw_pix_fmt = AV_PIX_FMT_VAAPI;
        //wanted_hw_pix_fmt = AV_PIX_FMT_YUV420P;
        //wanted_hw_pix_fmt = AV_PIX_FMT_VAAPI;
        //wanted_hw_pix_fmt = AV_PIX_FMT_VDPAU;
    }else if(decoder->id==AV_CODEC_ID_MJPEG){
        qDebug()<<"Codec mjpeg";
        qDebug()<<all_hw_configs_for_this_codec(decoder).c_str();
        wanted_hw_pix_fmt=AV_PIX_FMT_YUVJ422P;
        //wanted_hw_pix_fmt=AV_PIX_FMT_YUVJ420P;
        //wanted_hw_pix_fmt=AV_PIX_FMT_CUDA;
        //wanted_hw_pix_fmt = AV_PIX_FMT_DRM_PRIME;
        is_mjpeg= true;
    }else{
        assert(true);
        avformat_close_input(&input_ctx);
        return 0;
    }

    if (!(decoder_ctx = avcodec_alloc_context3(decoder))){
        qDebug()<<"avcodec_alloc_context3 failed";
        avformat_close_input(&input_ctx);
        return -1;
    }

    // From moonlight-qt. However, on PI, this doesn't seem to make any difference, at least for H265 decode.
    // (I never measured h264, but don't think there it is different).
    // Always request low delay decoding
    decoder_ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;
    // Allow display of corrupt frames and frames missing references
    decoder_ctx->flags |= AV_CODEC_FLAG_OUTPUT_CORRUPT;
    decoder_ctx->flags2 |= AV_CODEC_FLAG2_SHOW_ALL;

    AVStream *video = input_ctx->streams[video_stream];
    if (avcodec_parameters_to_context(decoder_ctx, video->codecpar) < 0){
        avformat_close_input(&input_ctx);
        return -1;
    }

    std::string selected_decoding_type="?";
    if(stream_config.enable_software_video_decoder ||
            // for mjpeg we always use sw decode r.n, since for some reason the "fallback" to sw decode doesn't work here
            // and also the PI cannot do HW mjpeg decode anyways at least no one bothered to fix ffmpeg for it.
            stream_config.video_codec==QOpenHDVideoHelper::VideoCodecMJPEG
            ){
        qDebug()<<"User wants SW decode / mjpeg";
        decoder_ctx->get_format  = get_sw_format;
        selected_decoding_type="SW";
    }else{
        qDebug()<<"Try HW decode";
        decoder_ctx->get_format  = get_hw_format;
        if (hw_decoder_init(decoder_ctx, kAvhwDeviceType) < 0){
          qDebug()<<"HW decoder init failed,fallback to SW decode";
          selected_decoding_type="SW(HW failed)";
          // Use SW decode as fallback ?!
          //return -1;
          // H264 and H265 sw decode is always YUV420P, MJPEG seems to be always YUVJ422P;
          if(is_mjpeg){
            wanted_hw_pix_fmt=AV_PIX_FMT_YUVJ422P;
          }
          wanted_hw_pix_fmt=AV_PIX_FMT_YUV420P;
        }else{
            selected_decoding_type="HW";
        }
    }

    // A thread count of 1 reduces latency for both SW and HW decode
    decoder_ctx->thread_count = 1;

   if ((ret = avcodec_open2(decoder_ctx, decoder, nullptr)) < 0) {
        qDebug()<<"Failed to open codec for stream ";//<< video_stream;
        avformat_close_input(&input_ctx);
        return -1;
    }
    AVPacket packet;
    // actual decoding and dump the raw data
    const auto decodingStart=std::chrono::steady_clock::now();
    int nFeedFrames=0;
    auto lastFrame=std::chrono::steady_clock::now();
    reset_before_decode_start();
    DecodingStatistcs::instance().set_decoding_type(selected_decoding_type.c_str());
    while (ret >= 0) {
        if(request_restart){
            request_restart=false;
            // Since we do streaming and SPS/PPS come in regular intervals, we can just cancel and restart at any time.
            break;
        }
        if ((ret = av_read_frame(input_ctx, &packet)) < 0){
            qDebug()<<"av_read_frame returned:"<<ret<<" "<<av_error_as_string(ret).c_str();
            if(ret==-110){ //-110   Connection timed out
                ret=0;
                continue;
            }
            break;
        }
        //qDebug()<<"Got av_packet"<<debug_av_packet(&packet).c_str();
        if(false){
             qDebug()<<"Got "<<debug_av_packet(&packet).c_str();
        }else{
            //std::vector<uint8_t> as_buff(packet.data,packet.data+packet.size);
            //qDebug()<<"Packet:"<<StringHelper::vectorAsString(as_buff).c_str()<<"\n";

            if (video_stream == packet.stream_index){
            //if(true){
                int limitedFrameRate=settings.generic.dev_limit_fps_on_test_file;
                if(settings.generic.dev_test_video_mode==QOpenHDVideoHelper::VideoTestMode::DISABLED){
                    // never limit the fps on decode when doing live streaming !
                    limitedFrameRate=-1;
                }
                if(limitedFrameRate>0){
                    const long frameDeltaNs=1000*1000*1000 / limitedFrameRate;
                    while (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-lastFrame).count()<frameDeltaNs){
                        // busy wait
                    }
                    lastFrame=std::chrono::steady_clock::now();
                }
                ret = decode_and_wait_for_frame(&packet);
                nFeedFrames++;
                if(limitedFrameRate>0){
                    const uint64_t runTimeMs=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-decodingStart).count();
                    const double runTimeS=runTimeMs/1000.0f;
                    const double fps=runTimeS==0 ? 0 : nFeedFrames/runTimeS;
                    //qDebug()<<"Fake fps:"<<fps;
                }
            }
        }
         av_packet_unref(&packet);
    }
    qDebug()<<"Broke out of the queue_data_dequeue_frame loop";
    //m_pull_frames_from_ffmpeg_thread->join();
    //m_pull_frames_from_ffmpeg_thread=nullptr;
    // flush the decoder - not needed
    //packet.data = NULL;
    //packet.size = 0;
    //ret = decode_and_wait_for_frame(&packet);
    DecodingStatistcs::instance().set_decode_time("-1");
    DecodingStatistcs::instance().set_primary_stream_frame_format("-1");
    avcodec_free_context(&decoder_ctx);
    qDebug()<<"avcodec_free_context done";
    avformat_close_input(&input_ctx);
    qDebug()<<"avformat_close_input_done";
    return 0;
}


// https://ffmpeg.org/doxygen/3.3/decode_video_8c-example.html
void AVCodecDecoder::open_and_decode_until_error_custom_rtp(const QOpenHDVideoHelper::VideoStreamConfig settings)
{
    // this is always for primary video, unless switching is enabled
    auto stream_config=settings.primary_stream_config;
    if(settings.generic.qopenhd_switch_primary_secondary){
        stream_config=settings.secondary_stream_config;
    }

    // This thread pulls frame(s) from the rtp decoder and therefore should have high priority
    SchedulingHelper::setThreadParamsMaxRealtime();
    av_log_set_level(AV_LOG_TRACE);
     assert(stream_config.video_codec==QOpenHDVideoHelper::VideoCodecH264 || stream_config.video_codec==QOpenHDVideoHelper::VideoCodecH265);
     if(stream_config.video_codec==QOpenHDVideoHelper::VideoCodecH264){
         decoder = avcodec_find_decoder(AV_CODEC_ID_H264);
     }else if(stream_config.video_codec==QOpenHDVideoHelper::VideoCodecH265){
         decoder = avcodec_find_decoder(AV_CODEC_ID_H265);
     }
     if (!decoder) {
         qDebug()<< "AVCodecDecoder::open_and_decode_until_error_custom_rtp: Codec not found";
         return;
     }
     // ----------------------
     bool use_pi_hw_decode=false;
     if (decoder->id == AV_CODEC_ID_H264) {
         qDebug()<<"H264 decode";
         qDebug()<<all_hw_configs_for_this_codec(decoder).c_str();
         if(!stream_config.enable_software_video_decoder){
             auto tmp = avcodec_find_decoder_by_name("h264_mmal");
             if(tmp!=nullptr){
                 decoder = tmp;
                 wanted_hw_pix_fmt = AV_PIX_FMT_MMAL;
                 use_pi_hw_decode=true;
             }else{
                 wanted_hw_pix_fmt = AV_PIX_FMT_YUV420P;
             }
         }else{
             wanted_hw_pix_fmt = AV_PIX_FMT_YUV420P;
         }
     }else if(decoder->id==AV_CODEC_ID_H265){
         qDebug()<<"H265 decode";
         if(!stream_config.enable_software_video_decoder){
             qDebug()<<all_hw_configs_for_this_codec(decoder).c_str();
             // HW format used by rpi h265 HW decoder
             wanted_hw_pix_fmt = AV_PIX_FMT_DRM_PRIME;
             use_pi_hw_decode=true;
         }else{

         }
     }
     // ------------------------------------
     decoder_ctx = avcodec_alloc_context3(decoder);
     if (!decoder_ctx) {
         qDebug()<< "AVCodecDecoder::open_and_decode_until_error_custom_rtp: Could not allocate video codec context";
         return;
     }
     // ----------------------------------
    // From moonlight-qt. However, on PI, this doesn't seem to make any difference, at least for H265 decode.
    // (I never measured h264, but don't think there it is different).
    // Always request low delay decoding
    decoder_ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;
    // Allow display of corrupt frames and frames missing references
    decoder_ctx->flags |= AV_CODEC_FLAG_OUTPUT_CORRUPT;
    decoder_ctx->flags2 |= AV_CODEC_FLAG2_SHOW_ALL;
    // --------------------------------------
    // --------------------------------------
    std::string selected_decoding_type="?";
    if(use_pi_hw_decode){
        decoder_ctx->get_format  = get_hw_format;
        if (hw_decoder_init(decoder_ctx, AV_HWDEVICE_TYPE_DRM) < 0){
          qDebug()<<"HW decoder init failed,fallback to SW decode";
          selected_decoding_type="SW(HW failed)";
          assert(true);
        }else{
            selected_decoding_type="HW";
        }
    }else{
        selected_decoding_type="SW";
    }
    // A thread count of 1 reduces latency for both SW and HW decode
    decoder_ctx->thread_count = 1;

    // ---------------------------------------

     if (avcodec_open2(decoder_ctx, decoder, NULL) < 0) {
         fprintf(stderr, "Could not open codec\n");
         avcodec_free_context(&decoder_ctx);
         return;
     }
     qDebug()<<"AVCodecDecoder::open_and_decode_until_error_custom_rtp()-begin loop";
#ifdef HACK_RAW
     m_rtp_receiver=std::make_unique<RawReceiver>(stream_config.udp_rtp_input_port,stream_config.udp_rtp_input_ip_address,stream_config.video_codec==1,settings.generic.dev_feed_incomplete_frames_to_decoder);
#else
     m_rtp_receiver=std::make_unique<RTPReceiver>(stream_config.udp_rtp_input_port,stream_config.udp_rtp_input_ip_address,stream_config.video_codec==1,settings.generic.dev_feed_incomplete_frames_to_decoder);
#endif

     reset_before_decode_start();
     DecodingStatistcs::instance().set_decoding_type(selected_decoding_type.c_str());
     AVPacket *pkt=av_packet_alloc();
     assert(pkt!=nullptr);
     bool has_keyframe_data=false;
     while(true){
         // We break out of this loop if someone requested a restart
         if(request_restart){
             request_restart=false;
             goto finish;
         }
         // or the decode config changed and we need a restart
         if(m_rtp_receiver->config_has_changed_during_decode){
             qDebug()<<"Break/Restart,config has changed during decode";
             goto finish;
         }
         //std::this_thread::sleep_for(std::chrono::milliseconds(3000));
         if(!has_keyframe_data){
              std::shared_ptr<std::vector<uint8_t>> keyframe_buf=m_rtp_receiver->get_config_data();
              if(keyframe_buf==nullptr){
                  std::this_thread::sleep_for(std::chrono::milliseconds(100));
                  continue;
              }
              qDebug()<<"Got decode data (before keyframe)";
              pkt->data=keyframe_buf->data();
              pkt->size=keyframe_buf->size();
              decode_config_data(pkt);
              has_keyframe_data=true;
              continue;
         }else{
             auto buf =m_rtp_receiver->get_next_frame(std::chrono::milliseconds(kDefaultFrameTimeout));
             if(buf==nullptr){
                 // No buff after X seconds
                 continue;
             }
             //qDebug()<<"Got frame";
             //qDebug()<<"Got decode data (after keyframe)";
             pkt->data=(uint8_t*)buf->get_nal().getData();
             pkt->size=buf->get_nal().getSize();
             decode_and_wait_for_frame(pkt,buf->get_nal().creationTime);
             //fetch_frame_or_feed_input_packet();
         }
     }
finish:
     qDebug()<<"AVCodecDecoder::open_and_decode_until_error_custom_rtp()-end loop";
     m_rtp_receiver=nullptr;
     avcodec_free_context(&decoder_ctx);
}

void AVCodecDecoder::timestamp_add_fed(int64_t ts)
{
    m_fed_timestamps_queue.push_back(ts);
    if(m_fed_timestamps_queue.size()>=MAX_FED_TIMESTAMPS_QUEUE_SIZE){
        m_fed_timestamps_queue.pop_front();
    }
}

bool AVCodecDecoder::timestamp_check_valid(int64_t ts)
{
    for(const auto& el:m_fed_timestamps_queue){
        if(el==ts)return true;
    }
    return false;
}

void AVCodecDecoder::timestamp_debug_valid(int64_t ts)
{
    const bool valid=timestamp_check_valid(ts);
    if(valid){
        qDebug()<<"Is a valid timestamp";
    }else{
        qDebug()<<"Is not a valid timestamp";
    }
}

void AVCodecDecoder::dirty_generic_decode_via_external_decode_service(const QOpenHDVideoHelper::VideoStreamConfig& settings)
{
    qopenhd::decode::service::decode_via_external_decode_service(settings,request_restart);
}
