#pragma once

#include <string>
#include <memory>

namespace tnclib {
    namespace utils {
        class PrintQueue {
        public:
            static PrintQueue& getInstance();

            void enqueue(const std::string& text);

            void stop();

            PrintQueue(const PrintQueue&) = delete;
            PrintQueue& operator=(const PrintQueue&) = delete;
            PrintQueue(PrintQueue&&) = delete;
            PrintQueue& operator=(PrintQueue&&) = delete;

        private:
            PrintQueue();
            ~PrintQueue();

            struct Impl;
            std::unique_ptr<Impl> impl;
        }; // PrintQueue
    } // namespace utils
} // namespace tnclib

