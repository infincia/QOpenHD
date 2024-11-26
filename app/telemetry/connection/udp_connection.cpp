#include "udp_connection.h"
#include "tutil/qopenhdmavlinkhelper.hpp"

#ifdef __windows__
#define _WIN32_WINNT 0x0600 // TODO dirty
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

UDPConnection::UDPConnection(const std::string local_ip, const int local_port, MAV_MSG_CB cb)
    : m_local_ip(local_ip), m_local_port(local_port), m_cb(cb)
{
    qDebug() << "UDPConnection initialized with local IP:" << local_ip.c_str()
             << "Port:" << local_port;
}

UDPConnection::~UDPConnection()
{
    qDebug() << "Destroying UDPConnection...";
    stop_looping_if();
}

void UDPConnection::start_looping()
{
    qDebug() << "Starting UDP receiving loop...";
    assert(m_receive_thread == nullptr);
    m_keep_receiving = true;
    m_receive_thread = std::make_unique<std::thread>(&UDPConnection::loop_receive, this);
    qDebug() << "UDP receiving loop started.";
}

void UDPConnection::stop_looping()
{
    assert(m_receive_thread != nullptr);
    qDebug() << "Stopping UDP receiving loop...";

    m_keep_receiving = false;

#ifdef __windows__
    if (shutdown(m_socket_fd, SD_BOTH) == SOCKET_ERROR) {
        qDebug() << "Error: Failed to shutdown socket -" << WSAGetLastError();
    }
    if (closesocket(m_socket_fd) == SOCKET_ERROR) {
        qDebug() << "Error: Failed to close socket -" << WSAGetLastError();
    }
    WSACleanup();
#else
    if (shutdown(m_socket_fd, SHUT_RDWR) < 0) {
        qDebug() << "Error: Failed to shutdown socket -" << strerror(errno);
    }
    if (close(m_socket_fd) < 0) {
        qDebug() << "Error: Failed to close socket -" << strerror(errno);
    }
#endif

    if (m_receive_thread && m_receive_thread->joinable()) {
        m_receive_thread->join();
        qDebug() << "UDP receive thread stopped.";
    }
    m_receive_thread = nullptr;
    qDebug() << "UDP receiving loop stopped.";
}

void UDPConnection::send_message(const mavlink_message_t &msg)
{
    qDebug() << "Preparing to send message...";
    auto opt_remote = get_current_remote();
    if (!opt_remote.has_value()) {
        qDebug() << "Error: No remote target set. Cannot send message.";
        return;
    }

    const Remote& remote = opt_remote.value();
    struct sockaddr_in dest_addr{};
    dest_addr.sin_family = AF_INET;
    inet_pton(AF_INET, remote.ip.c_str(), &dest_addr.sin_addr.s_addr);
    dest_addr.sin_port = htons(remote.port);

    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    uint16_t buffer_len = mavlink_msg_to_send_buffer(buffer, &msg);

    qDebug() << "Sending message to" << remote.ip.c_str() << ":" << remote.port;

    const auto send_len = sendto(
        m_socket_fd,
        reinterpret_cast<char*>(buffer),
        buffer_len,
        0,
        reinterpret_cast<const sockaddr*>(&dest_addr),
        sizeof(dest_addr));

    if (send_len != buffer_len) {
        qDebug() << "Error: Failed to send data to" << remote.to_string().c_str() << "-"
                 << strerror(errno);
    } else {
        qDebug() << "Message sent successfully to" << remote.to_string().c_str()
                 << "Bytes sent:" << send_len;
    }
}

void UDPConnection::loop_receive()
{
    while (m_keep_receiving) {
        qDebug() << "Starting UDP receive loop on IP:" << m_local_ip.c_str()
                 << "Port:" << m_local_port;

        connect_once();

        if (m_keep_receiving) {
            qDebug() << "Retrying connection in 2 seconds...";
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
    qDebug() << "Exiting UDP receive loop.";
}

bool UDPConnection::setup_socket()
{
    qDebug() << "Setting up UDP socket...";
#ifdef __windows__
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        qDebug() << "Error: Winsock startup failed -" << WSAGetLastError();
        return false;
    }
#endif

    m_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket_fd < 0) {
        qDebug() << "Error: Cannot create socket -" << strerror(errno);
        return false;
    }
    qDebug() << "Socket created successfully. FD:" << m_socket_fd;

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, m_local_ip.c_str(), &(addr.sin_addr));
    addr.sin_port = htons(m_local_port);

    if (bind(m_socket_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        qDebug() << "Error: Cannot bind port -" << strerror(errno);
        close(m_socket_fd);
        return false;
    }
    qDebug() << "Socket bound successfully to IP:" << m_local_ip.c_str()
             << "Port:" << m_local_port;
    return true;
}

void UDPConnection::connect_once()
{
    const bool success = setup_socket();
    if (success) {
        qDebug() << "Socket setup successful. Listening for data...";
        auto buffer = std::make_unique<std::vector<uint8_t>>(1500); // Enough for MTU 1500 bytes

        while (m_keep_receiving) {
            struct sockaddr_in src_addr{};
            socklen_t src_addr_len = sizeof(src_addr);

            const auto recv_len = recvfrom(
                m_socket_fd,
                reinterpret_cast<char*>(buffer->data()),
                buffer->size(),
                0,
                reinterpret_cast<struct sockaddr*>(&src_addr),
                &src_addr_len);

            if (recv_len == 0) {
                qDebug() << "Received zero-length packet. Ignoring.";
                continue;
            }

            if (recv_len < 0) {
                qDebug() << "Error: Data reception failed -" << strerror(errno);
                continue;
            }

            std::string remote_ip = inet_ntoa(src_addr.sin_addr);
            int remote_port = ntohs(src_addr.sin_port);

            qDebug() << "Received data from IP:" << remote_ip.c_str()
                     << "Port:" << remote_port
                     << "Bytes:" << recv_len;

            set_remote(remote_ip, remote_port);
            m_last_data_ms = QOpenHDMavlinkHelper::getTimeMilliseconds();
            process_data(buffer->data(), recv_len);
        }
    } else {
        qDebug() << "Socket setup failed. Cannot listen for data.";
    }

    clear_remote();
    qDebug() << "Connection closed and remote cleared.";
}