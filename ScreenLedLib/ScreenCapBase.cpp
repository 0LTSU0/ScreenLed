#include "ScreenCapBase.h"
#include <iostream>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

bool screenCaptureWorkerBase::loadConfigs(){
    std::cout << "screenCaptureWorkerBase::loadConfigs() using " << m_configPath << std::endl;

    auto confFile = std::ifstream(m_configPath);
    if (confFile.fail()) {
        if (!createConfigFile()){
            return false;
        }
    }

    confFile = std::ifstream(m_configPath);
    if (confFile.fail()) {
        return false; // should never be hit assuming the createConfigFile() works correctly
    }
    json conf;
    confFile >> conf;

    m_conf.c_debugSSInterval = conf["debugSSInterval"].get<int>();
    m_conf.c_keepDebugSSOnClipboard = conf["keepDebugSSOnClipboard"].get<bool>();
    m_conf.c_raspiIp = conf["raspiIp"].get<std::string>();
    m_conf.c_raspiPort = conf["raspiPort"].get<int>();
    m_conf.c_showDebugPreview = conf["showDebugPreview"].get<bool>();

    return true;
}

bool screenCaptureWorkerBase::createConfigFile(){
    json jconf;
    ScreenCapConfig defaultConfig;
    jconf["debugSSInterval"] = defaultConfig.c_debugSSInterval;
    jconf["keepDebugSSOnClipboard"] = defaultConfig.c_keepDebugSSOnClipboard;
    jconf["raspiIp"] = defaultConfig.c_raspiIp;
    jconf["raspiPort"] = defaultConfig.c_raspiPort;
    jconf["showDebugPreview"] = defaultConfig.c_showDebugPreview;

    std::ofstream file(m_configPath);
    if (!file) {
        std::cerr << "screenCaptureWorkerBase::createConfigFile() Failed to open config for writing." << std::endl;
        return false;
    }
    file << jconf.dump(4);
    file.close();
    return true;
}

ScreenCapConfig& screenCaptureWorkerBase::getCurrentConfig() {
    return m_conf;
}

void screenCaptureWorkerBase::updateCurrentConfig(ScreenCapConfig newConf) {
    m_conf = newConf;
    json jconf;
    jconf["debugSSInterval"] = m_conf.c_debugSSInterval;
    jconf["keepDebugSSOnClipboard"] = m_conf.c_keepDebugSSOnClipboard;
    jconf["raspiIp"] = m_conf.c_raspiIp;
    jconf["raspiPort"] = m_conf.c_raspiPort;
    jconf["showDebugPreview"] = m_conf.c_showDebugPreview;
    std::ofstream file(m_configPath);
    if (!file) {
        std::cerr << "screenCaptureWorkerBase::updateCurrentConfig() Failed to open config json for writing. The set values will be used during this session but changes won't be saved to disk" << std::endl;
        return;
    }
    file << jconf.dump(4);
    file.close();
}
