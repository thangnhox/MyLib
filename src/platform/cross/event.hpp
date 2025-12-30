// platform_select.hpp
#pragma once

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    using socket_t = SOCKET;
#else
    #include <sys/select.h>
    #include <unistd.h>
    using socket_t = int;
#endif

#include "tnclib/utils/event.hpp"
#include <atomic>
#include <thread>

namespace tnclib::platform {

    class SelectEventDispatcher final : public tnclib::utils::EventDispatcher {
    public:
        SelectEventDispatcher();
        ~SelectEventDispatcher() override;

        SourceId add_source(std::unique_ptr<tnclib::utils::EventSource> src) override;
        void remove_source(SourceId id) override;

        void start() override;
        void stop() override;

    private:
        void run_loop();

    private:
        std::unordered_map<SourceId, std::unique_ptr<tnclib::utils::EventSource>> sources_;
        std::unordered_map<socket_t, SourceId> fd_map_;

        std::atomic<bool> running_{false};
        std::thread worker_;
        std::mutex mutex_;
        SourceId next_id_{1};
    };

} // namespace tnclib::platform
