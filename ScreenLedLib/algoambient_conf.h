#ifndef ALGOAMBIENT_CONF_H
#define ALGOAMBIENT_CONF_H

#include <tuple>
#include <json.hpp>
#include <algoconfig_base.h>

using json = nlohmann::json;

struct AlgoAmbient_Config {
    int c_sampleSeconds = 1;

    static auto fields();
    auto toJson();
    bool fromJson(const json&);
    const char* jsonConfKey = "algoFlashBoostConf";
};

inline auto AlgoAmbient_Config::fields() {
    return std::make_tuple(
        FIELD_RANGE(AlgoAmbient_Config, c_sampleSeconds, "Sampling seconds", 0, 255, 1)
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
        {"max", 255}
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
        if (key == "flashTreshold")
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
