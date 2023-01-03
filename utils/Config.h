#ifndef BRIDGE_CONFIG_H
#define BRIDGE_CONFIG_H

#include <QString>
#include "../proxy/Proxy.h"

class ProviderData {
public:
    QString uuid;
    QString name;
    QString url;
    ProxyType proxyType = {};
    QString encodeType;
};

class Proxy;

class Config {
public:
    static const QString PROVIDER_KEY;
    static const QString PROXY_KEY;
    static const QString CLASH_BINARY_KEY;

    static void deleteProvider(const QString &qString);

public:
    static QString configDir();

    static QList<ProviderData> getProviders();

    static void setProvider(ProviderData &provider);

    static void setProxy(Proxy &provider);

    static QList<Proxy> getProxies();

    static QString get(const QString &key);

    static QString get(const QString &org, const QString &key);

    static void set(const QString &key, const QString &value);

    static void set(const QString &group, const QString &key, const QString &value);

    static void prepareConfigDir();

    static void setConfigDir(const QString &dir);

    static QString getProviderCacheFile(const QString &providerName);

    static QString getClashConfigFile();

private:
    static QString configFile();

    static QString preferConfigDir;

};


#endif //BRIDGE_CONFIG_H
