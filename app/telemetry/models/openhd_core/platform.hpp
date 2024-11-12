#ifndef PLATFORM_HPP
#define PLATFORM_HPP


#include <string>
#include <sstream>

// Some things conditionally depend on the platform we are running on

// When this one shows up a bit more work has to be done to run openhd on the
// platform (probably) ;)
static constexpr int X_PLATFORM_TYPE_UNKNOWN = 0;
// Generic X86
static constexpr int X_PLATFORM_TYPE_X86 = 1;
// Numbers 10..20 are reserved for rpi
// Right now we are only interested if it is an RPI of the
// generation RPI 4 / RPI CM4 or the generation before -
// NOTE: RPI 5 is currently not supported due to the complete lack of suitable
// HW acceleration
static constexpr int X_PLATFORM_TYPE_RPI_OLD = 10;
static constexpr int X_PLATFORM_TYPE_RPI_4 = 11;
static constexpr int X_PLATFORM_TYPE_RPI_CM4 = 12;
static constexpr int X_PLATFORM_TYPE_RPI_5 = 12;

// Numbers 20..30 are reserved for rockchip
static constexpr int X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W =
    20;  // Zero 3 W
static constexpr int X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5_A = 21;
static constexpr int X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5_B = 22;
static constexpr int X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_CM3 = 24;

static constexpr int X_PLATFORM_TYPE_ROCKCHIP_RV1126_UNDEFINED = 23;  // FUTURE

// Numbers 30..35 are reserved for allwinner
static constexpr int X_PLATFORM_TYPE_ALWINNER_X20 = 30;

// @Buldo is working on openipc / sigmastar, 36..39
static constexpr int X_PLATFORM_TYPE_OPENIPC_SIGMASTAR_UNDEFINED = 36;

// Numbers 40..45 are reserved for NVIDIA
static constexpr int X_PLATFORM_TYPE_NVIDIA_XAVIER = 40;

// Numbers 46..50 are reserved for QUALCOMM
static constexpr int X_PLATFORM_TYPE_QUALCOMM_QRB5165 = 46;
static constexpr int X_PLATFORM_TYPE_QUALCOMM_QCS405 = 47;
static constexpr int X_PLATFORM_TYPE_QUALCOMM_UNKNOWN = 48;
static std::string x_platform_type_to_string(int platform_type) {
    switch (platform_type) {
    case X_PLATFORM_TYPE_UNKNOWN:
        return "UNKNOWN";
    case X_PLATFORM_TYPE_X86:
        return "X86";
    case X_PLATFORM_TYPE_RPI_OLD:
        return "RPI<=3";
    case X_PLATFORM_TYPE_RPI_4:
        return "RPI 4";
    case X_PLATFORM_TYPE_RPI_5:
        return "RPI 5";
    // RPI END
    case X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W:
        return "RADXA ZERO3W";
    case X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_CM3:
        return "RADXA CM3";
    case X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5_A:
        return "RADXA ROCK5 A";
    case X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5_B:
        return "RADXA ROCK5 B";
    case X_PLATFORM_TYPE_ROCKCHIP_RV1126_UNDEFINED:
        return "RV1126 UNDEFINED";
    // ROCK END
    case X_PLATFORM_TYPE_ALWINNER_X20:
        return "X20";
    case X_PLATFORM_TYPE_OPENIPC_SIGMASTAR_UNDEFINED:
        return "OPENIPC SIGMASTAR";
    case X_PLATFORM_TYPE_NVIDIA_XAVIER:
        return "NVIDIA_XAVIER";
    default:
        break;
    }
    std::stringstream ss;
    ss << "ERR-UNDEFINED{" << platform_type << "}";
    return ss.str();
}



#endif // PLATFORM_HPP