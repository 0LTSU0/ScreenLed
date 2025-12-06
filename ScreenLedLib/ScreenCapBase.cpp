#include "ScreenCapBase.h"
#include <iostream>
#include <fstream>
#include <format>
#include <chrono>
#include "json.hpp"
#include <QThread>

using json = nlohmann::json;

void screenCaptureWorkerBase::run() {
    m_isRunning = true;
    for (const auto& c : m_conf.c_clientInfos) {
        if (!openUDPPort(c.host.c_str(), c.port)){
            std::cerr << "screenCaptureWorkerBase::run() FAILED TO OPEN UDP SOCKET TO: " << c.host << ":" << c.port << std::endl;
            return;
        }
    }
    initScreenShotting();
    int perfCtr = 0;
    auto measureStartTime = std::chrono::high_resolution_clock::now();
    double targetTimePerLoopMS = (1.0 / MAX_FPS) * 1000;
    while(m_isRunning) {
        auto loopStartTime = std::chrono::high_resolution_clock::now();
        takeScreenShot();
        runAnalFunc();
        sendRGBData(createRGBDataString().c_str());
        if (perfCtr == 10) {
            std::chrono::duration<double> timePer10Frames = std::chrono::high_resolution_clock::now() - measureStartTime;
            double avgTimePerFrame = timePer10Frames.count() / 10.0;
            m_fps = 1 / avgTimePerFrame;
            perfCtr = 0;
            measureStartTime = std::chrono::high_resolution_clock::now();
        }
        perfCtr++;
        std::chrono::duration<double, std::milli> loopTime = std::chrono::high_resolution_clock::now() - loopStartTime;
        if (loopTime.count() < targetTimePerLoopMS) {
            QThread::msleep(targetTimePerLoopMS - loopTime.count());
        }
    }
}

void screenCaptureWorkerBase::stop() {
    m_isRunning = false;
    m_fps = 0.0;
    closeUDPPorts();
    deinitScreenShotting();
}

const std::string screenCaptureWorkerBase::createRGBDataString() {
    std::string packet = "";
    for (const auto& val : m_rgbData) {
        packet.append(std::format("{},{},{};", val.r, val.g, val.b));
    }
    return packet;
}

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
    m_conf.c_showDebugPreview = conf["showDebugPreview"].get<bool>();
    m_conf.c_screenResX = conf["screenResX"].get<int>();
    m_conf.c_screenResY = conf["screenResY"].get<int>();
    m_conf.c_algo = conf["algo"].get<ScreenLedAlgorithm>();
    m_conf.c_autorunScriptPath = conf.contains("autorunScriptPath") ? QString::fromStdString(conf["autorunScriptPath"].get<std::string>()) : "";

    m_conf.c_clientInfos.clear();
    for (const auto& item : conf["clients"]) {
        clientInfo c;
        c.host = item["addr"];
        c.port = item["port"];
        c.deviceType = item["deviceType"];
        m_conf.c_clientInfos.push_back(c);
    }

    return true;
}

bool screenCaptureWorkerBase::createConfigFile(){
    json jconf;
    ScreenCapConfig defaultConfig;
    jconf["debugSSInterval"] = defaultConfig.c_debugSSInterval;
    jconf["keepDebugSSOnClipboard"] = defaultConfig.c_keepDebugSSOnClipboard;
    jconf["showDebugPreview"] = defaultConfig.c_showDebugPreview;
    jconf["screenResX"] = defaultConfig.c_screenResX;
    jconf["screenResY"] = defaultConfig.c_screenResY;
    jconf["algo"] = defaultConfig.c_algo;
    jconf["autorunScriptPath"] = defaultConfig.c_autorunScriptPath.toStdString();

    for (const auto& c : defaultConfig.c_clientInfos) {
        jconf["clients"].push_back({
            {"addr", c.host},
            {"port", c.port},
            {"deviceType", c.deviceType}
        });
    }

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
    jconf["showDebugPreview"] = m_conf.c_showDebugPreview;
    jconf["screenResX"] = m_conf.c_screenResX;
    jconf["screenResY"] = m_conf.c_screenResY;
    jconf["algo"] = m_conf.c_algo;
    jconf["autorunScriptPath"] = m_conf.c_autorunScriptPath.toStdString();
    for (const auto& c : m_conf.c_clientInfos) {
        jconf["clients"].push_back({
            {"addr", c.host},
            {"port", c.port},
            {"deviceType", c.deviceType}
        });
    }
    std::ofstream file(m_configPath);
    if (!file) {
        std::cerr << "screenCaptureWorkerBase::updateCurrentConfig() Failed to open config json for writing. The set values will be used during this session but changes won't be saved to disk" << std::endl;
        return;
    }
    file << jconf.dump(4);
    file.close();
}
