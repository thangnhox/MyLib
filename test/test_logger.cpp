#include "tnclib/services/logger.hpp"
#include <thread>
#include <chrono>

using tnclib::services::Logger;

int main() {
    Logger& log = Logger::getInstance();
    log.set_level(Logger::Level::Trace);

    log.set_text_color(Logger::Level::Critical, Logger::Color::Red);

    log.log(Logger::Level::Trace, "Trace message");
    log.log(Logger::Level::Debug, "Debug message");
    log.log(Logger::Level::Info, "Info message");
    log.log(Logger::Level::Warn, "Warning message");
    log.log(Logger::Level::Error, "Error message");
    log.log(Logger::Level::Critical, "Critical message");

    LOG_INFO("This is {}", 3);

    std::thread t1([&]{
        for (int i = 0; i < 5; ++i) {
            log.log(Logger::Level::Debug, "Thread 1 debug message");
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });

    std::thread t2([&]{
        for (int i = 0; i < 5; ++i) {
            log.log(Logger::Level::Warn, "Thread 2 warning message");
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });

    t1.join();
    t2.join();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}