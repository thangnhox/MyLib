#include "tnclib/utils/network.hpp"
#include "tnclib/utils/string.hpp"
#include "tnclib/services/logger.hpp"
#include "platform/cross/network.hpp"

namespace tnclib {
    namespace utils {

        std::shared_ptr<Network> Network::instance = nullptr;
        std::string Network::active_backend = "";
        std::unordered_map<std::string, std::function<std::shared_ptr<Network>()>> Network::backends = {
            {"system", []() { return std::make_shared<platform::CrossNetwork>(); }}
        };
        std::mutex Network::mutex;

        std::shared_ptr<Network> Network::Instance() {
            std::lock_guard<std::mutex> lock(mutex);
            return instance;
        }

        std::shared_ptr<Network> Network::Create(const std::string& backend) {
            std::lock_guard<std::mutex> lock(mutex);

            std::string lname = tnclib::utils::string_utils::to_lower(backend);

            if (instance) {
                if (active_backend != lname) {
                    LOG_WARN("Network Utils: {} is already created, cannot init {}", active_backend, lname);
                }

                return instance;
            }

            auto it = backends.find(lname);
            if (it != backends.end()) {
                LOG_INFO("Network Utils: Selected back end {}", lname);
                instance = it->second();
            } else {
                // fallback to system backend if unknown
                LOG_WARN("Network Utils: {} not found, fall back to default \"system\"", lname);
                auto sys = backends.find("system");
                if (sys != backends.end()) {
                    instance = sys->second();
                }
            }

            return instance;
        }

        void Network::RegisterBackend(const std::string& name,
                                      std::function<std::shared_ptr<Network>()> factory) {
            std::lock_guard<std::mutex> lock(mutex);

            std::string lname = tnclib::utils::string_utils::to_lower(name);

            if (lname == "system") {
                LOG_ERROR("Network Utils: \"system\" backend is reserved and cannot be overwritten");
                return;
            }

            backends[lname] = std::move(factory);
        }

    } // namespace utils
} // namespace tnclib
