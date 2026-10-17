#include "configurator.h"
#include <iostream>
#include <json.hpp>
#include <fstream>
#include <QFileInfo>
#include <QDir>
#include "Log.h"

using json = nlohmann::json;

ScreenLedConfigurator::ScreenLedConfigurator() {
    loadConfigs();
}


bool ScreenLedConfigurator::loadConfigs() {
    std::cout << "screenCaptureWorkerBase::loadConfigs() using " << m_configPath << std::endl;

    auto confFile = std::ifstream(m_configPath);
    if (confFile.fail()) {
        if (!createConfigFile()){
            return false;
        }
    }

    std::lock_guard<std::mutex> lock(m_conf_mutex);
    confFile = std::ifstream(m_configPath);
    if (confFile.fail()) {
        return false; // should never be hit assuming the createConfigFile() works correctly
    }
    json conf;
    confFile >> conf;

    m_conf.c_debugSSInterval = conf["debugSSInterval"].get<int>();
    m_conf.c_keepDebugSSOnClipboard = conf["keepDebugSSOnClipboard"].get<bool>();
    m_conf.c_showDebugPreview = conf["showDebugPreview"].get<bool>();
    m_conf.c_screenResX = conf["screenResX"].get<int>();
    m_conf.c_screenResY = conf["screenResY"].get<int>();
    m_conf.c_algo = conf["algo"].get<ScreenLedAlgorithm>();
    m_conf.c_autorunScriptPath = conf.contains("autorunScriptPath") ? QString::fromStdString(conf["autorunScriptPath"].get<std::string>()) : "";
    m_conf.c_preferredLocalNetworkInterface = conf.contains("preferredLocalNetworkInterface") ? conf["preferredLocalNetworkInterface"].get<std::string>() : "";
    m_conf.c_analyzerScreenArea = conf.contains("screenAnalysisArea") ? conf["screenAnalysisArea"].get<activeScreenArea>() : activeScreenArea::FULL;
    m_conf.c_analyzerDownscaleFactor = conf.contains("screenAnalysisDownscaleFactor") ? conf["screenAnalysisDownscaleFactor"].get<int>() : 1;

    m_conf.c_clientInfos.clear();
    for (const auto& item : conf["clients"]) {
        clientInfo c;
        c.host = item["addr"];
        c.port = item["port"];
        c.type = item["type"];
        c.ledStripArg = item["ledStripArg"];
        m_conf.c_clientInfos.push_back(c);
    }

    // config server config
    if (conf.contains("configServerConf")) {
        m_conf.c_configServerConf.enabled = conf["configServerConf"].contains("enabled") ?
                                            conf["configServerConf"]["enabled"].get<bool>() :
                                            m_conf.c_configServerConf.enabled; // default value from initialized conf
        m_conf.c_configServerConf.authToken = conf["configServerConf"].contains("authToken") ?
                                              conf["configServerConf"]["authToken"].get<std::string>() :
                                              m_conf.c_configServerConf.authToken; // default value from initialized conf
    }

    //algo confs
    const auto algoFlashBoostConfigJsonKey = m_conf.c_algoFlashBoostConfig.jsonConfKey;
    if (conf.contains(algoFlashBoostConfigJsonKey))
    {
        m_conf.c_algoFlashBoostConfig.fromJson(conf[algoFlashBoostConfigJsonKey]);
    }
    const auto algoAmbientConfigJsonKey = m_conf.c_algoAmbientConfig.jsonConfKey;
    if (conf.contains(algoAmbientConfigJsonKey))
    {
        m_conf.c_algoAmbientConfig.fromJson(conf[algoAmbientConfigJsonKey]);
    }

    return true;
}

bool ScreenLedConfigurator::createConfigFile(){
    json jconf;
    ScreenCapConfig defaultConfig;
    jconf["debugSSInterval"] = defaultConfig.c_debugSSInterval;
    jconf["keepDebugSSOnClipboard"] = defaultConfig.c_keepDebugSSOnClipboard;
    jconf["showDebugPreview"] = defaultConfig.c_showDebugPreview;
    jconf["screenResX"] = defaultConfig.c_screenResX;
    jconf["screenResY"] = defaultConfig.c_screenResY;
    jconf["algo"] = defaultConfig.c_algo;
    jconf["autorunScriptPath"] = defaultConfig.c_autorunScriptPath.toStdString();
    jconf["preferredLocalNetworkInterface"] = defaultConfig.c_preferredLocalNetworkInterface;
    jconf["screenAnalysisArea"] = defaultConfig.c_analyzerScreenArea;
    jconf["screenAnalysisDownscaleFactor"] = defaultConfig.c_analyzerDownscaleFactor;

    for (const auto& c : defaultConfig.c_clientInfos) {
        jconf["clients"].push_back({
            {"addr", c.host},
            {"port", c.port},
            {"type", c.type},
            {"ledStripArg", c.ledStripArg}
        });
    }

    const QFileInfo info(QString::fromStdString(m_configPath));
    const QString directory = info.absolutePath();

    if (!QDir().mkpath(directory)) {
        LIB_ERROR() << "createConfigFile() failed to create dir " << directory;
        return false;
    }

    std::ofstream file(m_configPath);
    if (!file) {
        LIB_ERROR() << "createConfigFile() failed to open config file for writing " << info;
        return false;
    }
    LIB_INFO() << "Creating config file to " << info;
    file << jconf.dump(4);
    file.close();
    return true;
}

void ScreenLedConfigurator::updateCurrentConfig(ScreenCapConfig newConf, bool save) {
    std::lock_guard<std::mutex> lock(m_conf_mutex);
    m_conf = newConf;

    if (!save)
    {
        // change is only applied to m_conf
        return;
    }

    json jconf;
    jconf["debugSSInterval"] = m_conf.c_debugSSInterval;
    jconf["keepDebugSSOnClipboard"] = m_conf.c_keepDebugSSOnClipboard;
    jconf["showDebugPreview"] = m_conf.c_showDebugPreview;
    jconf["screenResX"] = m_conf.c_screenResX;
    jconf["screenResY"] = m_conf.c_screenResY;
    jconf["algo"] = m_conf.c_algo;
    jconf["autorunScriptPath"] = m_conf.c_autorunScriptPath.toStdString();
    jconf["preferredLocalNetworkInterface"] = m_conf.c_preferredLocalNetworkInterface;
    jconf["screenAnalysisArea"] = m_conf.c_analyzerScreenArea;
    jconf["screenAnalysisDownscaleFactor"] = m_conf.c_analyzerDownscaleFactor;
    for (const auto& c : m_conf.c_clientInfos) {
        jconf["clients"].push_back({
            {"addr", c.host},
            {"port", c.port},
            {"type", c.type},
            {"ledStripArg", c.ledStripArg}
        });
    }

    // algo confs
    jconf[m_conf.c_algoFlashBoostConfig.jsonConfKey]["flashTreshold"] = m_conf.c_algoFlashBoostConfig.c_flashTreshold;
    jconf[m_conf.c_algoFlashBoostConfig.jsonConfKey]["dimmingSpeed"] = m_conf.c_algoFlashBoostConfig.c_dimmingSpeed;

    jconf[m_conf.c_algoAmbientConfig.jsonConfKey]["samplingSeconds"] = m_conf.c_algoAmbientConfig.c_sampleSeconds;

    // config server config
    jconf["configServerConf"]["enabled"] = m_conf.c_configServerConf.enabled;
    jconf["configServerConf"]["authToken"] = m_conf.c_configServerConf.authToken;

    std::ofstream file(m_configPath);
    if (!file) {
        std::cerr << "screenCaptureWorkerBase::updateCurrentConfig() Failed to open config json for writing. The set values will be used during this session but changes won't be saved to disk" << std::endl;
        return;
    }
    file << jconf.dump(4);
    file.close();
}

ScreenCapConfig ScreenLedConfigurator::getCurrentConfig()
{
    std::lock_guard<std::mutex> lock(m_conf_mutex);
    return m_conf;
}

QFileInfo ScreenLedConfigurator::getConfigFilePath()
{
    return QFileInfo(QString::fromStdString(m_configPath));
}
