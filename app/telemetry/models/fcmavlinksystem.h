#ifndef FC_MAVLINK_SYSTEM_H
#define FC_MAVLINK_SYSTEM_H

#include <QElapsedTimer>
#include <QObject>
#include <QTimer>
#include <QQmlContext>

#include "../tutil/mavlink_include.h"

#include <atomic>
#include <optional>
#include <memory>

#include "util/lqutils_include.h"

/**
 * This used to be called OpenHD and was a mix of everything, it has become FCMavlinkSystem -
 * A QT model for (fire and forget) data from the mavlink FC connected to the air unit.
 * OpenHD always has only one connected FC, so this can be a singleton, too.
 * Also, note that nothing OpenHD specific should ever make it into here - OpenHD supports raw access to
 * the Flight Controller, but it is NOT a Flight Controller ;)
 * The corresponding qml element is called _fcMavlinkSystem.
 *
 * NOTE: This is a c++ - write, qml - read only model - this greatly increases simplicity.
 * You cannot 'talk' to the FC from here - use the action class for that.
 *
 * NOTE: When adding new values, please try and be specific about their unit - e.g. add a "volt" suffix if the value is in volts.
 */
class FCMavlinkSystem : public QObject
{
    Q_OBJECT
public:
    explicit FCMavlinkSystem(QObject *parent = nullptr);
    // singleton for accessing the model from c++
    static FCMavlinkSystem& instance();
    // Process a new telemetry message coming from the FC mavlink system
    // return true if we know what to do with this message type (aka this message type has been consumed)
    bool process_message(const mavlink_message_t& msg);
    // mavlink sys id of the FC. Pretty much always 1, but it is not a hard requirement that FC always use a sys id of 1.
    // If the FC has not been discovered yet, return std::nullopt.
    std::optional<uint8_t> get_fc_sys_id();
    // Set the mavlink system reference, once discovered.
    // NOTE: We only use the system to get broadcast message(s) (pass_through) and a few more things
    bool set_system_id(int sys_id);
public: // Stuff needs to be public for qt
    // These members can be written & read from c++, but are only readable from qml (which is a common recommendation for QT application(s)).
    // Aka we just set them in c++ by calling the setter declared from the macro, which then emits the changed signal if needed
    // and therefore updates the corresponding UI element in QT
    // Note that this macro is short for the following common pattern:
    //
    // private:
    //    int m_some_value=0;
    // signals:
    //    some_value_changed(int some_value)
    // public:
    // void set_some_value(int value){
    //    if(m_some_value==value)return;
    //    m_some_value=value;
    //    emit some-value_changed();
    // }
    //
    L_RO_PROP(double, battery_current_ampere, set_battery_current_ampere, 0)
    L_RO_PROP(double, battery_voltage_volt, set_battery_voltage_volt, 0)
    // legacy, not commonly supported by FCs
    L_RO_PROP(double, battery_voltage_single_cell, set_battery_voltage_single_cell, 0)
    L_RO_PROP(int, battery_percent, set_battery_percent, 0)
    // same as battery_percent, but as an "icon"
    L_RO_PROP(QString, battery_percent_gauge, set_battery_percent_gauge, "\uf091")
    // not directly battery, but similar
    L_RO_PROP(int,battery_consumed_mah,set_battery_consumed_mah,0)
    // TODO this value is not calculated yet
    L_RO_PROP(int,battery_consumed_mah_per_km,set_battery_consumed_mah_per_km,-1)
    // Ardupilot might show the same battery as multiple batteries when more than one current sensor is used
    // (Aparently a few peole do that)
    L_RO_PROP(double, battery_id0_current_ampere, set_battery_id0_current_ampere, 0)
    L_RO_PROP(double, battery_id0_voltage_volt, set_battery_id0_voltage_volt, 0)
    L_RO_PROP(int,    battery_id0_consumed_mah,set_battery_id0_consumed_mah,0)
    L_RO_PROP(QString,battery_id0_type,set_battery_id0_type,"N/A")
    L_RO_PROP(int,    battery_id0_remaining_time_s,set_battery_id0_remaining_time_s,-1)
    L_RO_PROP(double, battery_id1_current_ampere, set_battery_id1_current_ampere, 0)
    L_RO_PROP(double, battery_id1_voltage_volt, set_battery_id1_voltage_volt, 0)
    L_RO_PROP(int,    battery_id1_consumed_mah,set_battery_id1_consumed_mah,0)
    L_RO_PROP(QString,battery_id1_type,set_battery_id1_type,"N/A")
    L_RO_PROP(int,    battery_id1_remaining_time_s,set_battery_id1_remaining_time_s,-1)

    // roll, pitch and yaw
    L_RO_PROP(double, pitch, set_pitch, 0)
    L_RO_PROP(double, roll, set_roll, 0)
    L_RO_PROP(double, yaw, set_yaw, 0)
    // mixed
    L_RO_PROP(double, throttle, set_throttle, 0)
    L_RO_PROP(float,vibration_x,set_vibration_x,0)
    L_RO_PROP(float,vibration_y,set_vibration_y,0)
    L_RO_PROP(float,vibration_z,set_vibration_z,0)
    // see alive timer
    L_RO_PROP(bool,is_alive,set_is_alive,false)
    //
    L_RO_PROP(double,lat,set_lat,0.0)
    L_RO_PROP(double,lon,set_lon,0.0)
    L_RO_PROP(double,last_lat,set_last_lat,0.0)
    L_RO_PROP(double,last_lon,set_last_lon,0.0)
    L_RO_PROP(int,satellites_visible,set_satellites_visible,0)
    L_RO_PROP(double,gps_hdop,set_gps_hdop,-1)
    L_RO_PROP(double,gps_vdop,set_gps_vdop,-1)
    // gps lock type, see: mavlink_gps_raw_int_t / gps_status.fix_type
    L_RO_PROP(int,gps_fix_type,set_gps_fix_type,0)
    L_RO_PROP(QString,gps_status_fix_type_str,set_gps_status_fix_type_str,"Unknown") // User-understandable string for UI
    // Home point (lat/lon/...) as reported by the FC via MAVLINK_MSG_ID_HOME_POSITION
    L_RO_PROP(double,home_latitude,set_home_latitude,0.0);
    L_RO_PROP(double,home_longitude,set_home_longitude,0.0);

    L_RO_PROP(double,vx,set_vx,0.0)
    L_RO_PROP(double,vy,set_vy,0.0)
    L_RO_PROP(double,vz,set_vz,0.0)
    //
    L_RO_PROP(double,altitude_rel_m,set_altitude_rel_m,0.0)
    L_RO_PROP(double,altitude_msl_m,set_altitude_msl_m,0.0)
    //
    L_RO_PROP(double,vehicle_vx_angle,set_vehicle_vx_angle,0.0);
    L_RO_PROP(double,vehicle_vy_angle,set_vehicle_vy_angle,0.0);
    L_RO_PROP(double,vehicle_vz_angle,set_vehicle_vz_angle,0.0);
    ////
    L_RO_PROP(double,wind_speed,set_wind_speed,0)
    L_RO_PROP(double,wind_direction,set_wind_direction,0)
    L_RO_PROP(float,mav_wind_direction,set_mav_wind_direction,0)
    L_RO_PROP(float,mav_wind_speed,set_mav_wind_speed,0)
    // Not openhd rc or something, but the RSSI of the (for example) OpenLRS receiver
    // value reported by the FC. Between [0...100], -1 -=> No value reported
    L_RO_PROP(int,rc_rssi_percentage,set_rc_rssi_percentage,-1);
    L_RO_PROP(int,imu_temp_degree,set_imu_temp_degree,0);
    L_RO_PROP(int,preasure_sensor_temperature_degree,set_preasure_sensor_temperature_degree,0)
    L_RO_PROP(int,preasure_sensor2_temperature_degree,set_preasure_sensor2_temperature_degree,0)
    L_RO_PROP(int,airspeed_sensor_temperature_degree,set_airspeed_sensor_temperature_degree,99)
    L_RO_PROP(int,esc_temp,set_esc_temp,0);
    L_RO_PROP(int,battery_temperature,set_battery_temperature,0)
    L_RO_PROP(QString,flight_time,set_flight_time,"00:00")
    L_RO_PROP(double,flight_distance_m,set_flight_distance_m,0)
    L_RO_PROP(double,lateral_speed,set_lateral_speed,0)  
    L_RO_PROP(double,home_distance,set_home_distance,0)
    L_RO_PROP(int,boot_time,set_boot_time,0)
    L_RO_PROP(int,hdg,set_hdg,0)
    L_RO_PROP(double,ground_speed_meter_per_second,set_ground_speed_meter_per_second,0)
    L_RO_PROP(double,air_speed_meter_per_second,set_air_speed_meter_per_second,0)

    L_RO_PROP(float,clipping_x,set_clipping_x,0.0)
    L_RO_PROP(float,clipping_y,set_clipping_y,0.0)
    L_RO_PROP(float,clipping_z,set_clipping_z,0.0)
    L_RO_PROP(float,aoa,set_aoa,0.0)
    // This is not calculated by qopenhd, it comes from the vfr hud message
    L_RO_PROP(float,vertical_speed_indicator_mps,set_vertical_speed_indicator_mps,0) //m/s], positive is up
    //
    L_RO_PROP(QString,mav_type_str,set_mav_type_str,"UNKNOWN");
    L_RO_PROP(QString,autopilot_type_str,set_autopilot_type_str,"UNKNOWN"); //R.n Generic (inav), ardu and pixhawk
    L_RO_PROP(QString, last_ping_result_flight_ctrl,set_last_ping_result_flight_ctrl,"NA")
    // update rate: here we keep track of how often we get the "MAVLINK_MSG_ID_ATTITUDE" messages.
    // (since it controlls the art. horizon). This is pretty much the only thing we perhaps need to manually set the update rate on
    L_RO_PROP(float,curr_update_rate_mavlink_message_attitude,set_curr_update_rate_mavlink_message_attitude,-1)
    // We expose the sys id for the OSD to show - note that this value should not be used by any c++ code
    L_RO_PROP(int,for_osd_sys_id,set_for_osd_sys_id,-1);
    L_RO_PROP(int,distance_sensor_distance_cm,set_distance_sensor_distance_cm,-1);
    // (GPS) reported time
    L_RO_PROP(quint64,sys_time_unix_usec,set_sys_time_unix_usec,0);
    L_RO_PROP(QString,sys_time_unix_as_str,set_sys_time_unix_as_str,"N/A");
public:
    void calculate_home_distance();
    void calculate_home_course();
    // Updates the flight time by increasing the time when armed
    void updateFlightTimer();
    // Something something luke
    void updateVehicleAngles();
    // Something somethng luke
    void updateWind();

    Q_PROPERTY(int home_course MEMBER m_home_course WRITE set_home_course NOTIFY home_course_changed)
    void set_home_course(int home_course);

    Q_PROPERTY(int home_heading MEMBER m_home_heading WRITE set_home_heading NOTIFY home_heading_changed)
    void set_home_heading(int home_heading);

    Q_PROPERTY(bool armed MEMBER m_armed WRITE set_armed NOTIFY armed_changed)
    void set_armed(bool armed);

    Q_PROPERTY(QString flight_mode MEMBER m_flight_mode WRITE set_flight_mode NOTIFY flight_mode_changed)
    void set_flight_mode(QString flight_mode);
signals:
    // mavlink
    void armed_changed(bool armed);
    void flight_mode_changed(QString flight_mode);
    void home_course_changed(int home_course);
    void home_heading_changed(int home_heading);
private:
    // other members
    bool m_armed = false;
    QString m_flight_mode = "------";

    int gps_quality_count = 0;

    int m_home_heading = 0; //this is actual global heading
    int m_home_course = 0; //this is the relative course from nose

    double speed_last_time = 0.0;

    qint64 m_flight_distance_last_time_ms= 0;
    double total_dist= 0.0;

    QElapsedTimer totalTime;
    QElapsedTimer flightTimeStart;

    std::unique_ptr<QTimer> m_flight_time_timer = nullptr;
    //
    std::unique_ptr<QTimer> m_alive_timer = nullptr;
    std::atomic<int64_t> m_last_heartbeat_ms = -1;
    std::atomic<int64_t> m_last_message_ms= -1;
    void update_alive();
    std::chrono::steady_clock::time_point m_last_update_update_rate_mavlink_message_attitude=std::chrono::steady_clock::now();
    int m_n_messages_update_rate_mavlink_message_attitude=0;
    // -----------------------
private:
    void send_message_hud_connection(bool connected);
    void send_message_arm_change(bool armed);
private:
    static bool get_SHOW_FC_MESSAGES_IN_HUD();
    // Used to calculate efficiency in mAh / km
    void recalculate_efficiency();
    double m_efficiency_last_distance_m=0;
    int m_efficiency_last_charge_consumed_mAh=0;
    std::chrono::steady_clock::time_point m_efficiency_last_update=std::chrono::steady_clock::now();
private:
    // Feature: log warning if heartbeats are received, but no "attitude" messages -
    // we use this as a hint that the telemetry rate(s) are messed up
    // Every 10 heartbeats, check when we received the last "attitude" message - if we didn't receive an attiude message in this interval,
    // log a warning
    int m_n_heartbeats=0;
    int m_n_attitude_messages=0;
private:
    std::atomic<bool> m_discovered=false;
    int m_sys_id=-1;
};



#endif // FC_MAVLINK_SYSTEM_H
