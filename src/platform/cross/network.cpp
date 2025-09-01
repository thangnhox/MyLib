#include "platform/cross/network.hpp"
#include "tnclib/services/logger.hpp"

#include <fstream>

namespace tnclib {
    namespace platform {

        CrossNetwork::CrossNetwork() = default;

        CrossNetwork::~CrossNetwork() {
        #ifdef _WIN32
            if (initialized) {
                WSACleanup();
            }
        #endif
        }

        bool CrossNetwork::Init() {
        #ifdef _WIN32
            WSADATA wsaData;
            int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
            if (result != 0) {
                LOG_ERROR("Network Utils: WSAStartup failed with error: {}", result);
                initialized = false;
                return false;
            }
        #endif
            initialized = true;
            return true;
        }

        int CrossNetwork::CreateTCPSocket() {
            if (!initialized) {
                LOG_ERROR("Network Utils: Network not initialized!");
                return -1;
            }

            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0) {
                LOG_ERROR("Network Utils: Failed to create TCP socket");
            }

            LOG_INFO("Network Utils: Created TCP socket {}", sock);
            return sock;
        }

        int CrossNetwork::CreateUDPSocket() {
            if (!initialized) {
                LOG_ERROR("Network Utils: Network not initialized!");
                return -1;
            }

            int sock = socket(AF_INET, SOCK_DGRAM, 0);
            if (sock < 0) {
                LOG_ERROR("Network Utils: Failed to create UDP socket");
            }

            LOG_INFO("Network Utils: Created UDP socket {}", sock);
            return sock;
        }

        bool CrossNetwork::ConnectTCP(int sock, const std::string &ip, int port) {
            if (!initialized) {
                LOG_ERROR("Network Utils: Network not initialized!");
                return false;
            }

            struct addrinfo hints{}, *res = nullptr;
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;

            std::string portStr = std::to_string(port);
            int status = getaddrinfo(ip.c_str(), portStr.c_str(), &hints, &res);
            if (status != 0) {
                LOG_ERROR("Network Utils: getaddrinfo failed for {}:{}", ip, port);
            #ifdef _WIN32
                LOG_ERROR("Getaddrinfo failed: {}", WSAGetLastError());
            #else
                const char* error = gai_strerror(status);
                LOG_ERROR("Getaddrinfo failed: {}", error ? error : "unknown error");
            #endif
                return false;
            }

            bool connected = false;
            for (struct addrinfo* p = res; p != nullptr; p = p->ai_next) {
                if (::connect(sock, p->ai_addr, static_cast<int>(p->ai_addrlen)) == 0) {
                    connected = true;
                    break;
                }
            }

            if (!connected) {
                LOG_ERROR("Network Utils: Unable to connect to {}:{}", ip, port);
            }

            LOG_INFO("Network Utils: Connected to {}:{} using sock {}", ip, port, sock);
            freeaddrinfo(res);
            return connected;
        }

        bool CrossNetwork::Send(int sock, const std::vector<uint8_t> &data) {
            if (!initialized) {
                LOG_ERROR("Network Utils: Network not initialized!");
                return false;
            }

            size_t totalSent = 0;
            while (totalSent < data.size()) {
                int sent = send(sock,
                                reinterpret_cast<const char*>(data.data() + totalSent),
                                static_cast<int>(data.size() - totalSent),
                                0);
                if (sent <= 0) {
                #ifdef _WIN32
                    LOG_ERROR("Network Utils: Send failed: {}", WSAGetLastError());
                #else
                    LOG_ERROR("Network Utils: Send failed");
                #endif
                    return false;
                }
                totalSent += sent;
            }

            LOG_INFO("Network Utils: Sent {} bytes through sock {}", data.size(), sock);
            return true;
        }

        bool CrossNetwork::Receive(int sock, const std::function<void(std::span<uint8_t>, size_t, size_t)>& callBack) {
            if (!initialized) {
                LOG_ERROR("Network Utils: Network not initialized!");
                return false;
            }

            LOG_INFO("Network Utils: Start receiving from sock {}", sock);
            
            std::vector<uint8_t> tempBuffer(1024);
            int received = 0;
            size_t total_received = 0;

            while (true) {
                received = recv(sock, tempBuffer.data(), tempBuffer.size(), 0);

                if (received == 0) {
                    break;
                }
                if (received < 0) {
                    LOG_ERROR("Network Utils: Receive failed");
                    return false;
                }

                std::span<uint8_t> receivedBlockView(tempBuffer.data(), received);

                total_received += received;
                callBack(receivedBlockView, received, total_received);
            }

            LOG_INFO("Network Utils: Received {} bytes from sock {}", total_received, sock);
            return true;
        }

        void CrossNetwork::Close(int sock) {
            if (!initialized) {
                LOG_WARN("Network Utils: Close called but network not initialized");
                return;
            }

            if (sock < 0) {
                LOG_WARN("Network Utils: Close called with invalid socket");
                return;
            }

        #ifdef _WIN32
            closesocket(sock);
        #else
            close(sock);
        #endif
            LOG_INFO("Network Utils: Sock {} closed", sock);
        }

    } // namespace platform
} // namespace tnclib
