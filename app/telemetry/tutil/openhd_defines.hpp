#ifndef OPENHD_DEFINES_H
#define OPENHD_DEFINES_H

// Needs to be kept in sync with openhd

//namespace OHDDefines{
// OpenHD mavlink sys IDs
static constexpr auto OHD_SYS_ID_GROUND = 100;
static constexpr auto OHD_SYS_ID_AIR = 101;
static_assert(OHD_SYS_ID_GROUND != OHD_SYS_ID_AIR);
// other
static constexpr auto OHD_GROUND_SERVER_TCP_PORT=5760;

static constexpr auto OHD_GROUND_CLIENT_UDP_PORT_OUT = 14550;
static constexpr auto OHD_GROUND_CLIENT_UDP_PORT_IN = 14551;

// MAV_COMP_ID_CAMERA=100
static constexpr auto OHD_COMP_ID_AIR_CAMERA_PRIMARY=100;
static constexpr auto OHD_COMP_ID_AIR_CAMERA_SECONDARY=OHD_COMP_ID_AIR_CAMERA_PRIMARY+1;
// MAV_COMP_ID_ONBOARD_COMPUTER=191
static constexpr auto OHD_COMP_ID_LINK_PARAM=191;


#endif // OPENHD_DEFINES_H
