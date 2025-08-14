#pragma once

#include "event_dispatcher.hpp"

#include <cstdint>

namespace tnclib {
    namespace utils {
        class EventListener {
        public:
            explicit EventListener(EventDispatcher& dispatcher);
            void listen_fd(int fd, uint32_t events, EventDispatcher::EventHandler handler);

        private:
            class Impl;
            Impl* pImpl;
        }; // event listener
    } // namespace utils
} //namespace tnclib

