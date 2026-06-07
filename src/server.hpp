#include <XblazeAPI.hpp>

#include <Geode/Geode.hpp>
#include <asp/iter/Split.hpp>

#include <string_view>
#include <unordered_map>

using namespace geode::prelude;

namespace xblazeapi {
    arc::Future<ServerResponse> requestGDServers(
        std::string_view endpoint,
        std::string_view body,
        int timeout
    ) {
        auto req = web::WebRequest()
            .userAgent("")
            .bodyString(body)
            .timeout(std::chrono::seconds(timeout));

        auto res = co_await req.post(fmt::format("{}{}", BOOMLINGS, endpoint));
        if (!res.ok()) {
            log::error("Failed to request endpoint '{}' ({}): {}", endpoint, res.code(), res.errorMessage());
            co_return Err(res.code());
        }

        auto ret = res.string();

        if (ret.isErr()) {
            log::error("Could not get response from endpoint '{}': {}", endpoint, ret.unwrapErr());
            co_return Err(0);
        }
        auto unwrapped = ret.unwrap();
        auto num = utils::numFromString<int>(unwrapped);
        if (num.isOk() && num.unwrap() < 0) {
            co_return Err(num.unwrap());
        }

        co_return Ok(unwrapped);
    }

    std::unordered_map<std::string, std::string> formatResponse(std::string_view response, std::string_view sep) {
        std::unordered_map<std::string, std::string> map;
        map.reserve(std::count(response.begin(), response.end(), sep));

        size_t lastPos = 0;
        while (lastPos != response.npos) {
            const size_t pos = response.find_first_of(sep, lastPos); // "1:23:4:5" will return 2
            const size_t next = response.find_first_of(sep, pos + 1); // Will return 5
            map.emplace(response.substr(lastPos, pos), response.substr(pos, next));
            lastPos = next + 1;
        }

        return map;
    }
}