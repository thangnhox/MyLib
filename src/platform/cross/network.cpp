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

        std::vector<tnclib::utils::Network::InternetAddress> CrossNetwork::ResolveDomain(const std::string& hostname, const std::string& service, ResolutionHint hint) {
            std::vector<tnclib::utils::Network::InternetAddress> results;
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
                tnclib::utils::Network::InternetAddress addr;
                
                if (p->ai_family == AF_INET) { // IPv4
                    struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
                    char ip_str[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &(ipv4->sin_addr), ip_str, sizeof(ip_str));

                    addr.port = ntohs(ipv4->sin_port);
                    addr.ip = ip_str;
                    addr.family = AddressFamily::IPv4;
                } else if (p->ai_family == AF_INET6) { // IPv6
                    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
                    char ip_str[INET6_ADDRSTRLEN];
                    inet_ntop(AF_INET6, &(ipv6->sin6_addr), ip_str, sizeof(ip_str));

                    addr.port = ntohs(ipv6->sin6_port);
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
                    af_val = AF_INET;
                    break;
                default:
                    LOG_ERROR("Network Utils: Unknown address family");
                    return -1;
            }

            int sock_type;
            switch (type) {
                case ConnectionType::SEQ:
                    sock_type = SOCK_SEQPACKET;
                    break;
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

            // log the created socket with its type and family
            LOG_INFO("Network Utils: Created socket {} (Family: {}, Type: {})", 
                sock, 
                af_val == AF_INET ? "IPv4" : (af_val == AF_INET6 ? "IPv6" : "Unknown"),
                sock_type == SOCK_STREAM ? "TCP" : (sock_type == SOCK_DGRAM ? "UDP" : "SEQ")
            );

            return sock;
        }

        int CrossNetwork::CreateTCPSocket(AddressFamily family) {
            return CreateSocket(ConnectionType::TCP, family);
        }

        int CrossNetwork::CreateUDPSocket(AddressFamily family) {
            return CreateSocket(ConnectionType::UDP, family);
        }

        bool CrossNetwork::Bind(int sock, const tnclib::utils::Network::InternetAddress& address) {
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

                    if (address.ip.empty()) {
                        sa6->sin6_addr = in6addr_any; // Bind to any address
                    } else if (inet_pton(AF_INET6, address.ip.c_str(), &(sa6->sin6_addr)) <= 0) {
                        LOG_ERROR("Network Utils: Invalid IPv6 address {}", address.ip);
                        return false;
                    }

                    if (bind(sock, (struct sockaddr*)sa6, len) < 0) {
                        LOG_ERROR("Network Utils: Failed to bind IPv6 socket {}: {}", sock, strerror(errno));
                        return false;
                    }
                    LOG_INFO("Network Utils: Bound IPv6 socket {} to {}:{}", sock, address.ip.empty() ? "*" : address.ip, address.port);

                    break;
                }
                case AddressFamily::IPv4:
                default: {
                    sockaddr_in* sa = (sockaddr_in*)&ss;
                    sa->sin_family = AF_INET;
                    sa->sin_port = htons(address.port);
                    len = sizeof(sockaddr_in);

                    if (address.ip.empty()) {
                        sa->sin_addr.s_addr = INADDR_ANY; // Bind to any address
                    } else if (inet_pton(AF_INET, address.ip.c_str(), &(sa->sin_addr)) <= 0) {
                        LOG_ERROR("Network Utils: Invalid IPv4 address {}", address.ip);
                        return false;
                    }

                    if (bind(sock, (struct sockaddr*)sa, len) < 0) {
                        LOG_ERROR("Network Utils: Failed to bind IPv4 socket {}: {}", sock, strerror(errno));
                        return false;
                    }
                    LOG_INFO("Network Utils: Bound IPv4 socket {} to {}:{}", sock, address.ip.empty() ? "*" : address.ip, address.port);
                    
                    break;
                }
            }

            return true;
        }

        bool CrossNetwork::Connect(int sock, const tnclib::utils::Network::InternetAddress& address) {
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

        bool CrossNetwork::Listen(int sock, int backlog) {
            if (!initialized) {
                LOG_ERROR("Network Utils: Network not initialized!");
                return false;
            }

            if (listen(sock, backlog) < 0) {
                LOG_ERROR("Network Utils: Listen failed on sock {}: {}", sock, strerror(errno));
                return false;
            }

            LOG_INFO("Network Utils: Listening on sock {} with backlog {}", sock, backlog);
            return true;
        }

        int CrossNetwork::Accept(int sock) {
            if (!initialized) {
                LOG_ERROR("Network Utils: Network not initialized!");
                return -1;
            }

            tnclib::utils::Network::InternetAddress outAddress;

            sockaddr_storage their_addr;
            socklen_t addr_size = sizeof(their_addr);
            int new_fd = accept(sock, (struct sockaddr *)&their_addr, &addr_size);
            if (new_fd == -1) {
                LOG_ERROR("Network Utils: Accept failed on sock {}: {}", sock, strerror(errno));
                return -1;
            }

            char ipstr[INET6_ADDRSTRLEN];
            int port;

            if (their_addr.ss_family == AF_INET) {
                sockaddr_in *s = (sockaddr_in *)&their_addr;
                port = ntohs(s->sin_port);
                inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
                outAddress.family = AddressFamily::IPv4;
            } else { // AF_INET6
                sockaddr_in6 *s = (sockaddr_in6 *)&their_addr;
                port = ntohs(s->sin6_port);
                inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
                outAddress.family = AddressFamily::IPv6;
            }

            outAddress.ip = ipstr;
            outAddress.port = port;

            LOG_INFO("Network Utils: Accepted connection on sock {} from {}:{}", sock, outAddress.ip, outAddress.port);
            return new_fd;
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
                sockaddr_storage srcAddr{};
                socklen_t addrLen = sizeof(srcAddr);
                
                received = recvfrom(sock, tempBuffer.data(), tempBuffer.size(), 0, (struct sockaddr *)&srcAddr, &addrLen);

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

        std::optional<uint16_t> CrossNetwork::GetPort(const std::string& scheme, ConnectionType type) const {
            if (!initialized) {
                LOG_ERROR("Network Utils: Network not initialized!");
                return std::nullopt;
            }

            std::string connection_type;
            switch (type) {
                case ConnectionType::SEQ:
                    connection_type = "SEQ";
                    break;
                case ConnectionType::TCP:
                    connection_type = "TCP";
                    break;
                case ConnectionType::UDP:
                    connection_type = "UDB";
                    break;
                default:
                    LOG_ERROR("Network Utils: Unknown connection type");
                    return std::nullopt;
            }

            servent* service = getservbyname(scheme.c_str(), connection_type.c_str());

            if (service != nullptr) {
                // service->s_port is in Network Byte Order (Big Endian).
                // ntohs (Network TO Host Short) converts it to the local CPU's format.
                return ntohs(service->s_port);
            }

            LOG_ERROR("Network Utils: Failed to convert scheme to port");
            return std::nullopt;

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
