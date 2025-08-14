#pragma once

#include <functional>
#include <cstdint>

namespace tnclib {
    namespace utils {
        class EventDispatcher {
        public:
            using EventHandler = std::function<void()>;

            EventDispatcher();
            ~EventDispatcher();

            // note: fd means file descriptor
            void add_fd(int fd, uint32_t events, EventHandler handler);
            void modify_fd(int fd, uint32_t events);
            void remove_fd(int fd);

            void run();

        private:
            class Impl;
            Impl* pImpl;
        }; // event dispatcher
    } // namespace utils
} // namespace tnclib

