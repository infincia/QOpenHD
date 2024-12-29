#include "UDPReceiver.h"
#include "common/StringHelper.hpp"
#include "common/SchedulingHelper.hpp"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define SHUT_RD SD_RECEIVE
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#include <utility>
#include <vector>
#include <sstream>
#include <array>
#include <cstring>
#include <iostream>
#include <qdebug.h>

UDPReceiver::UDPReceiver(std::string tag, Configuration config, DATA_CALLBACK onDataReceivedCallbackX)
    : m_tag(tag), m_config(config), m_on_data_received_cb(std::move(onDataReceivedCallbackX))
{
    qDebug() << "UDPReceiver " << m_tag.c_str() << "with " << m_config.to_string().c_str();
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
    }
#endif
}

UDPReceiver::~UDPReceiver() {
#ifdef _WIN32
    WSACleanup();
#endif
}

void UDPReceiver::startReceiving() {
    m_receiving = true;
    m_receive_thread = std::make_unique<std::thread>([this] {
        if (m_config.set_sched_param_max_realtime) {
            SchedulingHelper::setThreadParamsMaxRealtime();
        }
        this->receiveFromUDPLoop();
    });
}

void UDPReceiver::stopReceiving() {
    m_receiving = false;
    shutdown(m_socket, SHUT_RD);
    if (m_receive_thread->joinable()) {
        m_receive_thread->join();
    }
    m_receive_thread.reset();
#ifdef _WIN32
    closesocket(m_socket);
#else
    close(m_socket);
#endif
}

void UDPReceiver::receiveFromUDPLoop() {
    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socket == -1) {
        std::cerr << "Error creating socket\n";
        return;
    }

    int enable = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&enable, sizeof(int)) < 0) {
        std::cerr << "Error setting SO_REUSEADDR\n";
    }

#ifndef _WIN32
    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0) {
        std::cerr << "Error setting SO_REUSEPORT\n";
    }
#endif

    if (m_config.opt_os_receive_buff_size) {
        increase_socket_recv_buff_size(m_socket, m_config.opt_os_receive_buff_size.value());
    }

    struct sockaddr_in myaddr = {};
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(m_config.udp_port);

    if (m_config.udp_ip_address.has_value()) {
        if (inet_pton(AF_INET, m_config.udp_ip_address.value().c_str(), &myaddr.sin_addr) <= 0) {
            std::cerr << "Invalid IP address\n";
        }
    } else {
        myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    if (bind(m_socket, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1) {
        std::cerr << "Error binding to port " << m_config.udp_port << "\n";
        return;
    }

    auto buff = std::make_unique<std::array<uint8_t, UDP_PACKET_MAX_SIZE>>();
    sockaddr_in source = {};
    socklen_t sourceLen = sizeof(sockaddr_in);

    while (m_receiving) {
        ssize_t message_length = recvfrom(m_socket, (char *)buff->data(), UDP_PACKET_MAX_SIZE, 0, (sockaddr *)&source, &sourceLen);

#ifdef _WIN32
        if (message_length < 0) {
            int err = WSAGetLastError();
            qDebug() << "UDP Error:" << err;
            continue;
        }
#endif

        if (message_length > 0) {
            m_on_data_received_cb(buff->data(), (size_t)message_length);
            m_n_received_bytes += message_length;

            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &source.sin_addr, ip, INET_ADDRSTRLEN);
            m_sender_ip = ip;
        }
    }

#ifdef _WIN32
    closesocket(m_socket);
#else
    close(m_socket);
#endif
}

int UDPReceiver::getPort() const {
    return m_config.udp_port;
}
