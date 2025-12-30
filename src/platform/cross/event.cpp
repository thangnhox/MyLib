#include "event.hpp"

namespace tnclib::platform {

    SelectEventDispatcher::SelectEventDispatcher() = default;

    SelectEventDispatcher::~SelectEventDispatcher() {
        stop();
    }

    tnclib::utils::EventDispatcher::SourceId
    SelectEventDispatcher::add_source(std::unique_ptr<tnclib::utils::EventSource> src) {
        std::lock_guard<std::mutex> lock(mutex_);

        SourceId id = next_id_++;
        socket_t fd = static_cast<socket_t>(src->fd());

        fd_map_[fd] = id;
        sources_[id] = std::move(src);

        return id;
    }

    void SelectEventDispatcher::remove_source(SourceId id) {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = sources_.find(id);
        if (it == sources_.end())
            return;

        socket_t fd = static_cast<socket_t>(it->second->fd());
        fd_map_.erase(fd);
        sources_.erase(it);
    }

    void SelectEventDispatcher::start() {
        if (running_.exchange(true))
            return;

        worker_ = std::thread([this] {
            run_loop();
        });
    }

    void SelectEventDispatcher::stop() {
        if (!running_.exchange(false))
            return;

        if (worker_.joinable()) {
            worker_.join();
        }
    }

    void SelectEventDispatcher::run_loop() {
        while (running_) {
            fd_set readfds;
            FD_ZERO(&readfds);

            socket_t maxfd = 0;

            {
                std::lock_guard<std::mutex> lock(mutex_);
                for (const auto& [fd, id] : fd_map_) {
                    FD_SET(fd, &readfds);
                    if (fd > maxfd) {
                        maxfd = fd;
                    }
                }
            }

            timeval timeout{};
            timeout.tv_sec = 1;   // wake periodically for stop()
            timeout.tv_usec = 0;

    #ifdef _WIN32
            int ret = select(0, &readfds, nullptr, nullptr, &timeout);
    #else
            int ret = select(maxfd + 1, &readfds, nullptr, nullptr, &timeout);
    #endif

            if (ret <= 0) {
                continue;
            }

            std::lock_guard<std::mutex> lock(mutex_);

            for (const auto& [fd, id] : fd_map_) {
                if (FD_ISSET(fd, &readfds)) {
                    auto& src = sources_[id];

                    tnclib::utils::Event ev{
                        tnclib::utils::EventType::Readable,
                        id,
                        0
                    };

                    src->on_event(ev);
                }
            }
        }
    }

} // namespace tnclib::platform
