#include "Http.h"
#include <stdexcept>
#include "widgets/httplib_config.h"

namespace core {

    namespace {
        // "https://host:port/path?query" -> {"https://host:port", "/path?query"}
        std::pair<std::string, std::string> SplitUrl(const std::string& url) {
            size_t protocol_end = url.find("://");
            if (protocol_end == std::string::npos) {
                throw std::invalid_argument("Invalid URL: " + url);
            }
            size_t path_start = url.find('/', protocol_end + 3);
            if (path_start == std::string::npos) {
                return { url, "/" };
            }
            return { url.substr(0, path_start), url.substr(path_start) };
        }
    }

    std::string HttpGet(const std::string& url, const std::map<std::string, std::string>& headers) {
        auto [host, path] = SplitUrl(url);
        httplib::Client client(host);
        client.set_connection_timeout(10);
        client.set_read_timeout(15);
        client.set_follow_location(true);

        httplib::Headers request_headers(headers.begin(), headers.end());
        auto result = client.Get(path, request_headers);
        if (!result) {
            throw std::runtime_error("Can't get data: " + httplib::to_string(result.error()));
        }
        if (result->status < 200 || result->status >= 300) {
            // Начало ответа обычно объясняет причину (например, лимит запросов у API)
            throw std::runtime_error("HTTP " + std::to_string(result->status) + ": " + result->body.substr(0, 200));
        }
        return result->body;
    }

    std::string UrlEncode(const std::string& text) {
        static const char* kHex = "0123456789ABCDEF";
        std::string out;
        for (unsigned char c : text) {
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                out += (char)c;
            }
            else {
                out += '%';
                out += kHex[c >> 4];
                out += kHex[c & 15];
            }
        }
        return out;
    }
}
