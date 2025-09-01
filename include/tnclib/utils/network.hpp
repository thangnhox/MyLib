#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <mutex>

namespace tnclib {
    namespace utils {

        class Network {
        public:
            virtual ~Network() = default;

            virtual bool Init() = 0;
            virtual int CreateTCPSocket() = 0;
            virtual int CreateUDPSocket() = 0;
            virtual bool ConnectTCP(int sock, const std::string& ip, int port) = 0;
            virtual bool Send(int sock, const std::vector<uint8_t>& data) = 0;
            virtual std::vector<uint8_t> Receive(int sock) = 0;
            virtual void Close(int sock) = 0;

        public:
            /// Get the active network instance (nullptr if not created).
            static std::shared_ptr<Network> Instance();

            /// Create or return an existing network backend (thread-safe).
            static std::shared_ptr<Network> Create(const std::string& backend = "system");

            /// Register a custom backend factory by name.
            static void RegisterBackend(const std::string& name,
                                        std::function<std::shared_ptr<Network>()> factory);

        protected:
            Network() = default;

        private:
            // No copy/move to prevent slicing
            Network(const Network&) = delete;
            Network& operator=(const Network&) = delete;
            Network(Network&&) = delete;
            Network& operator=(Network&&) = delete;

        private:
            static std::shared_ptr<Network> instance;
            static std::string active_backend;
            static std::unordered_map<std::string, std::function<std::shared_ptr<Network>()>> backends;
            static std::mutex mutex;
        };

    } // namespace utils
} // namespace tnclib
