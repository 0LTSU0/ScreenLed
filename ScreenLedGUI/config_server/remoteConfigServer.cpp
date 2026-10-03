#include "remoteConfigServer.h"

#include "ifResolver.h"
#include "CrowFiles.h"
#include "nlohmann2crow.h"
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

        auto j = currentConfig.c_algoFlashBoostConfig.toJson();
        return crow::response(to_crow(j));
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
            auto j = to_nlohmann(body);
            auto newAlgoConf = AlgoFlashBoost_Config();
            if (newAlgoConf.fromJson(j))
            {
                currentConfig.c_algoFlashBoostConfig = newAlgoConf;
            }
        }

        currentConfig.c_algo = algo;
        m_screenLedConfiguratorPtr->updateCurrentConfig(currentConfig, true);

        // this should be done async for sure
        configUpdated();

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
