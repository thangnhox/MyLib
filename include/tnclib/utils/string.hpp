#pragma once

#include <algorithm>
#include <cctype>
#include <string>

namespace tnclib {
    namespace utils {
        namespace string_utils {

            inline std::string to_lower(const std::string& text) {
                std::string result = text;
                std::transform(result.begin(), result.end(), result.begin(),
                               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                return result;
            }

            inline std::string to_upper(const std::string& text) {
                std::string result = text;
                std::transform(result.begin(), result.end(), result.begin(),
                               [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
                return result;
            }

        } // namespace string_utils
    } // namespace utils
} // namespace tnclib
