#ifndef BRIDGE_CONFIG_H
#define BRIDGE_CONFIG_H

#include <QString>
#include "../proxy/Proxy.h"

class ProviderData {
public:
    QString uuid;
    QString name;
    QString url;
    double updatePeriod = 0.5;
    ProxyType proxyType = {};
    QString encodeType;
};

struct ProxyCmdConfigData {
    QString binaryPath = {};
    ushort listenPort = 8888;
    ushort controllerPort = 9090;
    ushort socksPort = 1089;
    bool allowLan = true;
    QString bindAddress = "*";
    QString logLevel = "info";
};

class Proxy;

class Config {
public:
    static const QString KEY_PROVIDER;
    static const QString KEY_PROXY;
    static const QString KEY_PROXY_CMD_BINARY;
    static const QString KEY_PROXY_CMD_LISTEN_PORT;
    static const QString KEY_PROXY_CMD_CONTROLLER_PORT;
    static const QString KEY_PROXY_CMD_SOCKS_PORT;
    static const QString KEY_PROXY_CMD_ALLOW_LAN;
    static const QString KEY_PROXY_CMD_BIND_ADDRESS;
    static const QString KEY_PROXY_CMD_LOG_LEVEL;


public:
    static QString appConfigDir();

    static QList<ProviderData> getProviders();

    static void setProvider(ProviderData &provider);

    static void deleteProvider(const QString &qString);

    static void setProxy(Proxy &provider);

    static QList<Proxy> getProxies();

    static ProxyCmdConfigData getProxyCmdConfig();

    static void setClashConfig(ProxyCmdConfigData &config);

    static QString get(const QString &key);

    static QString get(const QString &org, const QString &key);

    static void set(const QString &key, const QString &value);

    static void set(const QMap<QString, QString> &kvs);

    static void set(const QString &group, const QString &key, const QString &value);

    static void prepareConfigDir();

    static void setConfigDir(const QString &dir);

    static QString getProviderCacheFile(const QString &providerName);

    static QString getProxyConfigFilePath();

private:
    static QString configFile();

    static QString preferConfigDir;

};


#endif //BRIDGE_CONFIG_H
