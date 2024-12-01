//
// Created by consti10 on 04.09.22.
//

#ifndef HELLO_DRMPRIME__AVCODEC_HELPER_HPP_
#define HELLO_DRMPRIME__AVCODEC_HELPER_HPP_

// Include all the "av" stuff we need
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>
#include <libavutil/buffer.h>
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include "libavutil/hwcontext.h"
#include "libavutil/hwcontext_drm.h"
#include "libavutil/pixdesc.h"
}

#include <iostream>
#include <string>
#include <sstream>

// For some reaseon av_frame_cropped_width doesn't exit on ffmpeg default on ubuntu
// but on rpi, it does !
static int x_av_frame_cropped_width(const AVFrame* frame){
  return frame->width;
}

static int x_av_frame_cropped_height(const AVFrame* frame){
  return frame->height;
}


static std::string safe_av_hwdevice_get_type_name(enum AVHWDeviceType type){
  auto tmp= av_hwdevice_get_type_name(type);
  if(tmp== nullptr){
	return "null";
  }
  return {tmp};
}

static std::string av_error_as_string(int err){
    char buf[1024];
    av_make_error_string(buf,1024,err);
    return std::string(buf);
}

static std::string safe_av_get_pix_fmt_name(enum AVPixelFormat pix_fmt){
  auto tmp= av_get_pix_fmt_name(pix_fmt);
  if(tmp== nullptr){
	return "null";
  }
  return {tmp};
}
static std::string safe_av_get_colorspace_name(enum AVColorSpace val){
  auto tmp= av_color_space_name(val);
  if(tmp== nullptr){
	return "null";
  }
  return {tmp};
}

static std::string all_av_hwframe_transfer_formats(AVBufferRef *hwframe_ctx){
  std::stringstream ss;
  if(hwframe_ctx== nullptr){
	ss<<"Frame has no hwframe_ctx\n";
	return ss.str();
  }
  enum AVPixelFormat *formats;
  const auto err = av_hwframe_transfer_get_formats(hwframe_ctx, AV_HWFRAME_TRANSFER_DIRECTION_FROM, &formats, 0);
  if (err < 0) {
	ss<<"av_hwframe_transfer_get_formats error\n";
	return ss.str();
  }
  ss<<"Supported transfers:";
  for (int i = 0; formats[i] != AV_PIX_FMT_NONE; i++) {
	ss<<i<<":"<<safe_av_get_pix_fmt_name(formats[i])<<",";
  }
  ss<<"\n";
  av_freep(&formats);
  return ss.str();
}

static std::string all_av_hwdevice_types(){
  std::stringstream ss;
  AVHWDeviceType tmp_type=AV_HWDEVICE_TYPE_NONE;
  ss<< "Available HW device types:";
  while((tmp_type = av_hwdevice_iterate_types(tmp_type)) != AV_HWDEVICE_TYPE_NONE){
	ss<<" "<<safe_av_hwdevice_get_type_name(tmp_type);
  }
  ss<<"\n";
  return ss.str();
}

static std::string all_formats_to_string(const enum AVPixelFormat *pix_fmts){
  std::stringstream ss;
  ss<<"PixelFormats:[";
  if(pix_fmts== nullptr){
	ss<<"none]";
	return ss.str();
  }
  for(int i=0;;i++){
	auto tmp=pix_fmts[i];
	if(tmp==AV_PIX_FMT_NONE)break;
	ss<<safe_av_get_pix_fmt_name(tmp)<<",";
  }
  ss<<"]";
  return ss.str();
}

static std::string all_hw_configs_for_this_codec(const AVCodec *decoder){
    std::stringstream ss;
    ss<<"all_hw_configs_for_this_codec:\n";
    for (int i = 0;; i++) {
        const AVCodecHWConfig *config = avcodec_get_hw_config(decoder, i);
        if (!config) {
            if(i==0){
                ss<<"Codec does not support any HW configurations";
                return ss.str();
            }else{
                return ss.str();
            }

        }
        ss<<"HW config "<<i<<" ";
        ss<<"HW Device name: "<<safe_av_hwdevice_get_type_name(config->device_type);
        ss<<" PIX fmt: "<<safe_av_get_pix_fmt_name(config->pix_fmt);
        ss<<"\n";
    }
}

static std::string av_packet_flags_to_string(int flags){
    if(flags & AV_PKT_FLAG_KEY){
        return "AV_PKT_FLAG_KEY";
    }
    return std::to_string(flags);
}

static std::string debug_av_packet(const AVPacket* packet){
    std::stringstream ss;
    ss<<"AVPacket size:"<<packet->size<<",";
    ss<<" flags:"<<av_packet_flags_to_string(packet->flags)<<",";
    ss<<" side data elements:"<<packet->side_data_elems<<",";
    ss<<" pts:"<<packet->pts<<",";
    ss<<" dts:"<<packet->dts<<"";
    return ss.str();
}
static bool is_AV_PIX_FMT_YUV420P(int format){
    // ffmpeg still has YUVJ420 even though marked as to be removed, and we get it in case of mjpeg decode
    return format==AV_PIX_FMT_YUV420P || format==AV_PIX_FMT_YUVJ420P;
}
static bool is_AV_PIX_FMT_YUV422P(int format){
    // ffmpeg still has YUVJ420 even though marked as to be removed, and we get it in case of mjpeg decode
    return format==AV_PIX_FMT_YUV422P || format==AV_PIX_FMT_YUVJ422P;
}


static bool is_AV_PIX_FMT_YUV42XP( int format){
    return is_AV_PIX_FMT_YUV420P(format) || is_AV_PIX_FMT_YUV422P(format);
}

static std::string debug_frame(const AVFrame* frame){
    std::stringstream ss;
    ss<<""<<frame->width<<"x"<<frame->height;
    ss<<"crop bottom:"<<frame->crop_bottom<<" left:"<<frame->crop_left<<" top:"<<frame->crop_top<<" right:"<<frame->crop_right;
    ss<<"linesize:"<<frame->linesize[0]<<","<<frame->linesize[1]<<","<<frame->linesize[2]<<","<<frame->linesize[3];
    return ss.str();
}

/*struct XBestDecoder{
    const AVCodec *decoder;
    bool is_hw;
    enum AVPixelFormat wanted_hw_pix_fmt;
    AVHWDeviceType kAvhwDeviceType;
};

static XBestDecoder find_best_decoder(const AVCodec *decoder){

}*/


#endif //HELLO_DRMPRIME__AVCODEC_HELPER_HPP_
