#include "aohdsystem.h"

#include "../../common/StringHelper.hpp"
#include "../../common/TimeHelper.hpp"
#include "QOpenHDVideoHelper.hpp"
#include "tutil/telemetryutil.hpp"
#include "wificard.h"
#include "rcchannelsmodel.h"
#include "camerastreammodel.h"

#include <string>
#include <sstream>
#include <MavlinkTelemetry.h>

#include <logging/logmessagesmodel.h>
#include <logging/hudlogmessagesmodel.h>

#include "util/qopenhd.h"
#include "../tutil/qopenhdmavlinkhelper.hpp"

#include <../util/WorkaroundMessageBox.h>
#include "../settings/wblinksettingshelper.h"
#include "../settings/mavlinksettingsmodel.h"

#include "openhd_core/platform.hpp"

// From https://netbeez.net/blog/what-is-mcs-index/
static std::vector<int> get_dbm_20mhz(){
    return {-82,-79,-77,-74,-70,-66,-65,-64,-59,-57};
}
static std::vector<int> get_dbm_40mhz(){
    return {-79,-76,-74,-71,-67,-63,-62,-61,-56,-54};
}

static int get_required_dbm_for_rate(int channel_width,int mcs_index){
    const auto values=channel_width==20 ? get_dbm_20mhz() : get_dbm_40mhz();
    if(mcs_index>=0 && mcs_index<=values.size()){
        return values[mcs_index];
    }
    return 0;
}
// Bit field for boolean only value(s)
struct MonitorModeLinkBitfield {
    unsigned int stbc:1;
    unsigned int lpdc:1;
    unsigned int short_guard:1;
    unsigned int curr_rx_last_packet_status_good:1;
    unsigned int unused:4;
}
#ifdef __windows__
;
#else
__attribute__ ((packed));
static_assert(sizeof(MonitorModeLinkBitfield)==1);
#endif
static MonitorModeLinkBitfield parse_monitor_link_bitfield(uint8_t bitfield){
    MonitorModeLinkBitfield ret{};
#ifdef __windows__
#else
    std::memcpy((uint8_t*)&ret,&bitfield,1);
#endif
    return ret;
}

static std::string ohd_version_as_string(uint8_t major, uint8_t minor,
                                         uint8_t patch, uint8_t release_type) {
    std::stringstream ss;
    ss << (int)major << "." << (int)minor << "." << (int)patch << "-evo";
    if (release_type == 0) {
        // This is a 'proper' release
    } else if (release_type == 1) {
        ss << "-beta";  // testers only
    } else if (release_type == 2) {
        ss << "-alpha";  // internal only
    } else {
        ss << "-unknown";
    }
    return ss.str();
}

AOHDSystem::AOHDSystem(const bool is_air,QObject *parent)
    : QObject{parent},m_is_air(is_air)
{
    m_alive_timer = std::make_unique<QTimer>(this);
    QObject::connect(m_alive_timer.get(), &QTimer::timeout, this, &AOHDSystem::update_alive);
    m_alive_timer->start(1000);
}

AOHDSystem &AOHDSystem::instanceAir()
{
    static AOHDSystem air(true);
    return air;
}

AOHDSystem &AOHDSystem::instanceGround()
{
    static AOHDSystem ground(false);
    return ground;
}

bool AOHDSystem::process_message(const mavlink_message_t &msg)
{
    if(msg.sysid != get_own_sys_id()){ // improper usage
        qDebug()<<"AOHDSystem::process_message: wron sys id";
        return false;
    }
    autofech_params_if_apropriate();
    m_last_message_ms=QOpenHDMavlinkHelper::getTimeMilliseconds();
    bool consumed=false;
    switch(msg.msgid){
        case MAVLINK_MSG_ID_OPENHD_VERSION_MESSAGE:{
            mavlink_openhd_version_message_t parsedMsg;
            mavlink_msg_openhd_version_message_decode(&msg,&parsedMsg);
            set_openhd_version(ohd_version_as_string(parsedMsg.major,parsedMsg.minor,parsedMsg.patch,parsedMsg.release_type).c_str());
            consumed=true;
        }break;
        case MAVLINK_MSG_ID_ONBOARD_COMPUTER_STATUS:{
            mavlink_onboard_computer_status_t parsedMsg;
            mavlink_msg_onboard_computer_status_decode(&msg,&parsedMsg);
            process_onboard_computer_status(parsedMsg);
            consumed=true;
        }break;
        case MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_CARD:{
            mavlink_openhd_stats_monitor_mode_wifi_card_t parsedMsg;
            mavlink_msg_openhd_stats_monitor_mode_wifi_card_decode(&msg,&parsedMsg);
            //qDebug()<<"Got MAVLINK_MSG_ID_OPENHD_WIFI_CARD"<<(int)parsedMsg.card_index<<" "<<(int)parsedMsg.rx_rssi;
            process_x0(parsedMsg);
            consumed=true;
        }break;
        case MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK:{
            mavlink_openhd_stats_monitor_mode_wifi_link_t parsedMsg;
            mavlink_msg_openhd_stats_monitor_mode_wifi_link_decode(&msg,&parsedMsg);
            process_x1(parsedMsg);
            consumed=true;
        }break;
        case MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY:{
            mavlink_openhd_stats_telemetry_t parsedMsg;
            mavlink_msg_openhd_stats_telemetry_decode(&msg,&parsedMsg);
            process_x2(parsedMsg);
            consumed=true;
        }break;
        case MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR:{
            mavlink_openhd_stats_wb_video_air_t parsedMsg;
            mavlink_msg_openhd_stats_wb_video_air_decode(&msg,&parsedMsg);
            process_x3(parsedMsg);
            consumed=true;
        }break;
        case MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_FEC_PERFORMANCE:{
            mavlink_openhd_stats_wb_video_air_fec_performance_t parsedMsg;
            mavlink_msg_openhd_stats_wb_video_air_fec_performance_decode(&msg,&parsedMsg);
            process_x3b(parsedMsg);
            consumed=true;
        }break;
        case MAVLINK_MSG_ID_OPENHD_CAMERA_STATUS_AIR:{
            mavlink_openhd_camera_status_air_t parsedMsg;
            mavlink_msg_openhd_camera_status_air_decode(&msg,&parsedMsg);
            if(msg.compid==OHD_COMP_ID_AIR_CAMERA_PRIMARY){
                CameraStreamModel::instance(0).update_mavlink_openhd_camera_status_air(parsedMsg);
            }else if(msg.compid==OHD_COMP_ID_AIR_CAMERA_SECONDARY){
                CameraStreamModel::instance(1).update_mavlink_openhd_camera_status_air(parsedMsg);
                set_dirty_air_has_secondary_cam(true);
                // Feature - tell user to enable 2 cameras in qopenhd
                set_n_openhd_cameras(2);
                const int value_in_qopenhd=QOpenHDVideoHelper::get_qopenhd_n_cameras();
                if(value_in_qopenhd!=2){
                    const auto elapsed=std::chrono::steady_clock::now()-m_last_n_cameras_message;
                    if(elapsed>std::chrono::seconds(5)){
                        LogMessagesModel::instanceGround().add_message_debug("QOpenHD","Please enable 2 camera control");
                        m_last_n_cameras_message=std::chrono::steady_clock::now();
                    }
                }

            }
            consumed=true;
        }break;
        case MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND:{
            mavlink_openhd_stats_wb_video_ground_t parsedMsg;
            mavlink_msg_openhd_stats_wb_video_ground_decode(&msg,&parsedMsg);
            process_x4(parsedMsg);
            consumed=true;
        }break;
        case MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_FEC_PERFORMANCE:{
            mavlink_openhd_stats_wb_video_ground_fec_performance_t parsedMsg;
            mavlink_msg_openhd_stats_wb_video_ground_fec_performance_decode(&msg,&parsedMsg);
            process_x4b(parsedMsg);
            consumed=true;
        }break;
        case MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION:{
            mavlink_openhd_onboard_computer_status_extension_t parsedMsg;
            mavlink_msg_openhd_onboard_computer_status_extension_decode(&msg,&parsedMsg);
            consumed=true;
        }break;
        case MAVLINK_MSG_ID_HEARTBEAT:{
            mavlink_heartbeat_t parsedMsg;
            mavlink_msg_heartbeat_decode(&msg,&parsedMsg);
            m_last_heartbeat_ms=QOpenHDMavlinkHelper::getTimeMilliseconds();
            if(parsedMsg.autopilot!=MAV_AUTOPILOT_INVALID){
                qDebug()<<"Warning OpenHD systems should always set autopilot to none";
            }
            consumed=true;
        }break;
        case MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE:{
             mavlink_rc_channels_override_t parsedMsg;
             mavlink_msg_rc_channels_override_decode(&msg,&parsedMsg);
             RCChannelsModel::instanceGround().update_all_channels(Telemetryutil::mavlink_msg_rc_channels_override_to_array(parsedMsg));
            consumed=true;
        };break;
        case MAVLINK_MSG_ID_STATUSTEXT:{
             mavlink_statustext_t parsedMsg;
             mavlink_msg_statustext_decode(&msg,&parsedMsg);
             auto tmp=Telemetryutil::statustext_convert(parsedMsg);
             if(m_is_air){
                LogMessagesModel::instanceOHDAir().addLogMessage("OHD[A]",tmp.message.c_str(),tmp.level);
             }else{
                LogMessagesModel::instanceGround().addLogMessage("OHD[G]",tmp.message.c_str(),tmp.level);
             }
             // Notify user in HUD of external device connect / disconnect events
             if(tmp.message.find("External device") != std::string::npos){
                HUDLogMessagesModel::instance().add_message(tmp.level,tmp.message.c_str());
             }
            consumed=true;
        }break;
        case MAVLINK_MSG_ID_OPENHD_WIFBROADCAST_SUPPORTED_CHANNELS:{
            mavlink_openhd_wifbroadcast_supported_channels_t parsedMsg;
            mavlink_msg_openhd_wifbroadcast_supported_channels_decode(&msg,&parsedMsg);
            if(!m_is_air){
                WBLinkSettingsHelper::instance().process_message_openhd_wifibroadcast_supported_channels(parsedMsg);
            }
            consumed=true;
        }break;
        case MAVLINK_MSG_ID_OPENHD_WIFBROADCAST_ANALYZE_CHANNELS_PROGRESS:{
            mavlink_openhd_wifbroadcast_analyze_channels_progress_t parsedMsg;
            mavlink_msg_openhd_wifbroadcast_analyze_channels_progress_decode(&msg,&parsedMsg);
            if(!m_is_air){
                WBLinkSettingsHelper::instance().process_message_openhd_wifibroadcast_analyze_channels_progress(parsedMsg);
            }
            consumed=true;
        }break;
        case MAVLINK_MSG_ID_OPENHD_WIFBROADCAST_SCAN_CHANNELS_PROGRESS:{
            mavlink_openhd_wifbroadcast_scan_channels_progress_t parsedMsg;
            mavlink_msg_openhd_wifbroadcast_scan_channels_progress_decode(&msg,&parsedMsg);
            if(!m_is_air){
                WBLinkSettingsHelper::instance().process_message_openhd_wifibroadcast_scan_channels_progress(parsedMsg);
            }
            consumed=true;
        }break;
        case MAVLINK_MSG_ID_OPENHD_SYS_STATUS1:{
            mavlink_openhd_sys_status1_t parsed;
            mavlink_msg_openhd_sys_status1_decode(&msg,&parsed);
            process_sys_status1(parsed);
            consumed=true;
        }break;
        case MAVLINK_MSG_ID_OPENHD_WIFBROADCAST_GND_OPERATING_MODE:{
            if(m_is_air) qDebug()<<"Message mismatch"; // only ground reports operating mode
            mavlink_openhd_wifbroadcast_gnd_operating_mode_t parsed;
            mavlink_msg_openhd_wifbroadcast_gnd_operating_mode_decode(&msg,&parsed);
            process_op_mode(parsed);
            consumed=true;
        }break;
        default:{
            qDebug()<<"Unknown openhd message type";
        }break;
        /*case MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE:{
            mavlink_openhd_log_message_t parsedMsg;
            mavlink_msg_openhd_log_message_decode(&msg,&parsedMsg);
            const QString message{parsedMsg.text};
            const quint64 timestamp=parsedMsg.timestamp;
            const quint8 severity=parsedMsg.severity;
            qDebug()<<"Log message:"<<message;
            LogMessagesModel::instance().addLogMessage({"OHD",message,timestamp,LogMessagesModel::log_severity_to_color(severity)});
            break;
        }*/
    }
    return consumed;
}

QString AOHDSystem::get_rate_for_mcs_bw(int mcs, int bw)
{
    return "TODO";
}

void AOHDSystem::process_onboard_computer_status(const mavlink_onboard_computer_status_t &msg)
{
    set_curr_cpuload_perc(msg.cpu_cores[0]);
    set_curr_soc_temp_degree(msg.temperature_core[0]);
    set_curr_txc_temp_degree(msg.temperature_core[1]);
    // temporary, we repurpose this value
    set_curr_cpu_freq_mhz(msg.storage_type[0]);
    set_curr_isp_freq_mhz(msg.storage_type[1]);
    set_curr_h264_freq_mhz(msg.storage_type[2]);
    set_curr_core_freq_mhz(msg.storage_type[3]);
    set_curr_v3d_freq_mhz(msg.storage_usage[0]);
    set_curr_space_left_mb(msg.storage_usage[1]);
    set_rpi_undervolt_error(msg.link_tx_rate[0]==1);
    set_microhard_enabled(msg.link_rx_rate[0]);
    set_microhard_rssi(msg.link_rx_rate[1]);
    set_microhard_tx_pwr(msg.link_rx_rate[2]);
    set_microhard_bw(msg.link_rx_rate[3]);
    set_microhard_freq(msg.link_rx_rate[4]);
    set_microhard_noise(msg.link_rx_rate[5]);
    set_microhard_snr(msg.link_rx_rate[6]);
    set_ina219_voltage_millivolt(msg.storage_usage[2]);
    set_ina219_current_milliamps(msg.storage_usage[3]);
    set_ram_usage_perc(msg.ram_usage);
    set_ram_total(msg.ram_total);
    int16_t air_reported_sys_id=msg.fan_speed[0];
    set_air_reported_fc_sys_id(air_reported_sys_id);
    const uint8_t ohd_platform=msg.link_type[0];
    set_ohd_platform(ohd_platform);
    if(m_is_air && QOpenHD::instance().is_platform_rpi() && ohd_platform==30){
        // Air is x20, and qopenhd is running on rpi
        if(!m_x20_rpi_upgrade_warning_logged){
            QOpenHD::instance().show_toast("X20 -> RPI has high latency.\nPlease upgrade your ground station to next gen.");
            m_x20_rpi_upgrade_warning_logged=true;
        }
    }
    const auto platform_as_str=x_platform_type_to_string(ohd_platform);
    set_ohd_platform_type_as_string(platform_as_str.c_str());
}

void AOHDSystem::process_x0(const mavlink_openhd_stats_monitor_mode_wifi_card_t &msg){
    if(m_is_air && msg.card_index>1){
        qDebug()<<"Air only has 1 wifibroadcats card";
        return;
    }
    //qDebug()<<"Got mavlink_openhd_stats_monitor_mode_wifi_card_t";
    if(m_is_air){
        if(msg.card_index!=0){
            qDebug()<<"Air only has 1 wifibroadcats card";
            return;
        }
        auto& card=WiFiCard::instance_air();
        card.process_mavlink(msg);
        set_current_rx_rssi(card.curr_rx_rssi_dbm());
    }else{
        if(msg.card_index<0 || msg.card_index>=4){
            qDebug()<<"Gnd invalid card index"<<msg.card_index;
            return;
        }
        auto& card=WiFiCard::instance_gnd(msg.card_index);
        card.process_mavlink(msg);
        set_current_rx_rssi(WiFiCard::helper_get_gnd_curr_best_rssi());
    }
    // TODO: r.n we don't differentiate signal quality per card
    if(msg.card_index==0){
        set_current_rx_signal_quality(msg.rx_signal_quality_adapter);
    }
}

void AOHDSystem::process_x1(const mavlink_openhd_stats_monitor_mode_wifi_link_t &msg){
    //qDebug()<<"Got mavlink_openhd_stats_monitor_mode_wifi_link_t";
    set_curr_rx_packet_loss_perc(msg.curr_rx_packet_loss_perc);
    set_count_tx_inj_error_hint(msg.count_tx_inj_error_hint);
    set_count_tx_dropped_packets(msg.count_tx_dropped_packets);
    const int new_mcs_index=msg.curr_tx_mcs_index;
    if(m_is_air){
        // We are only interested in the mcs index of the air unit
        if(m_curr_mcs_index!=-1 && new_mcs_index!=m_curr_mcs_index){
            std::stringstream ss;
            ss<<"MCS index changed to "<<new_mcs_index;
            HUDLogMessagesModel::instance().add_message_info(ss.str().c_str());
        }
    }
    set_curr_mcs_index(new_mcs_index);
    set_curr_channel_mhz(msg.curr_tx_channel_mhz);
    set_curr_channel_width_mhz(msg.curr_tx_channel_w_mhz);
    if(m_is_air){
        WBLinkSettingsHelper::instance().validate_and_set_air_channel_width_mhz(msg.curr_tx_channel_w_mhz);
    }else{
        WBLinkSettingsHelper::instance().validate_and_set_gnd_channel_mhz(msg.curr_tx_channel_mhz);
        WBLinkSettingsHelper::instance().validate_and_set_gnd_channel_width_mhz(msg.curr_tx_channel_w_mhz);
    }
    set_curr_bitrate_kbits(msg.curr_rate_kbits);
    set_curr_n_rate_adjustments(msg.curr_n_rate_adjustments);
    set_tx_packets_per_second_and_bits_per_second(StringHelper::bitrate_and_pps_to_string(msg.curr_tx_bps,msg.curr_tx_pps).c_str());
    set_rx_packets_per_second_and_bits_per_second(StringHelper::bitrate_and_pps_to_string(msg.curr_rx_bps,msg.curr_rx_pps).c_str());
    const auto bitfield=parse_monitor_link_bitfield(msg.bitfield);
    set_wb_stbc_enabled(bitfield.stbc);
    set_wb_lpdc_enabled(bitfield.lpdc);
    set_wb_short_guard_enabled(bitfield.short_guard);
    set_curr_rx_last_packet_status_good(bitfield.curr_rx_last_packet_status_good);

    // Feature: Warning if dBm falls below minimum threshold for current MCS index on packets that need to be received
    // We need to get the mcs index from the other system (aka from air if we are running on ground) since that's whats being injected
    const int mcs_index_other=m_is_air ? AOHDSystem::instanceGround().m_curr_mcs_index : AOHDSystem::instanceAir().m_curr_mcs_index;
    const int minimum_dbm=get_required_dbm_for_rate(m_curr_channel_width_mhz,mcs_index_other);
    const int curr_rx_dbm=m_current_rx_rssi;
    const auto dbm_remaining=curr_rx_dbm - minimum_dbm;
    //qDebug()<<"On "<<(m_is_air ? "AIR" : "GND" )<<" RX mcs:"<<mcs_index_other<<" RX rssi:"<<curr_rx_dbm<<" minumum dbm:"<<minimum_dbm<<" remaining dbm:"<<dbm_remaining;
    if(minimum_dbm !=0 && curr_rx_dbm>-127){
        if(dbm_remaining<=0){
            // warning
            set_dbm_too_low_warning(2);
        }else if(dbm_remaining<=2){
            // caution
            set_dbm_too_low_warning(1);
        }else{
            set_dbm_too_low_warning(0);
        }
    }else{
        set_dbm_too_low_warning(0);
    }
    set_wb_link_pollution_perc(msg.pollution_perc);
    set_wb_link_curr_foreign_pps(msg.dummy1);
}

void AOHDSystem::process_x2(const mavlink_openhd_stats_telemetry_t &msg)
{
    //qDebug()<<"Got mavlink_openhd_stats_telemetry_t";
    set_curr_telemetry_rx_pps(Telemetryutil::pps_to_string(msg.curr_rx_pps));
    set_curr_telemetry_tx_pps(Telemetryutil::pps_to_string(msg.curr_tx_pps));
    set_curr_telemetry_rx_bps(Telemetryutil::bitrate_bps_to_qstring(msg.curr_rx_bps));
    set_curr_telemetry_tx_bps(Telemetryutil::bitrate_bps_to_qstring(msg.curr_tx_bps));
    set_tx_tele_packets_per_second_and_bits_per_second(StringHelper::bitrate_and_pps_to_string(msg.curr_tx_bps,msg.curr_tx_pps).c_str());
    set_rx_tele_packets_per_second_and_bits_per_second(StringHelper::bitrate_and_pps_to_string(msg.curr_rx_bps,msg.curr_rx_pps).c_str());
}

void AOHDSystem::process_x3(const mavlink_openhd_stats_wb_video_air_t &msg){
    //qDebug()<<"Got mavlink_openhd_stats_wb_video_air_t";
    if(!m_is_air){
        qDebug()<<"warning got mavlink_openhd_stats_wb_video_air from ground";
        return;
    }
    // We use parts of this message ourself, but mostly, we just forward it to the right CameraStreamModel
    // (it is
    if(msg.link_index==0 || msg.link_index==1){
        auto& cam=CameraStreamModel::instance(msg.link_index);
        cam.update_mavlink_openhd_stats_wb_video_air(msg);
    }
    // dirty
    if(msg.link_index!=0)return;
    if(x_last_dropped_packets<0){
        x_last_dropped_packets=msg.curr_dropped_frames;
    }else{
        const auto delta=msg.curr_dropped_frames-x_last_dropped_packets;
        x_last_dropped_packets=msg.curr_dropped_frames;
        if(delta>0){
            const auto elapsed_since_last=std::chrono::steady_clock::now()-m_last_tx_error_hud_message;
            if(elapsed_since_last>std::chrono::seconds(3)){
                //HUDLogMessagesModel::instance().add_message_warning("TX error (dropped packets)");
                m_last_tx_error_hud_message=std::chrono::steady_clock::now();
            }
            set_tx_is_currently_dropping_packets(true);
        }else{
            set_tx_is_currently_dropping_packets(false);
        }
    }
    set_thermal_protection_level(msg.dummy0);
}

void AOHDSystem::process_x3b(const mavlink_openhd_stats_wb_video_air_fec_performance_t &msg)
{
    if(!m_is_air){
        qDebug()<<"warning got mavlink_openhd_stats_wb_video_air_fec_performance_t from ground";
        return;
    }
    if(msg.link_index==0 || msg.link_index==1){
        auto& cam=CameraStreamModel::instance(msg.link_index);
        cam.update_mavlink_openhd_stats_wb_video_air_fec_performance(msg);
    }
}

void AOHDSystem::process_x4(const mavlink_openhd_stats_wb_video_ground_t &msg){
    //qDebug()<<"Got mavlink_openhd_stats_wb_video_ground_t";
    if(m_is_air){
         qDebug()<<"warning got mavlink_openhd_stats_wb_video_ground from air";
         return;
    }
    if(msg.link_index==0 || msg.link_index==1){
        auto& cam=CameraStreamModel::instance(msg.link_index);
        cam.update_mavlink_openhd_stats_wb_video_ground(msg);
    }
}

void AOHDSystem::process_x4b(const mavlink_openhd_stats_wb_video_ground_fec_performance_t &msg)
{
    if(m_is_air){
        qDebug()<<"warning got mavlink_openhd_stats_wb_video_ground_fec_performance_ from air";
        return;
    }
    if(msg.link_index==0 || msg.link_index==1){
        auto& cam=CameraStreamModel::instance(msg.link_index);
        cam.update_mavlink_openhd_stats_wb_video_ground_fec_performance(msg);
    }
}

void AOHDSystem::process_sys_status1(const mavlink_openhd_sys_status1_t &msg)
{
    set_wifi_hotspot_state(msg.wifi_hotspot_state);
    set_wifi_hotspot_frequency(msg.wifi_hotspot_frequency);
    set_external_devices_count(msg.external_devices_count);
}

void AOHDSystem::process_op_mode(const mavlink_openhd_wifbroadcast_gnd_operating_mode_t &msg)
{
    set_wb_gnd_operating_mode(msg.operating_mode);
    set_tx_operating_mode(msg.tx_passive_mode_is_enabled);
}

void AOHDSystem::autofech_params_if_apropriate()
{
    QSettings settings;
    const bool dev_disable_autofetch=settings.value("dev_disable_autofetch",false).toBool();
    if(dev_disable_autofetch)return;
    if(!m_is_air){
        // Ground - auto-fetch IF ;)
        if(m_wb_gnd_operating_mode==0){
            // Link is active
            if(!MavlinkSettingsModel::is_air_or_cam_param_busy()){
                // None of the air param set(s) are currently busy
                if(!MavlinkSettingsModel::instanceGround().has_params_fetched() && !MavlinkSettingsModel::instanceGround().is_x_busy()){
                    qDebug()<<"Init autofetch of groun param set";
                    // Ground has neither already fetched all params or is busy fetching already
                    MavlinkSettingsModel::instanceGround().try_refetch_all_parameters_async(true);
                }
            }
        }
    }else{
        // AIR - auto fetch IF ;)
        if(!MavlinkSettingsModel::instanceGround().is_x_busy()){
            // Ground is currently not fetching params
            if(m_curr_rx_last_packet_status_good){
                // air reports a working uplink
                if(!MavlinkSettingsModel::is_air_or_cam_param_busy()){
                    // None of the (up to 3) air param set(s) are currently fetching params
                    if(!MavlinkSettingsModel::instanceAir().has_params_fetched()){
                        qDebug()<<"Init autofetch of air param set";
                        MavlinkSettingsModel::instanceAir().try_refetch_all_parameters_async();
                    }else if(!MavlinkSettingsModel::instanceAirCamera().has_params_fetched()){
                        qDebug()<<"Init autofetch of air camera1 param set";
                        MavlinkSettingsModel::instanceAirCamera().try_refetch_all_parameters_async();
                    }else if(!MavlinkSettingsModel::instanceAirCamera2().has_params_fetched()){
                        qDebug()<<"Init autofetch of air camera2 param set";
                        MavlinkSettingsModel::instanceAirCamera2().try_refetch_all_parameters_async();
                    }
                }
            }
        }
    }
}


void AOHDSystem::update_alive()
{
    // NOTE: Since we are really resourcefully with the link, we consider the system alive if any message coming from it has
    // come through, not only a heartbeat
    // AIR: Quite lossy, and r.n we send about 2 to 3 telemetry packets per second
    // GROUND: OpenHD (can) send a lot of data to QOpenHD, since there are no bw constraints
    const int64_t tmp=m_last_message_ms;
    if(tmp<=-1) {
        update_alive_status_with_hud_message(false);
        return;
    }
    const auto elapsed_ms=QOpenHDMavlinkHelper::getTimeMilliseconds()-m_last_message_ms;
    const bool alive=elapsed_ms < (m_is_air ? 3*1000 : 2*1000);
    update_alive_status_with_hud_message(alive);
}

void AOHDSystem::update_alive_status_with_hud_message(bool alive)
{
    if(alive != m_is_alive){
        // message when state changes
        std::stringstream message;
        if(m_is_air){
            message << "Air unit ";
        }else{
            message << "Ground unit ";
        }
        if(alive){
            message << "connected";
            HUDLogMessagesModel::instance().add_message_info(message.str().c_str());
            //QOpenHD::instance().textToSpeech_sayMessage(message.str().c_str());
        }else{
            message << "disconnected";
            HUDLogMessagesModel::instance().add_message_warning(message.str().c_str());
        }
        if(!m_is_air){
            LogMessagesModel::instanceGround().add_message_debug("QOpenHD",message.str().c_str());
        }else{
            LogMessagesModel::instanceOHDAir().add_message_debug("QOpenHD",message.str().c_str());
        }
        if(alive){
            /*if(m_is_air && !MavlinkSettingsModel::instanceAir().has_params_fetched()){
                MavlinkSettingsModel::instanceAir().try_refetch_all_parameters_async();
            }
            if(m_is_air && !MavlinkSettingsModel::instanceAirCamera().has_params_fetched()){
                MavlinkSettingsModel::instanceAirCamera().try_refetch_all_parameters_async();
            }
            if(m_is_air && !MavlinkSettingsModel::instanceAirCamera2().has_params_fetched()){
                MavlinkSettingsModel::instanceAirCamera2().try_refetch_all_parameters_async();
            }
            if(!m_is_air && !MavlinkSettingsModel::instanceGround().has_params_fetched()){
                MavlinkSettingsModel::instanceGround().try_refetch_all_parameters_async();
            }*/
        }
        set_is_alive(alive);
    }
}
