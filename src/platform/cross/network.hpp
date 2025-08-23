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

            int CreateTCPSocket() override;
            int CreateUDPSocket() override;
            bool ConnectTCP(int sock, const std::string& ip, int port) override;
            bool Send(int sock, const std::vector<uint8_t>& data) override;
            std::vector<uint8_t> Receive(int sock, size_t bufferSize) override;
            void Close(int sock) override;

        private:
            bool initialized = false;
        }; 
    } // namespace platform
} // namespace tnclib
