#include "platform/cross/system_utils.hpp"

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

namespace tnclib {
    namespace platform {

        void SystemUtils::localtime(std::time_t& timep, std::tm& result) {
        #if defined(_WIN32)
            localtime_s(&result, &timep);
        #else
            localtime_r(&timep, &result);
        #endif
        }

        bool SystemUtils::is_tty(FILE* stream) {
        #if defined(_WIN32)
            return _isatty(_fileno(stream));
        #else
            return isatty(fileno(stream));
        #endif
        }

    }
}
