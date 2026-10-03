#pragma once

#include <string>
#include <QNetworkInterface>
#include <QDebug>
#include <QString>

namespace nwInterfaceHelper {

inline std::string getIPv4ForInterface(const std::string& interfaceName)
{
    QNetworkInterface iface = QNetworkInterface::interfaceFromName(QString::fromStdString(interfaceName));
    if (!iface.isValid()) {
        return "";
    }
    for (const QNetworkAddressEntry &entry : iface.addressEntries())
    {
        const QHostAddress &addr = entry.ip();
        if (addr.protocol() == QAbstractSocket::IPv4Protocol)
        {
            qDebug() << "getIPv4ForInterface() returning " << addr.toString();
            return addr.toString().toStdString();
        }
    }
    return "";
}

}
