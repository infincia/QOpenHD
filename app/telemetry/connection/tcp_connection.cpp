#include "tcp_connection.h"
#include "tutil/qopenhdmavlinkhelper.hpp"

#ifdef _WIN32
#define _WIN32_WINNT 0x0600 // Windows Vista and above
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#endif
#include <qdebug.h>
#include "mavlinkchannel.h"

static int tcp_socket_try_connect(const std::string remote_ip, const int remote_port, const int timeout_seconds) {
    qDebug() << "Attempting to connect to:" << remote_ip.c_str() << ":" << remote_port << "with timeout:" << timeout_seconds << "s";

    int sockfd;
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        qDebug() << "WSAStartup failed:" << WSAGetLastError();
        return -1;
    }
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET) {
        qDebug() << "Cannot create socket:" << WSAGetLastError();
        WSACleanup();
        return -1;
    }
    u_long mode = 1; // Non-blocking mode
    if (ioctlsocket(sockfd, FIONBIO, &mode) != NO_ERROR) {
        qDebug() << "Cannot set non-blocking mode:" << WSAGetLastError();
        closesocket(sockfd);
        WSACleanup();
        return -1;
    }
#else
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        qDebug() << "Cannot create socket:" << strerror(errno);
        return -1;
    }
    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0) {
        qDebug() << "Cannot set non-blocking mode:" << strerror(errno);
        close(sockfd);
        return -1;
    }
#endif

    struct sockaddr_in remote_addr {};
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(remote_port);
    if (inet_pton(AF_INET, remote_ip.c_str(), &remote_addr.sin_addr) <= 0) {
        qDebug() << "Invalid address:" << remote_ip.c_str();
#ifdef _WIN32
        closesocket(sockfd);
        WSACleanup();
#else
        close(sockfd);
#endif
        return -1;
    }

    const int connect_result = connect(sockfd, reinterpret_cast<sockaddr*>(&remote_addr), sizeof(remote_addr));
    if (connect_result == 0) {
        qDebug() << "Connected immediately.";
        return sockfd;
    }
#ifdef _WIN32
    if (connect_result == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) {
        qDebug() << "Connection error:" << WSAGetLastError();
        closesocket(sockfd);
        WSACleanup();
        return -1;
    }
#else
    if (connect_result == -1 && errno != EINPROGRESS) {
        qDebug() << "Connection error:" << strerror(errno);
        close(sockfd);
        return -1;
    }
#endif

    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(sockfd, &fdset);

    struct timeval tv;
    tv.tv_sec = timeout_seconds;
    tv.tv_usec = 0;

    const int select_result = select(sockfd + 1, NULL, &fdset, NULL, &tv);
    if (select_result != 1) {
        qDebug() << "Connection timed out or failed.";
#ifdef _WIN32
        closesocket(sockfd);
        WSACleanup();
#else
        close(sockfd);
#endif
        return -1;
    }

    // Check for socket errors
    int so_error;
    socklen_t len = sizeof(so_error);
#ifdef _WIN32
    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&so_error), &len);
    if (so_error != 0) {
        qDebug() << "Socket error after select:" << WSAGetLastError();
        closesocket(sockfd);
        WSACleanup();
        return -1;
    }
#else
    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
    if (so_error != 0) {
        qDebug() << "Socket error after select:" << strerror(so_error);
        close(sockfd);
        return -1;
    }
#endif

    qDebug() << "Connection established successfully.";
    return sockfd;
}

static bool linux_send_message(int sockfd, const std::string& dest_ip, const int dest_port, const uint8_t* data, int data_len) {
    struct sockaddr_in dest_addr {};
    dest_addr.sin_family = AF_INET;
    inet_pton(AF_INET, dest_ip.c_str(), &dest_addr.sin_addr.s_addr);
    dest_addr.sin_port = htons(dest_port);

#ifdef MSG_NOSIGNAL
    auto flags = MSG_NOSIGNAL;
#else
    auto flags = 0; // No MSG_NOSIGNAL available, handle it accordingly
#endif
    const auto send_len = sendto(
        sockfd,
        reinterpret_cast<const char*>(data),
        data_len,
        flags,
        reinterpret_cast<const sockaddr*>(&dest_addr),
        sizeof(dest_addr));

    if (send_len != data_len) {
        qDebug() << "Failed to send message. Sent:" << send_len << "Expected:" << data_len;
        return false;
    }
    return true;
}

TCPConnection::TCPConnection(MAV_MSG_CB cb, std::string remote_ip, int remote_port, int mavlink_channel)
    : m_cb(cb), m_remote_ip(remote_ip), m_remote_port(remote_port), m_mav_channel(mavlink_channel) {}

TCPConnection::~TCPConnection() {
    if (is_looping()) {
        stop_looping();
    }
}

void TCPConnection::set_remote(std::string remote_ip, int remote_port) {
    assert(m_receive_thread == nullptr);
    m_remote_ip = remote_ip;
    m_remote_port = remote_port;
}

bool TCPConnection::is_looping() {
    return m_receive_thread != nullptr;
}

void TCPConnection::start_looping() {
    assert(m_receive_thread == nullptr);
    m_keep_receiving = true;
    m_receive_thread = std::make_unique<std::thread>(&TCPConnection::loop_connect_receive, this);
}

void TCPConnection::stop_looping_if() {
    if (is_looping()) stop_looping();
}

void TCPConnection::stop_looping() {
    assert(m_receive_thread != nullptr);
    qDebug() << "Stopping connection";
    m_keep_receiving = false;
#ifdef _WIN32
    shutdown(m_socket_fd, SD_BOTH);
    closesocket(m_socket_fd);
    WSACleanup();
#else
    shutdown(m_socket_fd, SHUT_RDWR);
    close(m_socket_fd);
#endif
    m_receive_thread->join();
    m_receive_thread = nullptr;
    qDebug() << "Connection stopped.";
}

void TCPConnection::send_message(const mavlink_message_t& msg) {
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    const int buffer_len = mavlink_msg_to_send_buffer(buffer, &msg);
    if (!m_keep_receiving) {
        return; // Otherwise sendto blocks
    }
    if (m_socket_fd < 0) {
        qDebug() << "Cannot send message: Socket not connected.";
        return;
    }
    if (!linux_send_message(m_socket_fd, m_remote_ip, m_remote_port, buffer, buffer_len)) {
        qDebug() << "Failed to send MAVLink message.";
    }
}

bool TCPConnection::threadsafe_is_alive() {
    const int32_t now_ms = QOpenHDMavlinkHelper::getTimeMilliseconds();
    const auto elapsed = now_ms - m_last_data_ms;
    return elapsed <= 3 * 1000;
}

void TCPConnection::process_data(const uint8_t* data, int data_len) {
    m_last_data_ms = QOpenHDMavlinkHelper::getTimeMilliseconds();
    mavlink_message_t msg;
    for (int i = 0; i < data_len; i++) {
        uint8_t res = mavlink_parse_char(m_mav_channel, data[i], &msg, &m_recv_status);
        if (res) {
            process_mavlink_message(msg);
        }
    }
}

void TCPConnection::process_mavlink_message(mavlink_message_t message) {
    m_cb(message);
}

void TCPConnection::receive_until_stopped() {
    auto buffer = std::make_unique<std::vector<uint8_t>>(1500);
    while (m_keep_receiving) {
        const auto elapsed_last_message = QOpenHDMavlinkHelper::getTimeMilliseconds() - m_last_data_ms;
        if (elapsed_last_message > 3 * 1000) {
            qDebug() << "No message for more than 3 seconds, disconnecting.";
            return;
        }
        const auto recv_len = recvfrom(
            m_socket_fd,
            reinterpret_cast<char*>(buffer->data()),
            buffer->size(),
            0,
            nullptr,
            nullptr);

        if (recv_len == 0) {
            continue;
        }

        if (recv_len < 0) {
            continue;
        }
        process_data(buffer->data(), recv_len);
    }
}

void TCPConnection::loop_connect_receive() {
    qDebug() << "Starting connection loop.";
    while (m_keep_receiving) {
        m_socket_fd = tcp_socket_try_connect(m_remote_ip, m_remote_port, 3);
        if (m_socket_fd > 0) {
            m_last_data_ms = QOpenHDMavlinkHelper::getTimeMilliseconds();
            receive_until_stopped();
            qDebug() << "Exited receive loop.";
#ifdef _WIN32
            closesocket(m_socket_fd);
#else
            close(m_socket_fd);
#endif
            m_socket_fd = -1;
        } else {
            int count = 0;
            while (m_keep_receiving && count < 3) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                count++;
            }
        }
    }
    qDebug() << "Connection loop ended.";
}
