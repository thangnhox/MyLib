#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>
#include <unordered_map>

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

            inline std::vector<std::string> split(const std::string& text, char delimiter) {
                std::vector<std::string> tokens;
                size_t start = 0;
                size_t end = text.find(delimiter);
                while (end != std::string::npos) {
                    tokens.push_back(text.substr(start, end - start));
                    start = end + 1;
                    end = text.find(delimiter, start);
                }
                tokens.push_back(text.substr(start));
                return tokens;
            }

            inline std::string trim(const std::string& text) {
                const auto strBegin = text.find_first_not_of(" \t\n\r\f\v");
                if (strBegin == std::string::npos)
                    return ""; // no content

                const auto strEnd = text.find_last_not_of(" \t\n\r\f\v");
                const auto strRange = strEnd - strBegin + 1;

                return text.substr(strBegin, strRange);
            }

            // Convert list of "key=value" strings to a map using template
            template<typename value_type>
            inline std::unordered_map<std::string, value_type> list_to_map(const std::vector<std::string>& list, char delimiter = '=') {
                std::unordered_map<std::string, value_type> result;
                for (const auto& item : list) {
                    size_t pos = item.find(delimiter);
                    if (pos != std::string::npos) {
                        std::string key = trim(item.substr(0, pos));
                        std::string value_str = trim(item.substr(pos + 1));
                        if constexpr (std::is_same_v<value_type, std::string>) {
                            result[key] = value_str;
                        } else if constexpr (std::is_integral_v<value_type>) {
                            try {
                                result[key] = static_cast<value_type>(std::stoll(value_str));
                            } catch (...) {
                                result[key] = 0; // or some default value
                            }
                        } else if constexpr (std::is_floating_point_v<value_type>) {
                            try {
                                result[key] = static_cast<value_type>(std::stod(value_str));
                            } catch (...) {
                                result[key] = 0.0; // or some default value
                            }
                        } else {
                            // Unsupported type, skip or handle error
                        }
                    }
                }
                return result;
            }

        } // namespace string_utils
    } // namespace utils
} // namespace tnclib
