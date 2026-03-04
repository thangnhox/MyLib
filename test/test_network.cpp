#include "tnclib/utils/network.hpp"
#include "tnclib/utils/init.hpp"

#include <string>
#include <iostream>
#include <vector>
#include <cstdint>
#include <cstdio>
#include <span>

int main() {
    using namespace tnclib::utils;

	init_default_implementation();

    // --- Test Data ---
    std::string host = "daotao3.stu.edu.vn";
    std::string path = "/tntt_20242/";
    std::string request =
        "GET " + path + " HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";
    // -----------------

    std::cout << "--- Starting Network Interface Call Test ---" << std::endl;

    // 1. Create Network Backend
    // This assumes a concrete "system" implementation has been registered elsewhere.
    auto network = Network::Create("system");
    if (!network) {
        std::printf("Failed to create network backend\n");
        return -1;
    }

    // 2. Initialize Backend
    if (!network->Init()) {
        std::printf("Failed to init backend\n");
        return -1;
    }

    // 3. Resolve Hostname
    auto addrs = network->ResolveDomain(host, "http", Network::ResolutionHint::Unspecified);
    if (addrs.empty()) {
        std::printf("DNS resolution failed for %s\n", host.c_str());
        return -1;
    }

    Network::InternetAddress addr = addrs.front();
    if (addr.port == 0) {
        addr.port = 80;
    }

    // 4. Create Socket
    int sock = network->CreateTCPSocket(addr.family);
    if (sock < 0) {
        std::printf("Failed to create socket\n");
        return -1;
    }

    // 5. Connect
    if (!network->Connect(sock, addr)) {
        network->Close(sock);
        std::printf("Failed to connect\n");
        return -1;
    }

    // 6. Send Request
    std::vector<uint8_t> request_data(request.begin(), request.end());
    if (!network->Send(sock, request_data)) {
        network->Close(sock);
        std::printf("Failed to send data\n");
        return -1;
    }

    // 7. Receive Response
    std::vector<uint8_t> response;
    size_t total_received = 0;

    bool ok = network->Receive(sock,
        [&response, &total_received](std::span<uint8_t> data, size_t received_bytes, size_t total) {
            // Append received bytes to response
            response.insert(response.end(), data.begin(), data.begin() + received_bytes);
            total_received = total;
            // Optionally, print progress here:
            // std::printf("... Received %zu bytes, Total: %zu\n", received_bytes, total);
        }
    );

    if (!ok || response.empty()) {
        network->Close(sock);
        std::printf("Failed to receive data\n");
        return -1;
    }

    // 8. Close Socket
    network->Close(sock);

    // 9. Output Received Data
    std::string myString(reinterpret_cast<const char*>(response.data()), response.size());
    std::cout << "\n--- Received Data ---" << std::endl;
    std::cout << myString << std::endl;
    std::cout << "---------------------" << std::endl;

    return 0;
}
