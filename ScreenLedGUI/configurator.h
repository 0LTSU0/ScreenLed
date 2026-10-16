#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include <string>
#include <Commons.h>
#include <mutex>
#include <QStandardPaths>

class ScreenLedConfigurator
{
public:
    ScreenLedConfigurator();
    void updateCurrentConfig(ScreenCapConfig newConf, bool save);
    ScreenCapConfig getCurrentConfig();

private:
    bool loadConfigs();
    bool createConfigFile();

    std::string m_configPath = (QStandardPaths::writableLocation(
                                QStandardPaths::AppLocalDataLocation)
                                + "/screenLedConfig.json").toStdString();

    ScreenCapConfig m_conf;
    std::mutex m_conf_mutex;
};

#endif // CONFIGURATOR_H
