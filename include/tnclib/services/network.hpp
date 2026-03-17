#pragma once
#include <future>
#include <thread>
#include <string>
#include <functional>
#include <vector>
#include <cstdint>
#include <expected>

#include "tnclib/utils/network.hpp"

namespace tnclib {
    namespace services {

        class Network {
        public:
            explicit Network(const std::string& backend = "system");

            std::future<std::expected<std::vector<uint8_t>, std::string>>
            request(const std::string& url, const std::vector<uint8_t>& payload, const std::function<void(size_t)>& progress);

        private:
            std::shared_ptr<tnclib::utils::Network> net;
        };

    }
}
