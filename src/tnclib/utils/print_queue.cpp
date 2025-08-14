#include "tnclib/utils/print_queue.hpp"

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace tnclib {
    namespace utils {
        struct PrintQueue::Impl {
            std::queue<std::string> messages;
            std::mutex mtx;
            std::condition_variable cv;
            std::atomic<bool> running{true};
            std::thread worker;

            Impl() {
                worker = std::thread(&Impl::processQueue, this);
            }

            ~Impl() {
                stop();
            }

            void processQueue() {
                while (running) {
                    std::unique_lock<std::mutex> lock(mtx);
                    cv.wait(lock, [this] { return !messages.empty() || !running; });

                    if (!running && messages.empty()) {
                        break;
                    }

                    std::string msg = std::move(messages.front());
                    messages.pop();
                    lock.unlock();

                    std::cout << msg << std::endl;
                }
            }

            void enqueue(const std::string& text) {
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    messages.push(text);
                }
                cv.notify_one();
            }

            void stop() {
                running = false;
                cv.notify_all();
                if (worker.joinable())
                    worker.join();
            }
        }; // PrintQueue:Impl

        PrintQueue& PrintQueue::getInstance() {
            static PrintQueue instance;
            return instance;
        }

        PrintQueue::PrintQueue()
            : impl(std::make_unique<Impl>()) {}

        PrintQueue::~PrintQueue() {
            stop();
        }

        void PrintQueue::enqueue(const std::string& text) {
            impl->enqueue(text);
        }

        void PrintQueue::stop() {
            if (impl)
                impl->stop();
        }


    } // namespace utils
} // namespace tnclib

