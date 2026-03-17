#include "tnclib/services/network.hpp"

namespace tnclib {
	namespace services {

		Network::Network(const std::string& backend)
			: net(tnclib::utils::Network::Create(backend)) {
			if (net) net->Init();
		}

		std::future<std::expected<std::vector<uint8_t>, std::string>>
        Network::request(const std::string& url, const std::vector<uint8_t>& payload, const std::function<void(size_t)>& progress) {
			return std::async(
				std::launch::async,
				[this, url, payload, progress] -> std::expected<std::vector<uint8_t>, std::string> 
				{
					// 1. Service Logic: Parse the URI using the utility
					auto uri = tnclib::utils::Network::ParseUri(url);
					
					// 2. Service Logic: Resolve domain to IP
					auto addresses = net->ResolveDomain(uri.host, uri.scheme);
					if (addresses.empty()) return std::unexpected("DNS Resolution Failed");
					// Assign custom port from uri
					if (uri.port) {
						for (auto& address : addresses) {
							address.port = uri.port;
						}
					}

					// 3. Service Logic: Perform the socket dance
					int sock = net->CreateTCPSocket();
					if (sock < 0) return std::unexpected("Socket Creation Failed");

					if (!net->Connect(sock, addresses[0])) {
						net->Close(sock);
						return std::unexpected("Connection Failed");
					}

					// 4. Send and Receive (using your utility functions)
					net->Send(sock, payload);
					
					std::vector<uint8_t> fullResponse;
					bool success = net->Receive(sock, [&](std::span<uint8_t> data, size_t, size_t total_received) {
						fullResponse.insert(fullResponse.end(), data.begin(), data.end());
						progress(total_received);
					});

					net->Close(sock);
					
					if (!success) return std::unexpected("Receive Error");
					return fullResponse;
				}
			);
		}

	}
}

