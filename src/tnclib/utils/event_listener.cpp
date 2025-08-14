#include "tnclib/utils/event_listener.hpp"

namespace tnclib {
    namespace utils {
        class EventListener::Impl {
        public:
            EventDispatcher& dispatcher;
            Impl(EventDispatcher& disp) : dispatcher(disp) {}

            void listen_fd(int fd, uint32_t events, EventDispatcher::EventHandler handler) {
                dispatcher.add_fd(fd, events, handler);
            }
        }; //event listener implements

        EventListener::EventListener(EventDispatcher& dispatcher) : pImpl(new Impl(dispatcher)) {}
        void EventListener::listen_fd(int fd, uint32_t events, EventDispatcher::EventHandler handler) {
            pImpl->listen_fd(fd, events, handler);
        } 
    } // namespace utils
} //namespace tnclib

