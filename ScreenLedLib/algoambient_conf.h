#ifndef ALGOAMBIENT_CONF_H
#define ALGOAMBIENT_CONF_H

#include <tuple>
#include <json.hpp>
#include <algoconfig_base.h>

using json = nlohmann::json;

struct AlgoAmbient_Config {
    double c_sampleSeconds = 1;

    static auto fields();
    auto toJson();
    bool fromJson(const json&);
    const char* jsonConfKey = "algoAmbientConf";
};

inline auto AlgoAmbient_Config::fields() {
    return std::make_tuple(
        FIELD_RANGE(AlgoAmbient_Config, c_sampleSeconds, "Sampling seconds", 0, 60, 0.1)
    );
}

// Create JSON version of this config to use with e.g. remote config server
inline auto AlgoAmbient_Config::toJson()
{
    json samplingWindowSeconds = {
        {"name", "samplingWindowSeconds"},
        {"value", c_sampleSeconds},
        {"type", "float"},
        {"min", 0},
        {"max", 60}
    };
    json j = {samplingWindowSeconds};
    return j;
}

// Update values from nlohmann json
inline bool AlgoAmbient_Config::fromJson(const json& j)
{
    if (!j.is_object()) {
        return false; // unexpected format. TODO: do we need to further checking
    }
    for (const auto& [key, value] : j.items())
    {
        if (key == "samplingSeconds")
        {
            if (value >= 0 && value <= 255)
            {
                c_sampleSeconds = value;
            }
        }
    }
    return true;
}

#endif // ALGOAMBIENT_CONF_H
