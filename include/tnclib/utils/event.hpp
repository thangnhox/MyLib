// event_dispatcher.hpp
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include <mutex>

namespace tnclib::utils {

    enum class EventType {
        Readable,
        Writable,
        Error,
        Hangup,
        Timer,
        Signal,
        Custom
    };

    struct Event {
        EventType type;
        std::uint64_t source_id;
        uint32_t raw_events;
    };

    class EventSource {
    public:
        virtual ~EventSource() = default;
        virtual int fd() const = 0;
        virtual uint32_t interests() const = 0;
        virtual void on_event(const Event&) = 0;
    };

    class EventDispatcher {
    public:
        using SourceId = std::uint64_t;
        
        virtual ~EventDispatcher() = default;

        virtual SourceId add_source(std::unique_ptr<EventSource> src) = 0;
        virtual void remove_source(SourceId id) = 0;

        virtual void start() = 0; // background thread
        virtual void stop() = 0;  // safe from any thread

    public:
        /// Get the active EventDispatcher instance (nullptr if not created).
        static std::shared_ptr<EventDispatcher> Instance();

        /// Create or return an existing EventDispatcher backend (thread-safe).
        static std::shared_ptr<EventDispatcher> Create(const std::string& backend = "system");

        /// Register a custom backend factory by name.
        static void RegisterBackend(const std::string& name,
                                    std::function<std::shared_ptr<EventDispatcher>()> factory);

    protected:
        EventDispatcher() = default;

    private:
        EventDispatcher(const EventDispatcher&) = delete;
        EventDispatcher& operator=(const EventDispatcher&) = delete;
        EventDispatcher(EventDispatcher&&) = delete;
        EventDispatcher& operator=(EventDispatcher&&) = delete;

    private:
        static std::shared_ptr<EventDispatcher> instance;
        static std::string active_backend;
        static std::unordered_map<std::string, std::function<std::shared_ptr<EventDispatcher>()>> backends;
        static std::mutex mutex;
    };

} // namespace tnclib::utils
