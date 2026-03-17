#include "tnclib/utils/network.hpp"
#include "tnclib/utils/string.hpp"
#include "tnclib/services/logger.hpp"

namespace tnclib {
    namespace utils {

        std::shared_ptr<Network> Network::instance = nullptr;
        std::string Network::active_backend = "";
        std::unordered_map<std::string, std::function<std::shared_ptr<Network>()>> Network::backends = {};
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

        Network::Uri Network::ParseUri(const std::string& uri, const std::string& defaultScheme) {
            Uri result;

            // Find the scheme
            size_t scheme_end = uri.find("://");
            if (scheme_end != std::string::npos) {
                result.scheme = uri.substr(0, scheme_end);
            } else {
                result.scheme = defaultScheme;
                scheme_end = 0; // No scheme found, start from beginning
            }

            // Find the host
            // Host can be domain name, IPv4, or [IPv6]
            size_t host_start = scheme_end + (scheme_end > 0 ? 3 : 0);
            size_t path_start = uri.find('/', host_start);
            size_t host_end = host_start;
            // Find [ and ] for IPv6
            if (uri[host_start] == '[') {
                size_t ipv6_end = uri.find(']', host_start);
                if (ipv6_end != std::string::npos) {
                    result.host = uri.substr(host_start + 1, ipv6_end - host_start - 1);
                    host_end = ipv6_end + 1; // Move past ']'
                } else {
                    // Malformed IPv6, treat as normal host
                    host_end = (path_start != std::string::npos) ? path_start : uri.length();
                    result.host = uri.substr(host_start, host_end - host_start);
                }
                // Check for port after IPv6
                if (host_end < uri.length() && uri[host_end] == ':') {
                    size_t port_start = host_end + 1;
                    size_t port_end = (path_start != std::string::npos) ? path_start : uri.length();
                    if (port_start < port_end) {
                        try {
                            result.port = std::stoi(uri.substr(port_start, port_end - port_start));
                        } catch (...) {
                            result.port = 0; // Invalid port
                        }
                    }
                    host_end = port_end;
                }
            } else {
                host_end = (path_start != std::string::npos) ? path_start : uri.length();
                size_t port_pos = uri.find(':', host_start);
                if (port_pos != std::string::npos && port_pos < host_end) {
                    result.host = uri.substr(host_start, port_pos - host_start);
                    try {
                        result.port = std::stoi(uri.substr(port_pos + 1, host_end - port_pos - 1));
                    } catch (...) {
                        result.port = 0; // Invalid port
                    }
                } else {
                    result.host = uri.substr(host_start, host_end - host_start);
                }
            }

            // Find the path, query, and fragment
            if (path_start != std::string::npos) {
                size_t query_start = uri.find('?', path_start);
                size_t fragment_start = uri.find('#', path_start);

                if (query_start != std::string::npos && (fragment_start == std::string::npos || query_start < fragment_start)) {
                    result.path = uri.substr(path_start, query_start - path_start);
                    if (fragment_start != std::string::npos) {
                        result.query = uri.substr(query_start + 1, fragment_start - query_start - 1);
                        result.fragment = uri.substr(fragment_start + 1);
                    } else {
                        result.query = uri.substr(query_start + 1);
                    }
                } else if (fragment_start != std::string::npos) {
                    result.path = uri.substr(path_start, fragment_start - path_start);
                    result.fragment = uri.substr(fragment_start + 1);
                } else {
                    result.path = uri.substr(path_start);
                }
            } else {
                result.path = "/";
            }

            return result;
        }

    } // namespace utils
} // namespace tnclib
