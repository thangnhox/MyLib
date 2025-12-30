#include "tnclib/services/logger.hpp"
#include "tnclib/utils/print_queue.hpp"
#include "platform/cross/system_utils.hpp"

#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <atomic>
#include <thread>
#include <iostream>

namespace tnclib {
    namespace services {

        using tnclib::utils::PrintQueue;

        static const char* level_name(Logger::Level lvl) {
            switch (lvl) {
                case Logger::Level::Trace: return "TRACE";
                case Logger::Level::Debug: return "DEBUG";
                case Logger::Level::Info: return "INFO";
                case Logger::Level::Warn: return "WARN";
                case Logger::Level::Error: return "ERROR";
                case Logger::Level::Critical: return "CRITICAL";
                case Logger::Level::Off: return "OFF";
            }
            return "?";
        }

        static std::string now_string() {
            using namespace std::chrono;
            auto tp = system_clock::now();
            std::time_t t = system_clock::to_time_t(tp);
            std::tm tm{};

            tnclib::platform::SystemUtils::localtime(t, tm);

            std::ostringstream oss;
            oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
            return oss.str();
        }

        static std::string thread_id_string() {
            std::ostringstream oss;
            oss << std::this_thread::get_id();
            return oss.str();
        }

        static std::string rgba_to_ansi(uint32_t rgba) {
            uint8_t r = (rgba >> 24) & 0xFF;
            uint8_t g = (rgba >> 16) & 0xFF;
            uint8_t b = (rgba >> 8) & 0xFF;

            std::ostringstream oss;
            oss << "\x1b[38;2;" << int(r) << ";" << int(g) << ";" << int(b) << "m";
            return oss.str();
        }

        struct Logger::Impl {
            std::atomic<Logger::Level> current_level{Logger::Level::Info};
            std::unordered_map<Logger::Level, uint32_t> header_colors;
            std::unordered_map<Logger::Level, uint32_t> text_colors;
            std::atomic<bool> use_colors{true};
        }; // logger implement


        Logger& Logger::getInstance() {
            static Logger instance;
            return instance;
        }

        Logger::Logger() : pImpl(new Impl) {
            if(!tnclib::platform::SystemUtils::is_tty(stdout)) {
                pImpl->use_colors.store(false);
            }
            set_default_colors();
        }

        void Logger::set_default_colors() {
            set_header_color(Level::Trace, Color::LightGray);
            set_text_color(Level::Trace, Color::White);

            set_header_color(Level::Debug, Color::Cyan);
            set_text_color(Level::Debug, Color::White);

            set_header_color(Level::Info, Color::Green);
            set_text_color(Level::Info, Color::White);

            set_header_color(Level::Warn, Color::Yellow);
            set_text_color(Level::Warn, Color::White);

            set_header_color(Level::Error, Color::Red);
            set_text_color(Level::Error, Color::White);

            set_header_color(Level::Critical, Color::Magenta);
            set_text_color(Level::Critical, Color::White);
        }

        Logger::~Logger() { delete pImpl; }

        void Logger::set_level(Level level) { pImpl->current_level.store(level, std::memory_order_relaxed); }
        Logger::Level Logger::level() const { return pImpl->current_level.load(std::memory_order_relaxed); }

        void Logger::set_header_color(Level lvl, uint32_t color) { pImpl->header_colors[lvl] = color; }
        void Logger::set_text_color(Level lvl, uint32_t color) { pImpl->text_colors[lvl] = color; }

        void Logger::set_header_color(Level lvl, Color color) { set_header_color(lvl, static_cast<uint32_t>(color)); }
        void Logger::set_text_color(Level lvl, Color color) { set_text_color(lvl, static_cast<uint32_t>(color)); }

        void Logger::enable_colors(bool enable) { pImpl->use_colors.store(enable); }

        void Logger::log(Level lvl, std::string_view msg) {
            if (lvl < pImpl->current_level.load(std::memory_order_relaxed)) return;

            std::ostringstream oss;
            oss << now_string() << " [T:" << thread_id_string() << "] ";

            auto hc = pImpl->header_colors.find(lvl);
            auto tc = pImpl->text_colors.find(lvl);

            if (pImpl->use_colors.load() && hc != pImpl->header_colors.end())
                oss << rgba_to_ansi(hc->second);

            oss << "[" << level_name(lvl) << "]";
            if (pImpl->use_colors.load()) oss << "\x1b[0m "; else oss << " ";

            if (pImpl->use_colors.load() && tc != pImpl->text_colors.end())
                oss << rgba_to_ansi(tc->second);

            oss << msg;

            if (pImpl->use_colors.load())
                oss << "\x1b[0m";

            PrintQueue::getInstance().enqueue(oss.str());
        }

    } // namespace services
} // namespace tnclib
