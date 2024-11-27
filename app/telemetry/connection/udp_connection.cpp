#include "udp_connection.h"
#include "tutil/qopenhdmavlinkhelper.hpp"

#ifdef __windows__
#define _WIN32_WINNT 0x0600 //TODO dirty
#include <winsock2.h>
#include <Ws2tcpip.h> // For InetPton
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <qdebug.h>
#include "mavlinkchannel.h"

#ifdef WINDOWS
#define GET_ERROR(_x) WSAGetLastError()
#else
#define GET_ERROR(_x) strerror(_x)
#endif

#ifdef __windows__
#endif


UDPConnection::UDPConnection(const std::string local_ip,const int local_port,MAV_MSG_CB cb)
    :m_local_ip(local_ip),m_local_port(local_port),m_cb(cb)
{

}

UDPConnection::~UDPConnection()
{
    stop_looping_if();
}


void UDPConnection::start_looping()
{
    assert(m_receive_thread==nullptr);
    m_keep_receiving=true;
    m_receive_thread=std::make_unique<std::thread>(&UDPConnection::loop_receive,this);
}

void UDPConnection::stop_looping()
{
    assert(m_receive_thread!=nullptr);
    qDebug()<<"UDP stop - begin";
    m_keep_receiving=false;
#ifdef __windows__
    shutdown(m_socket_fd, SD_BOTH);

    closesocket(m_socket_fd);

    WSACleanup();
#else
    // This should interrupt a recv/recvfrom call.
    shutdown(m_socket_fd, SHUT_RDWR);

    // But on Mac, closing is also needed to stop blocking recv/recvfrom.
    close(m_socket_fd);
#endif
    if(m_receive_thread){
        m_receive_thread->join();
    }
    m_receive_thread=nullptr;
    qDebug()<<"UDP stop - end";
}

void UDPConnection::send_message(const mavlink_message_t &msg)
{
    auto opt_remote=get_current_remote();
    if(opt_remote.has_value()){
        const Remote& remote=opt_remote.value();
        struct sockaddr_in dest_addr {};
        dest_addr.sin_family = AF_INET;
        inet_pton(AF_INET, remote.ip.c_str(), &dest_addr.sin_addr.s_addr);
        dest_addr.sin_port = htons(remote.port);

        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t buffer_len = mavlink_msg_to_send_buffer(buffer, &msg);

        const auto send_len = sendto(
            m_socket_fd,
            reinterpret_cast<char*>(buffer),
            buffer_len,
            0,
            reinterpret_cast<const sockaddr*>(&dest_addr),
            sizeof(dest_addr));

        if (send_len != buffer_len) {
            qDebug()<<"Cannot send data to "<<remote.to_string().c_str();
        }
    }
}

bool UDPConnection::threadsafe_is_alive(){
    const int32_t now_ms=QOpenHDMavlinkHelper::getTimeMilliseconds();;
    const auto elapsed=now_ms-m_last_data_ms;
    return elapsed <= 3*1000;
}

bool UDPConnection::is_looping()
{
    return m_receive_thread!=nullptr;
}

void UDPConnection::stop_looping_if()
{
    if(is_looping())stop_looping();
}

void UDPConnection::process_data(const uint8_t *data, int data_len)
{
    mavlink_message_t msg;
    for (int i = 0; i < data_len; i++) {
        uint8_t res = mavlink_parse_char(m_mav_channel, (uint8_t)data[i], &msg, &m_recv_status);
        if (res) {
            process_mavlink_message(msg);
        }
    }
}

void UDPConnection::process_mavlink_message(mavlink_message_t message)
{
    m_cb(message);
}

void UDPConnection::loop_receive()
{
    while(m_keep_receiving){
        qDebug()<<"UDP start receiving on "<<m_local_ip.c_str()<<":"<<m_local_port;
        connect_once();
        if(m_keep_receiving)std::this_thread::sleep_for(std::chrono::seconds(2));// try again in X seconds
    }
}

bool UDPConnection::setup_socket()
{
#ifdef __windows__
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        qDebug() << "Error: Winsock failed, error: %d", WSAGetLastError();
        return false;
    }
#else
    m_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (m_socket_fd < 0) {
        qDebug()<<"Cannot create socket"<<strerror(errno);
        return false;
    }

    struct sockaddr_in addr {};
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, m_local_ip.c_str(), &(addr.sin_addr));
    addr.sin_port = htons(m_local_port);

    // TODO needed ?
    // Without setting reuse, this might block infinite on some platforms
    /*int enable = 1;
    if (setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){
        LogWarn()<<"Cannot set socket reuse"; // warn,but continue anyway.
    }*/
    if (bind(m_socket_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        qDebug()<<"Cannot bind port "<<strerror(errno);
        // TODO finalize merge
/*if (errno==98){ //port already bound error code
if(_local_port==14550){
    _local_port=14551;
} else if (_local_port == 14551){
    _local_port= 14550;
} else {
    _local_port= m_local_port; //finally back to whatever is set if its not 14550 or 14551.. unlikely
}
}*/
        close(m_socket_fd);
        return false;
    }
#endif
    return true;
}

void UDPConnection::connect_once()
{
    const bool success=setup_socket();
    if(success){
        // Enough for MTU 1500 bytes.
        auto buffer=std::make_unique<std::vector<uint8_t>>();
        buffer->resize(1500);

        while (m_keep_receiving) {
            struct sockaddr_in src_addr = {};
            socklen_t src_addr_len = sizeof(src_addr);
            const auto recv_len = recvfrom(
                m_socket_fd,
                (char*)buffer->data(),
                buffer->size(),
                0,
                reinterpret_cast<struct sockaddr*>(&src_addr),
                &src_addr_len);
            //qDebug()<<"Gt data";

            if (recv_len == 0) {
                // This can happen when shutdown is called on the socket,
                // therefore we check _should_exit again.
                continue;
            }

            if (recv_len < 0) {
                // This happens on destruction when close(_socket_fd) is called,
                // therefore be quiet.
                // LogErr() << "recvfrom error: " << GET_ERROR(errno);
                continue;
            }
            const std::string remote_ip=inet_ntoa(src_addr.sin_addr);
            const int remote_port=ntohs(src_addr.sin_port);
            set_remote(remote_ip,remote_port);
            m_last_data_ms=QOpenHDMavlinkHelper::getTimeMilliseconds();
            process_data(buffer->data(),recv_len);
        }
    }
    // TODO close socket
    // set the remote back to unknown
    clear_remote();
}

std::optional<UDPConnection::Remote> UDPConnection::get_current_remote()
{
    std::lock_guard<std::mutex> lock(m_remote_nutex);
    if(m_curr_remote.has_value()){
        return m_curr_remote.value();
    }
    return std::nullopt;
}


void UDPConnection::set_remote(const std::string ip, int port)
{
    std::lock_guard<std::mutex> lock(m_remote_nutex);
    if(m_curr_remote.has_value()){
        auto& remote=m_curr_remote.value();
        if(remote.ip!=ip || remote.port != port){
            auto new_remote=Remote{ip,port};
            qDebug()<<"Remote changed from "<<remote.to_string().c_str()<<" to "<<new_remote.to_string().c_str();
            m_curr_remote=remote;
        }
    }else{
        auto new_remote=Remote{ip,port};
        qDebug()<<"Got remote "<<new_remote.to_string().c_str();
        m_curr_remote=new_remote;
    }
}

void UDPConnection::clear_remote()
{
    std::lock_guard<std::mutex> lock(m_remote_nutex);
    m_curr_remote=std::nullopt;
}

