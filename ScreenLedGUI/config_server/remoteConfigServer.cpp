#include "remoteConfigServer.h"

#include "ifResolver.h"
#include "CrowFiles.h"
#include <random>

std::string ConfigHttpServer::generateAuthToken()
{
    std::random_device rd;
    std::ostringstream out;
    for (int i = 0; i < 32; ++i) {
        out << std::hex
            << std::setw(2)
            << std::setfill('0')
            << (rd() & 0xff);
    }
    qDebug() << "ConfigHttpServer::generateAuthToken() generated: " << out.str();
    return out.str();
}

std::string ConfigHttpServer::getConfigStrForQR()
{
    std::ostringstream os;
    os << m_serverConf.host << "|" << m_serverConf.port << "|" << m_serverConf.authToken;
    return os.str();
}

void ConfigHttpServer::startApp()
{
    // TEMP endpoint to host simple site
    CROW_ROUTE(app, "/")
    ([this] {
        auto mainHtml = std::string(CROW_FILES) + "/main.html";
        qDebug() << "Serving:" << QString::fromStdString(mainHtml);
        std::ifstream file(mainHtml);
        if (!file.is_open()) {
            qDebug() << "Failed to open:" << QString::fromStdString(mainHtml);
            return crow::response(500, "Failed to load main.html");
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        auto html = buffer.str();

        auto resp = crow::response(html);
        resp.set_header(
            "Set-Cookie",
            "auth_token=" + m_serverConf.authToken + "; HttpOnly; SameSite=Strict"
        );

        return resp;
    });

    CROW_ROUTE(app, "/api/algorithms").methods(crow::HTTPMethod::GET)
    ([&](const crow::request& req) {
        //auto cookie = req.get_header_value("Cookie");
        //if (cookie.find(m_serverConf.authToken) == std::string::npos) {
        //    return crow::response(401, "Unauthorized");
        //}

        auto currentScreenLedConf = m_screenLedConfiguratorPtr->getCurrentConfig();
        crow::json::wvalue result;
        crow::json::wvalue::list algorithms;
        for (const auto& algo : algoNameMap)
        {
            crow::json::wvalue obj;
            obj["name"] = algo.first;
            obj["type"] = algo.second;
            obj["active"] = algo.second == currentScreenLedConf.c_algo;
            obj["supportsConfig"] = ScreenLedAlgorithmSupportsConfig(algo.second);
            algorithms.push_back(std::move(obj));
        }
        result["algorithms"] = std::move(algorithms);

        return crow::response(result);
    });

    CROW_ROUTE(app, "/api/algoConfig/<int>").methods(crow::HTTPMethod::GET)
    ([&](const crow::request& req, int id) {
        crow::json::wvalue response;
        auto algo = static_cast<ScreenLedAlgorithm>(id);
        auto currentConfig = m_screenLedConfiguratorPtr->getCurrentConfig();
        if (!ScreenLedAlgorithmSupportsConfig(algo))
        {
            return crow::response(response);
        }

        // TODO: more sensible solution to this instead of lot of ifs. Also these configs should be somehow generated dynamically
        crow::json::wvalue::list configFields;
        if (algo == ScreenLedAlgorithm::FLASH_BOOST)
        {
            crow::json::wvalue obj;
            obj["name"] = "flashTreshold";
            obj["value"] = currentConfig.c_algoFlashBoostConfig.c_flashTreshold;
            obj["type"] = "int";
            obj["min"] = std::get<0>(AlgoFlashBoost_Config::fields()).minVal;
            obj["max"] = std::get<0>(AlgoFlashBoost_Config::fields()).maxVal;
            configFields.push_back(std::move(obj));
            crow::json::wvalue obj2;
            obj2["name"] = "dimmingSpeed";
            obj2["value"] = currentConfig.c_algoFlashBoostConfig.c_dimmingSpeed;
            obj2["type"] = "int";
            obj2["min"] = std::get<1>(AlgoFlashBoost_Config::fields()).minVal;
            obj2["max"] = std::get<1>(AlgoFlashBoost_Config::fields()).maxVal;
            configFields.push_back(std::move(obj2));
        }

        response["fields"] = std::move(configFields);
        return crow::response(response);
    });

    CROW_ROUTE(app, "/api/algoConfig/<int>").methods(crow::HTTPMethod::POST)
    ([this](const crow::request& req, int id) {

        auto algo = static_cast<ScreenLedAlgorithm>(id);

        // Validate the algorithm ID.
        if (!ScreenLedAlgorithmSupportsConfig(algo) &&
            algo != ScreenLedAlgorithm::MEAN_DEFAULT &&
            algo != ScreenLedAlgorithm::MEDIAN)
        {
            return crow::response(400, "Invalid algorithm");
        }

        auto body = crow::json::load(req.body);

        if (!body)
        {
            return crow::response(400, "Invalid JSON");
        }

        auto currentConfig =
            m_screenLedConfiguratorPtr->getCurrentConfig();

        /*
        * Update algorithm-specific configuration if applicable.
        */
        if (algo == ScreenLedAlgorithm::FLASH_BOOST)
        {
            if (!body.has("flashTreshold") ||
                !body.has("dimmingSpeed"))
            {
                return crow::response(
                    400,
                    "Missing FlashBoost configuration"
                );
            }

            const int flashTreshold =
                body["flashTreshold"].i();

            const int dimmingSpeed =
                body["dimmingSpeed"].i();

            const auto flashField =
                std::get<0>(AlgoFlashBoost_Config::fields());

            const auto dimmingField =
                std::get<1>(AlgoFlashBoost_Config::fields());

            if (flashTreshold < flashField.minVal ||
                flashTreshold > flashField.maxVal)
            {
                return crow::response(
                    400,
                    "flashTreshold out of range"
                );
            }

            if (dimmingSpeed < dimmingField.minVal ||
                dimmingSpeed > dimmingField.maxVal)
            {
                return crow::response(
                    400,
                    "dimmingSpeed out of range"
                );
            }

            currentConfig.c_algoFlashBoostConfig.c_flashTreshold =
                flashTreshold;

            currentConfig.c_algoFlashBoostConfig.c_dimmingSpeed =
                dimmingSpeed;
        }

        currentConfig.c_algo = algo;
        m_screenLedConfiguratorPtr->updateCurrentConfig(currentConfig, true);

        return crow::response(200);
    });

    m_serverThread = QThread::create([this]() {
        app.port(m_serverConf.port)
        .multithreaded().run();
    });

    m_serverThread->start();
    qDebug() << "ConfigHTTPServer started at 0.0.0.0:" << m_serverConf.port;
}

bool ConfigHttpServer::start()
{
    if (m_screenLedConfiguratorPtr == nullptr) { return false; }
    auto currentScreenLedConf = m_screenLedConfiguratorPtr->getCurrentConfig();
    m_serverConf = currentScreenLedConf.c_configServerConf;

    if (m_serverConf.authToken.empty())
    {
        m_serverConf.authToken = generateAuthToken();
    }

    if (m_serverConf.host.empty() && currentScreenLedConf.c_preferredLocalNetworkInterface.empty())
    {
        qDebug() << "ConfigHttpServer::start(): host and local network interface selections are both empty in config. Cannot start!";
        return false;
    } else if (m_serverConf.host.empty()) {
        qDebug() << "ConfigHttpServer::start(): host ip is empty but preferred nwinterface is set. Trying to resolve our ip";
        m_serverConf.host = nwInterfaceHelper::getIPv4ForInterface(currentScreenLedConf.c_preferredLocalNetworkInterface);
        if (m_serverConf.host.empty())
        {
            qDebug() << "Failed to resolve ipv4 for interface " << currentScreenLedConf.c_preferredLocalNetworkInterface << " Cannot start!";
            return false;
        }
    }

    startApp();
    currentScreenLedConf = m_screenLedConfiguratorPtr->getCurrentConfig(); //reget in case it changed from elsewhere
    currentScreenLedConf.c_configServerConf = m_serverConf;
    m_screenLedConfiguratorPtr->updateCurrentConfig(currentScreenLedConf, true);
    return true;
}

bool ConfigHttpServer::stop()
{
    if (m_serverThread == nullptr)
    {
        return true;
    }

    app.stop();

    m_serverThread->quit();
    m_serverThread->wait();
    delete m_serverThread;
    m_serverThread = nullptr;
    return true;
}
