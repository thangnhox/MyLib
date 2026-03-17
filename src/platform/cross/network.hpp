#pragma once

#include "tnclib/utils/network.hpp"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#endif

namespace tnclib {
    namespace platform {
        class CrossNetwork final : public tnclib::utils::Network {
        public:
            CrossNetwork();
            ~CrossNetwork() override;

            bool Init() override;

            std::vector<InternetAddress> ResolveDomain(const std::string& hostname, const std::string& service, ResolutionHint hint = ResolutionHint::Unspecified) override;
            int CreateSocket(ConnectionType type, AddressFamily family = AddressFamily::IPv4) override;
            int CreateTCPSocket(AddressFamily family = AddressFamily::IPv4) override;
            int CreateUDPSocket(AddressFamily family = AddressFamily::IPv4) override;
            bool Bind(int sock, const InternetAddress& address) override;
            bool Connect(int sock, const InternetAddress& address) override;
            bool Listen(int sock, int backlog) override;
            int Accept(int sock) override;
            bool Send(int sock, const std::vector<uint8_t>& data) override;
            bool Receive(int sock, const std::function<void(std::span<uint8_t>, size_t, size_t)>& callBack) override;
            std::optional<uint16_t> GetPort(const std::string& scheme, ConnectionType type = ConnectionType::TCP) const override;
            void Close(int sock) override;

        private:
            bool initialized = false;
        }; 
    } // namespace platform
} // namespace tnclib
