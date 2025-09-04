#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <span>
#include <unordered_map>
#include <mutex>
#include <cstdint>

namespace tnclib {
    namespace utils {

        class Network {
        public:
            enum class AddressFamily : int {
                Default = 0,
                IPv4 = 4,
                IPv6 = 6
            };

            enum class ConnectionType : int {
                TCP = 0,
                UDP = 1
            };

            enum class ResolutionHint : int {
                Unspecified,
                IPv4,
                IPv6
            };

            struct ResolvedAddress {
                std::string ip;
                AddressFamily family;
                int port;
            };

        public:
            virtual ~Network() = default;

            virtual bool Init() = 0;

            /**
             * @brief Resolves a domain name into a list of network addresses.
             *
             * This pure virtual function provides an abstract interface for a DNS resolver.
             * It translates a human-readable hostname (e.g., "www.example.com") into one
             * or more network addresses (e.g., "93.184.216.34"). **A default implementation is provided.**
             *
             * @param hostname The domain name or IP address to resolve.
             * @param service The service name (e.g., "http" or "80"), which can influence the resolution
             * based on the protocol. An empty string can be used if not applicable.
             * @param hint A hint to guide the resolution process, such as specifying the
             * desired address family (e.g., IPv4 or IPv6). Defaults to ResolutionHint::Unspecified.
             * @return A std::vector of ResolvedAddress objects, each representing a resolved
             * network address. The vector will be empty if the resolution fails.
             */
            virtual std::vector<ResolvedAddress> ResolveDomain(const std::string& hostname, const std::string& service, ResolutionHint hint = ResolutionHint::Unspecified) = 0;

            /**
             * @brief Creates a new network socket of a specified type.
             *
             * This pure virtual function provides an abstract interface for creating sockets.
             * It is implemented by a concrete derived class to handle the actual socket
             * creation for the specific platform or library. **A default implementation is provided,**
             * **making this interface usable without requiring a custom derived class.** This method
             * allows for polymorphic creation of different socket types based on a runtime parameter.
             *
             * @param type The type of socket to create (e.g., ConnectionType::TCP or ConnectionType::UDP).
             * @param family The address family for the socket (e.g., AddressFamily::IPv4 or AddressFamily::IPv6).
             * Defaults to AddressFamily::Default, which typically resolves to the most appropriate
             * family for the operating environment.
             * @return An integer representing the newly created socket handle or descriptor.
             * Returns -1 or a platform-specific error code on failure.
             */
            virtual int CreateSocket(ConnectionType type, AddressFamily family = AddressFamily::Default) = 0;

            /**
             * @brief Creates a new TCP (Transmission Control Protocol) socket.
             *
             * This is a specialized pure virtual function for creating a reliable,
             * connection-oriented TCP socket. Use this for communication that requires
             * guaranteed delivery, such as file transfers or secure connections.
             * **A default implementation is provided.**
             *
             * @param family The address family for the socket (e.g., AddressFamily::IPv4 or AddressFamily::IPv6).
             * Defaults to AddressFamily::Default.
             * @return An integer representing the new socket handle.
             * Returns -1 or a platform-specific error code on failure.
             */
            virtual int CreateTCPSocket(AddressFamily family = AddressFamily::Default) = 0;

            /**
             * @brief Creates a new UDP (User Datagram Protocol) socket.
             *
             * This is a specialized pure virtual function for creating a fast,
             * connectionless UDP socket. It is ideal for applications where
             * speed is prioritized over guaranteed delivery, such as real-time
             * video streaming or online gaming. **A default implementation is provided.**
             *
             * @param family The address family for the socket (e.g., AddressFamily::IPv4 or AddressFamily::IPv6).
             * Defaults to AddressFamily::Default.
             * @return An integer representing the new socket handle.
             * Returns -1 or a platform-specific error code on failure.
             */
            virtual int CreateUDPSocket(AddressFamily family = AddressFamily::Default) = 0;

            /**
             * @brief Establishes a connection to a remote address on a given socket.
             *
             * This pure virtual function provides an abstract interface for establishing a network connection.
             * It is implemented by a concrete derived class to perform the actual connection operation.
             * **A default implementation is provided.**
             *
             * @param sock The integer handle or descriptor of the socket to connect. This socket must be a
             * valid, previously created socket, typically of a connection-oriented type like TCP.
             * @param address The ResolvedAddress object containing the remote endpoint's IP address and port
             * to which the socket should connect.
             * @return Returns true on a successful connection, and false if the connection fails.
             */
            virtual bool Connect(int sock, const ResolvedAddress& address) = 0;

            /**
             * @brief Sends data over a connected socket.
             *
             * This pure virtual function provides an abstract interface for sending a buffer of bytes
             * through a socket. It is implemented by a concrete derived class to handle the
             * platform-specific send operation. **A default implementation is provided.**
             *
             * @param sock The integer handle or descriptor of the socket to use for sending.
             * @param data A constant reference to a vector of unsigned 8-bit integers (bytes) to be sent.
             * @return Returns true on a successful send operation, and false if an error occurs.
             */
            virtual bool Send(int sock, const std::vector<uint8_t>& data) = 0;

            /**
             * @brief Synchronously receives all data from a connected socket until the connection is closed.
             *
             * This function performs a **blocking read operation**. It will not return until the
             * remote peer gracefully closes the connection, an error occurs, or the connection is
             * unexpectedly terminated. Data is received in blocks and processed immediately by the
             * provided callback.
             *
             * @note This is a **synchronous (blocking) utility**. It is **not suitable for use on a
             * main application thread** or in any scenario where responsiveness is critical. It should
             * only be called from a dedicated worker thread to avoid blocking the application's
             * event loop or UI.
             *
             * @param sock The integer handle of the socket to read from.
             * @param callBack A function object that will be called for each block of data received.
             * The callback signature is `void(std::span<uint8_t> data, size_t received_bytes, size_t total_received)`.
             * It provides a view into the received data, the size of the current block, and the
             * cumulative total of bytes received so far.
             * @return Returns true on a successful read operation (connection closed cleanly), and
             * false if an error occurred during the receive process.
             */
            virtual bool Receive(int sock, const std::function<void(std::span<uint8_t>, size_t, size_t)>& callBack) = 0;

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
