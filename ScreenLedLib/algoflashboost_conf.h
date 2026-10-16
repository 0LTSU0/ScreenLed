#ifndef ALGOFLASHBOOST_CONF_H
#define ALGOFLASHBOOST_CONF_H

#include <tuple>
#include <json.hpp>
#include <algoconfig_base.h>

using json = nlohmann::json;

struct AlgoFlashBoost_Config {
    int c_flashTreshold = 70;
    int c_dimmingSpeed  = 10;

    static auto fields();
    auto toJson();
    bool fromJson(const json&);
    const char* jsonConfKey = "algoFlashBoostConf";
};

inline auto AlgoFlashBoost_Config::fields() {
    return std::make_tuple(
        FIELD_RANGE(AlgoFlashBoost_Config, c_flashTreshold, "Flash Threshold", 0, 255, 1),
        FIELD_RANGE(AlgoFlashBoost_Config, c_dimmingSpeed,  "Dimming Speed",   0, 100, 1)
    );
}

// Create JSON version of this config to use with e.g. remote config server
inline auto AlgoFlashBoost_Config::toJson()
{
    json flashTreshold = {
        {"name", "flashTreshold"},
        {"value", c_flashTreshold},
        {"type", "int"},
        {"min", 0},
        {"max", 255}
    };
    json dimmingSpeed = {
        {"name", "dimmingSpeed"},
        {"value", c_dimmingSpeed},
        {"type", "int"},
        {"min", 0},
        {"max", 100}
    };
    json j = {flashTreshold, dimmingSpeed};
    return j;
}

// Update values from nlohmann json
inline bool AlgoFlashBoost_Config::fromJson(const json& j)
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
                c_flashTreshold = value;
            }
        }
        else if (key == "dimmingSpeed")
        {
            if (value >= 0 && value <= 100)
            {
                c_dimmingSpeed = value;
            }
        }
    }
    return true;
}

#endif // ALGOFLASHBOOST_CONF_H
