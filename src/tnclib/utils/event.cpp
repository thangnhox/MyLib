#include "tnclib/utils/event.hpp"
#include "tnclib/utils/string.hpp"
#include "tnclib/services/logger.hpp"

namespace tnclib::utils {

    std::shared_ptr<EventDispatcher> EventDispatcher::instance = nullptr;
    std::string EventDispatcher::active_backend = "";
    std::unordered_map<std::string, std::function<std::shared_ptr<EventDispatcher>()>> EventDispatcher::backends = {};
    std::mutex EventDispatcher::mutex;

    std::shared_ptr<EventDispatcher> EventDispatcher::Instance() {
        std::lock_guard<std::mutex> lock(mutex);
        return instance;
    }

    std::shared_ptr<EventDispatcher> EventDispatcher::Create(const std::string& backend) {
        std::lock_guard<std::mutex> lock(mutex);

        std::string lname = tnclib::utils::string_utils::to_lower(backend);

        if (instance) {
            if (active_backend != lname) {
                LOG_WARN("EventDispatcher Utils: {} is already created, cannot init {}", active_backend, lname);
            }

            return instance;
        }

        auto it = backends.find(lname);
        if (it != backends.end()) {
            LOG_INFO("EventDispatcher Utils: Selected back end {}", lname);
            instance = it->second();
        } else {
            // fallback to system backend if unknown
            LOG_WARN("EventDispatcher Utils: {} not found, fall back to default \"system\"", lname);
            auto sys = backends.find("system");
            if (sys != backends.end()) {
                instance = sys->second();
            }
        }

        return instance;
    }

    void EventDispatcher::RegisterBackend(const std::string& name,
                                    std::function<std::shared_ptr<EventDispatcher>()> factory) {
        if (!factory) {
            throw std::invalid_argument("EventDispatcher backend factory is null");
        }

        std::lock_guard<std::mutex> lock(mutex);

        std::string lname = tnclib::utils::string_utils::to_lower(name);

        auto [it, inserted] = backends.emplace(lname, std::move(factory));
        if (!inserted) {
            throw std::logic_error(
                "EventDispatcher backend already registered: " + lname
            );
        }
    }

} // namespace tnclib::utils
