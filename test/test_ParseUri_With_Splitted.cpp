#include <tnclib/utils/network.hpp>
#include <tnclib/utils/string.hpp>

#include <iostream>

int main() {
    std::string uri = "https://daotao1.stu.edu.vn/path/to/test/?help=2&testValue=3#footer";

    tnclib::utils::Network::Uri parsed = tnclib::utils::Network::ParseUri(uri);

    std::cout << "--- URI Components ---" << std::endl;
    std::cout << "Scheme:   " << parsed.scheme << std::endl;
    std::cout << "Host:     " << parsed.host << std::endl;
    std::cout << "Port:     " << parsed.port << std::endl;
    std::cout << "Path:     " << parsed.path << std::endl;
    std::cout << "Query:    " << std::endl;
    auto queries = tnclib::utils::string_utils::split(parsed.query, '&');
    for (const auto& i : queries) {
        std::cout << "      " << i << std::endl;
    }
    std::cout << "Fragment: " << parsed.fragment << std::endl;
    std::cout << "----------------------" << std::endl;
}

