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
                IPv4 = 4,
                IPv6 = 6
            };

            enum class ConnectionType : int {
                SEQ,    // SOCK_SEQPACKAGE - SCTP
                TCP,    // SOCK_STREAM - TCP
                UDP     // SOCK_DGRAM - UDP
            };

            enum class ResolutionHint : int {
                Unspecified,
                IPv4,
                IPv6
            };

            struct InternetAddress {
                std::string ip;
                AddressFamily family;
                int port;
            };

            struct Uri {
                std::string scheme;
                std::string host;
                std::int32_t port = -1; // -1 indicates no port specified
                std::string path;
                std::string query;
                std::string fragment;
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
             * @return A std::vector of InternetAddress objects, each representing a resolved
             * network address. The vector will be empty if the resolution fails.
             */
            virtual std::vector<InternetAddress> ResolveDomain(const std::string& hostname, const std::string& service, ResolutionHint hint = ResolutionHint::Unspecified) = 0;

            /**
             * @brief Creates a new socket of the specified type and address family.
             * 
             * This pure virtual function provides an abstract interface for creating a network socket.
             * It is implemented by a concrete derived class to handle the platform-specific
             * socket creation. **A default implementation is provided.**
             * 
             * @param type The type of socket to create (e.g., ConnectionType::TCP or ConnectionType::UDP).
             * @param family The address family for the socket (e.g., AddressFamily::IPv4 or AddressFamily::IPv6).
             * Defaults to AddressFamily::IPv4.
             * @return An integer representing the new socket handle.
             * Returns -1 or a platform-specific error code on failure.
             */
            virtual int CreateSocket(ConnectionType type, AddressFamily family = AddressFamily::IPv4) = 0;

            /**
             * @brief Creates a new TCP (Transmission Control Protocol) socket.
             *
             * This is a specialized pure virtual function for creating a reliable,
             * connection-oriented TCP socket. Use this for communication that requires
             * guaranteed delivery, such as file transfers or secure connections.
             * **A default implementation is provided.**
             *
             * @param family The address family for the socket (e.g., AddressFamily::IPv4 or AddressFamily::IPv6).
             * Defaults to AddressFamily::IPv4.
             * @return An integer representing the new socket handle.
             * Returns -1 or a platform-specific error code on failure.
             */
            virtual int CreateTCPSocket(AddressFamily family = AddressFamily::IPv4) = 0;

            /**
             * @brief Creates a new UDP (User Datagram Protocol) socket.
             *
             * This is a specialized pure virtual function for creating a fast,
             * connectionless UDP socket. It is ideal for applications where
             * speed is prioritized over guaranteed delivery, such as real-time
             * video streaming or online gaming. **A default implementation is provided.**
             *
             * @param family The address family for the socket (e.g., AddressFamily::IPv4 or AddressFamily::IPv6).
             * Defaults to AddressFamily::IPv4.
             * @return An integer representing the new socket handle.
             * Returns -1 or a platform-specific error code on failure.
             */
            virtual int CreateUDPSocket(AddressFamily family = AddressFamily::IPv4) = 0;

            /**
             * @brief Binds a socket to a local address.
             *
             * This pure virtual function provides an abstract interface for binding a socket
             * to a specific local address and port. It is implemented by a concrete derived
             * class to perform the actual binding operation. **A default implementation is provided.**
             *
             * @param sock The integer handle or descriptor of the socket to bind.
             * @param address The InternetAddress object representing the local endpoint's IP address
             * and port to bind the socket to.
             * @return Returns true on a successful bind operation, and false if the bind fails.
             */
            virtual bool Bind(int sock, const InternetAddress& address) = 0;

            /**
             * @brief Establishes a connection to a remote address on a given socket.
             *
             * This pure virtual function provides an abstract interface for establishing a network connection.
             * It is implemented by a concrete derived class to perform the actual connection operation.
             * **A default implementation is provided.**
             *
             * @param sock The integer handle or descriptor of the socket to connect. This socket must be a
             * valid, previously created socket, typically of a connection-oriented type like TCP.
             * @param address The InternetAddress object containing the remote endpoint's IP address and port
             * to which the socket should connect.
             * @return Returns true on a successful connection, and false if the connection fails.
             */
            virtual bool Connect(int sock, const InternetAddress& address) = 0;

            /**
             * @brief Mark socket as passive to accept incoming connection requests.
             *
             * This pure virtual function provides an abstract interface for listening for
             * incoming connections on a socket. It is implemented by a concrete derived
             * class to perform the actual listening operation. **A default implementation is provided.**
             *
             * @param sock The integer handle or descriptor of the socket to listen on.
             * @param backlog The maximum length of the queue of pending connections.
             * @return Returns true on a successful listen operation, and false if the listen fails.
             */
            virtual bool Listen(int sock, int backlog) = 0;

            /**
             * @brief Accepts an incoming connection on a listening socket.
             *
             * This pure virtual function provides an abstract interface for accepting
             * incoming connection requests. It is implemented by a concrete derived
             * class to handle the actual accept operation. **A default implementation is provided.**
             *
             * @param sock The integer handle or descriptor of the listening socket.
             * @return On success, returns a new socket descriptor for the accepted connection.
             * On failure, returns -1 or a platform-specific error code.
             */
            virtual int Accept(int sock) = 0;

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

            /// Parse a URI string into its components.
            static Uri ParseUri(const std::string& uri);

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
