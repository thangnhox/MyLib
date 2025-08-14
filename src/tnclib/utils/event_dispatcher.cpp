#include "tnclib/utils/event_dispatcher.hpp"

#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <unordered_map>

namespace tnclib {
    namespace utils {
        class EventDispatcher::Impl {
        public:
            int epoll_fd;
            static constexpr int MAX_EVENTS = 64;
            std::unordered_map<int, EventHandler> handlers;

            Impl() {
                epoll_fd = epoll_create1(0);
                if (epoll_fd == -1) {
                    perror("epoll_create1");
                    exit(1);
                }
            }

            ~Impl() {
                close(epoll_fd);
            }

            void add_fd(int fd, uint32_t events, EventHandler handler) {
                epoll_event ev{};
                ev.events = events;
                ev.data.fd = fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
                    perror("epoll_trl: add_fd");
                    exit(1);
                }
                handlers[fd] = handler;
            }

            void modify_fd(int fd, uint32_t events) {
                epoll_event ev{};
                ev.events = events;
                ev.data.fd = fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) == -1) {
                    perror("epoll_ctl: modify_fd");
                    exit(1);
                }
            }

            void remove_fd(int fd) {
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                handlers.erase(fd);
            }

            void run() {
                epoll_event events[MAX_EVENTS];
                while (true) {
                    int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
                    if (nfds == -1) {
                        perror("epoll_wait");
                        exit(-1);
                    }
                    for (int i = 0; i < nfds; ++i) {
                        int fd = events[i].data.fd;
                        if (handlers.count(fd)) {
                            handlers[fd]();
                        }
                    }
                }
            }

        }; // Event dispatcher implementations

        EventDispatcher::EventDispatcher() : pImpl(new Impl) {}
        EventDispatcher::~EventDispatcher() { delete pImpl; }
        void EventDispatcher::add_fd(int fd, uint32_t events, EventHandler handler) { pImpl->add_fd(fd, events, handler); }
        void EventDispatcher::modify_fd(int fd, uint32_t events) { pImpl->modify_fd(fd, events); }
        void EventDispatcher::remove_fd(int fd) {  pImpl->remove_fd(fd); }
        void EventDispatcher::run() { pImpl->run(); }

    } // namespace utils
} // namespace tnclib

