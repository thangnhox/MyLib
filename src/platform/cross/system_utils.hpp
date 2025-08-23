#pragma once

#include <ctime>
#include <stdio.h>

namespace tnclib {
    namespace platform {

        class SystemUtils {
        public:
            static void localtime(std::time_t& timep, std::tm& result);
            static bool is_tty(FILE* stream);
        }; // system utils

    } // namespace platform
} //namespace tnclib
