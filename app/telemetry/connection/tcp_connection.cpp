#include "tcp_connection.h"
#include "tutil/qopenhdmavlinkhelper.hpp"

#ifdef __windows__
#define _WIN32_WINNT 0x0600 // TODO dirty
#include <winsock2.h>
#include <Ws2tcpip.h> // For InetPton
#include <Windows.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#include <qdebug.h>
#include "mavlinkchannel.h"

static int tcp_socket_try_connect(const std::string remote_ip, const int remote_port, const int timeout_seconds) {
    qDebug() << "Attempting to connect to" << remote_ip.c_str() << ":" << remote_port 
             << "with timeout" << timeout_seconds << "seconds";

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        qDebug() << "Error: Cannot create socket -" << strerror(errno);
        return -1;
    }
    qDebug() << "Socket created successfully. FD:" << sockfd;

#ifdef __linux__
    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0) {
        qDebug() << "Error: Cannot set socket to non-blocking mode -" << strerror(errno);
        close(sockfd);
        return -1;
    }
    qDebug() << "Socket set to non-blocking mode.";
#endif

    struct sockaddr_in remote_addr{};
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(remote_port);
    if (inet_pton(AF_INET, remote_ip.c_str(), &remote_addr.sin_addr) <= 0) {
        qDebug() << "Error: Invalid IP address format -" << remote_ip.c_str();
        close(sockfd);
        return -1;
    }

    int connect_result = connect(sockfd, reinterpret_cast<sockaddr*>(&remote_addr), sizeof(struct sockaddr_in));
    if (connect_result == 0) {
        qDebug() << "Connection established immediately.";
        return sockfd;
    }

    if (connect_result == -1 && errno != EINPROGRESS) {
        qDebug() << "Error: Connect failed and not EINPROGRESS -" << strerror(errno);
        close(sockfd);
        return -1;
    }

    qDebug() << "Connection in progress, waiting for completion.";

    fd_set fdset;
    struct timeval tv;
    FD_ZERO(&fdset);
    FD_SET(sockfd, &fdset);
    tv.tv_sec = timeout_seconds;
    tv.tv_usec = 0;

    int rc = select(sockfd + 1, NULL, &fdset, NULL, &tv);
    if (rc != 1) {
        qDebug() << "Error: Timeout or other issue in select() -" << strerror(errno);
        close(sockfd);
        return -1;
    }

    int so_error;
    socklen_t len = sizeof(so_error);
    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
    if (so_error != 0) {
        qDebug() << "Error: Socket error after select -" << strerror(so_error);
        close(sockfd);
        return -1;
    }

    qDebug() << "Socket connected successfully. FD:" << sockfd;
    return sockfd;
}

static bool linux_send_message(int sockfd, const std::string& dest_ip, const int dest_port, const uint8_t* data, int data_len) {
    qDebug() << "Sending message to" << dest_ip.c_str() << ":" << dest_port;

    struct sockaddr_in dest_addr{};
    dest_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, dest_ip.c_str(), &dest_addr.sin_addr) <= 0) {
        qDebug() << "Error: Invalid destination IP format -" << dest_ip.c_str();
        return false;
    }
    dest_addr.sin_port = htons(dest_port);

#ifdef MSG_NOSIGNAL
    auto flags = MSG_NOSIGNAL;
#else
    auto flags = 0; // No MSG_NOSIGNAL available
#endif

    int send_len = sendto(
        sockfd,
        reinterpret_cast<const char*>(data),
        data_len,
        flags,
        reinterpret_cast<const sockaddr*>(&dest_addr),
        sizeof(dest_addr)
    );

    if (send_len != data_len) {
        qDebug() << "Error: Failed to send message. Sent bytes:" << send_len 
                 << "Expected:" << data_len << "-" << strerror(errno);
        return false;
    }

    qDebug() << "Message sent successfully. Bytes:" << send_len;
    return true;
}

void TCPConnection::stop_looping() {
    assert(m_receive_thread != nullptr);
    qDebug() << "Stopping TCPConnection loop.";

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
    m_socket_fd = -1;

    if (m_receive_thread->joinable()) {
        m_receive_thread->join();
        qDebug() << "Receive thread stopped.";
    }

    m_receive_thread = nullptr;
    qDebug() << "TCPConnection loop stopped.";
}

void TCPConnection::receive_until_stopped() {
    qDebug() << "Receiving data until stopped.";
    auto buffer = std::make_unique<std::vector<uint8_t>>(1500); // Enough for MTU 1500 bytes

    while (m_keep_receiving) {
        const auto elapsed = QOpenHDMavlinkHelper::getTimeMilliseconds() - m_last_data_ms;
        if (elapsed > 3000) {
            qDebug() << "No message received for 3 seconds. Disconnecting.";
            return;
        }

        const auto recv_len = recvfrom(
            m_socket_fd,
            reinterpret_cast<char*>(buffer->data()),
            buffer->size(),
            0,
            nullptr,
            nullptr
        );

        if (recv_len == 0) {
            qDebug() << "Received length 0. Possibly socket closed.";
            continue;
        }

        if (recv_len < 0) {
            qDebug() << "Error receiving data -" << strerror(errno);
            continue;
        }

        qDebug() << "Received data. Bytes:" << recv_len;
        process_data(buffer->data(), recv_len);
    }
}
