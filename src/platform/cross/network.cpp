#include "platform/cross/network.hpp"
#include "tnclib/services/logger.hpp"

#include <fstream>
#include <cstring>
#include <cerrno>

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

        std::vector<tnclib::utils::Network::ResolvedAddress> CrossNetwork::ResolveDomain(const std::string& hostname, const std::string& service, ResolutionHint hint) {
            std::vector<tnclib::utils::Network::ResolvedAddress> results;
            struct addrinfo hints, *res, *p;
            int status;

            memset(&hints, 0, sizeof hints);
            hints.ai_socktype = SOCK_STREAM;

            switch (hint) {
                case ResolutionHint::IPv4:
                    hints.ai_family = AF_INET;
                    break;
                case ResolutionHint::IPv6:
                    hints.ai_family = AF_INET6;
                    break;
                default:
                    hints.ai_family = AF_UNSPEC;
            }

            if ((status = getaddrinfo(hostname.c_str(), service.c_str(), &hints, &res)) != 0) {
                LOG_ERROR("Network Utils: getaddrinfo failed for {}", hostname);
            #ifdef _WIN32
                LOG_ERROR("Network Utils: {}", WSAGetLastError());
            #else
                const char* error = gai_strerror(status);
                LOG_ERROR("Network Utils: {}", error ? error : "unknown error");
            #endif
                return results;
            }

            for(p = res; p != NULL; p = p->ai_next) {
                tnclib::utils::Network::ResolvedAddress addr;
                addr.port = std::stoi(service);
                
                if (p->ai_family == AF_INET) { // IPv4
                    struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
                    char ip_str[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &(ipv4->sin_addr), ip_str, sizeof(ip_str));
                    addr.ip = ip_str;
                    addr.family = AddressFamily::IPv4;
                } else if (p->ai_family == AF_INET6) { // IPv6
                    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
                    char ip_str[INET6_ADDRSTRLEN];
                    inet_ntop(AF_INET6, &(ipv6->sin6_addr), ip_str, sizeof(ip_str));
                    addr.ip = ip_str;
                    addr.family = AddressFamily::IPv6;
                }

                results.push_back(addr);
            }

            freeaddrinfo(res);
            return results;
        }

        int CrossNetwork::CreateSocket(ConnectionType type, AddressFamily family) {
            if (!initialized) {
                LOG_ERROR("Network Utils: Network not initialized!");
                return -1;
            }

            int af_val;
            switch (family) {
                case AddressFamily::IPv6:
                    af_val = AF_INET6;
                    break;
                case AddressFamily::IPv4:
                case AddressFamily::Default:
                default:
                    af_val = AF_INET; // default to IPv4
                    break;
            }

            int sock_type;
            switch (type) {
                case ConnectionType::TCP:
                    sock_type = SOCK_STREAM;
                    break;
                case ConnectionType::UDP:
                    sock_type = SOCK_DGRAM;
                    break;
                default:
                    LOG_ERROR("Network Utils: Unknown connection type");
                    return -1;
            }

            int sock = socket(af_val, sock_type, 0);
            if (sock < 0) {
                LOG_ERROR("Network Utils: Failed to create socket");
                return -1;
            }

            LOG_INFO("Network Utils: Created {} socket {} with family {}",
                     type == ConnectionType::TCP ? "TCP" : "UDP",
                     sock,
                     af_val == AF_INET6 ? "IPv6" : "IPv4");
            return sock;
        }

        int CrossNetwork::CreateTCPSocket(AddressFamily family) {
            return CreateSocket(ConnectionType::TCP, family);
        }

        int CrossNetwork::CreateUDPSocket(AddressFamily family) {
            return CreateSocket(ConnectionType::UDP, family);
        }

        bool CrossNetwork::Connect(int sock, const tnclib::utils::Network::ResolvedAddress& address) {
            int result = -1;
            sockaddr_storage ss; // A generic structure to hold either sockaddr_in or sockaddr_in6
            socklen_t len = 0;

            memset(&ss, 0, sizeof(ss));

            switch (address.family) {
                case AddressFamily::IPv6: {
                    sockaddr_in6* sa6 = (sockaddr_in6*)&ss;
                    sa6->sin6_family = AF_INET6;
                    sa6->sin6_port = htons(address.port);
                    len = sizeof(sockaddr_in6);

                    if (inet_pton(AF_INET6, address.ip.c_str(), &(sa6->sin6_addr)) <= 0) {
                        LOG_ERROR("Network Utils: Invalid IPv6 address {}", address.ip);
                        return false;
                    }
                    break;
                }
                case AddressFamily::IPv4:
                default: {
                    sockaddr_in* sa = (sockaddr_in*)&ss;
                    sa->sin_family = AF_INET;
                    sa->sin_port = htons(address.port);
                    len = sizeof(sockaddr_in);

                    if (inet_pton(AF_INET, address.ip.c_str(), &(sa->sin_addr)) <= 0) {
                        LOG_ERROR("Network Utils: Invalid IPv4 address {}", address.ip);
                        return false;
                    }
                    break;
                }
            }

            result = connect(sock, (struct sockaddr*)&ss, len);

            if (result != 0) {
                LOG_ERROR("Network Utils: Failed to connect to {}:{} with error: {}", 
                        address.ip, address.port, strerror(errno));
                return false;
            }

            LOG_INFO("Network Utils: Connected to {}:{} using sock {}", address.ip, address.port, sock);

            return true;
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
