#include <crow.h>
#include <json.hpp>

crow::json::wvalue to_crow(const nlohmann::json& j)
{
    if (j.is_null())
        return nullptr;

    if (j.is_boolean())
        return j.get<bool>();

    if (j.is_number_integer())
        return j.get<int64_t>();

    if (j.is_number_unsigned())
        return j.get<uint64_t>();

    if (j.is_number_float())
        return j.get<double>();

    if (j.is_string())
        return j.get<std::string>();

    if (j.is_array())
    {
        crow::json::wvalue::list result;
        result.reserve(j.size());

        for (const auto& item : j)
            result.push_back(to_crow(item));

        return result;
    }

    if (j.is_object())
    {
        crow::json::wvalue::object result;

        for (auto& [key, value] : j.items())
            result[key] = to_crow(value);

        return result;
    }

    return nullptr;
}

nlohmann::json to_nlohmann(const crow::json::rvalue& j)
{
    switch (j.t())
    {
    case crow::json::type::Null:
        return nullptr;

    case crow::json::type::False:
        return false;

    case crow::json::type::True:
        return true;

    case crow::json::type::Number:
        return j.d();

    case crow::json::type::String:
        return std::string(j.s());

    case crow::json::type::List:
    {
        nlohmann::json result = nlohmann::json::array();

        for (const auto& item : j)
            result.push_back(to_nlohmann(item));

        return result;
    }

    case crow::json::type::Object:
    {
        nlohmann::json result = nlohmann::json::object();

        for (const auto& item : j)
            result[item.key()] = to_nlohmann(item);

        return result;
    }
    }

    return nullptr;
}
