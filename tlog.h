#include <limits>

#include <nlohmann/json.hpp>

#include "Ensure.h"

namespace tlog {

const char *const ADDR = "addr";
const char *const SLAVE = "slave";
const char *const COUNT = "count";
const char *const VALUE = "value";

template <typename T, typename V>
bool inRange(V value)
{
    return
        std::numeric_limits<T>::min() <= value
        && std::numeric_limits<T>::max() >= value;
}

using json = nlohmann::json;

std::string toStr(const json &input)
{
    if(!input.count(ADDR)) return {};

    ENSURE(input[ADDR].is_number(), RuntimeError);

    const auto addr = input[ADDR].get<int>();

    ENSURE(inRange<uint16_t>(addr), RuntimeError);

    if(!input.count(VALUE)) return {};

    ENSURE(input[VALUE].is_array(), RuntimeError);

    const auto data = input[VALUE].get<std::vector<int>>();

    if(input.count(SLAVE))
    {
        ENSURE(input[SLAVE].is_number(), RuntimeError);
    }

    std::string output;

    output.reserve(data.size());

    for(auto i : data) output.push_back(char(i));
    return output;
}

} /* tlog */
