#include "platform/cross/network.hpp"
#include "tnclib/services/logger.hpp"

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
                LOG_ERROR("WSAStartup failed with error: {}", result);
                initialized = false;
                return false;
            }
        #endif
            initialized = true;
            return true;
        }

        int CrossNetwork::CreateTCPSocket() {
            if (!initialized) {
                LOG_ERROR("CreateTCPSocket: Network not initialized!");
                return -1;
            }

            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0) {
                LOG_ERROR("Failed to create TCP socket");
            }
            return sock;
        }

        int CrossNetwork::CreateUDPSocket() {
            if (!initialized) {
                LOG_ERROR("CreateUDPSocket: Network not initialized!");
                return -1;
            }

            int sock = socket(AF_INET, SOCK_DGRAM, 0);
            if (sock < 0) {
                LOG_ERROR("Failed to create UDP socket");
            }
            return sock;
        }

        bool CrossNetwork::ConnectTCP(int sock, const std::string &ip, int port) {
            if (!initialized) {
                LOG_ERROR("ConnectTCP: Network not initialized!");
                return false;
            }

            struct addrinfo hints{}, *res = nullptr;
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;

            std::string portStr = std::to_string(port);
            int status = getaddrinfo(ip.c_str(), portStr.c_str(), &hints, &res);
            if (status != 0) {
            #ifdef _WIN32
                LOG_ERROR("getaddrinfo failed: {}", WSAGetLastError());
            #else
                const char* error = gai_strerror(status);
                LOG_ERROR("getaddrinfo failed: {}", error ? error : "unknown error");
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
                LOG_ERROR("ConnectTCP: Unable to connect to {}:{}", ip, port);
            }

            freeaddrinfo(res);
            return connected;
        }

        bool CrossNetwork::Send(int sock, const std::vector<uint8_t> &data) {
            if (!initialized) {
                LOG_ERROR("Send: Network not initialized!");
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
                    LOG_ERROR("Send failed: {}", WSAGetLastError());
                #else
                    LOG_ERROR("Send failed");
                #endif
                    return false;
                }
                totalSent += sent;
            }
            return true;
        }

        std::vector<uint8_t> CrossNetwork::Receive(int sock, size_t bufferSize) {
            if (!initialized) {
                LOG_ERROR("Receive: Network not initialized!");
                return {};
            }

            std::vector<uint8_t> buffer(bufferSize);
            int received = recv(sock,
                                reinterpret_cast<char*>(buffer.data()),
                                static_cast<int>(bufferSize),
                                0);
            if (received <= 0) {
            #ifdef _WIN32
                LOG_ERROR("Receive failed: {}", WSAGetLastError());
            #else
                LOG_ERROR("Receive failed");
            #endif
                return {};
            }

            buffer.resize(received);
            return buffer;
        }

        void CrossNetwork::Close(int sock) {
            if (!initialized) {
                LOG_WARN("Close called but network not initialized");
                return;
            }

            if (sock < 0) {
                LOG_WARN("Close called with invalid socket");
                return;
            }

        #ifdef _WIN32
            closesocket(sock);
        #else
            close(sock);
        #endif
        }

    } // namespace platform
} // namespace tnclib
