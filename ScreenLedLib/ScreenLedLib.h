#include <string>

class ScreenLed_Worker{
public:
    ScreenLed_Worker(std::string configPath) : m_configPath(configPath) {
        loadConfigs();
    }

    void startWorker();

private:
    bool loadConfigs();

    std::string m_configPath;
};
