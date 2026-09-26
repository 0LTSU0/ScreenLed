#ifndef ALGOFLASHBOOST_CONF_H
#define ALGOFLASHBOOST_CONF_H

#include <tuple>
#include <algoconfig_base.h>

struct AlgoFlashBoost_Config {
    int c_flashTreshold = 70;
    int c_dimmingSpeed  = 10;

    static auto fields();
    const char* jsonConfKey = "algoFlashBoostConf";
};

inline auto AlgoFlashBoost_Config::fields() {
    return std::make_tuple(
        FIELD_RANGE(AlgoFlashBoost_Config, c_flashTreshold, "Flash Threshold", 0, 255, 1),
        FIELD_RANGE(AlgoFlashBoost_Config, c_dimmingSpeed,  "Dimming Speed",   0, 100, 1)
    );
}

#endif // ALGOFLASHBOOST_CONF_H
