#pragma once

#pragma push_macro("signals") // QT signals conflict with crow signals
#undef signals
#include <crow.h>
#pragma pop_macro("signals")

#include "../configurator.h"

#include <string>
#include <QDebug>
#include <QThread>

class ConfigHttpServer {
public:
    ConfigHttpServer(ScreenLedConfigurator* configuratorPtr) : m_screenLedConfiguratorPtr(configuratorPtr) {}

    ~ConfigHttpServer() {
        stop();
    }

    bool start();
    bool stop();
    std::string getConfigStrForQR();

private:
    static std::string generateAuthToken();
    void startApp();

    ConfigServerConf m_serverConf = ConfigServerConf();
    ScreenLedConfigurator* m_screenLedConfiguratorPtr = nullptr;
    crow::SimpleApp app;
    QThread* m_serverThread = nullptr;
};
