#include "Config.h"
#include "../proxy/Proxy.h"
#include <QProcessEnvironment>
#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QList>
#include <QUuid>

const QString Config::PROVIDER_KEY = "providers";
const QString Config::PROXY_KEY = "proxies";
const QString Config::CLASH_BINARY_KEY = "clash-binary";

QString Config::preferConfigDir = "";

QString Config::configDir() {
    if (preferConfigDir.length() > 0) {
        return preferConfigDir;
    }
    auto env = QProcessEnvironment::systemEnvironment();
    auto home = env.value("HOME");
    return home + "/.config/" + "bridge";
}

QString Config::get(const QString &key) {
    QSettings settings(configFile(), QSettings::Format::IniFormat);
    auto v = settings.value(key);
    if (v.isValid()) {
        return v.toString();
    }
    return {};
}

QString Config::get(const QString &group, const QString &key) {
    QSettings settings(configFile(), QSettings::Format::IniFormat);
    settings.beginGroup(group);
    auto v = settings.value(key);
    settings.endGroup();
    if (v.isValid()) {
        return v.toString();
    }
    return {};
}

QList<ProviderData> Config::getProviders() {
    QSettings settings(configFile(), QSettings::Format::IniFormat);
    int n = settings.beginReadArray(PROVIDER_KEY);

    QList<ProviderData> list;
    for (auto i = 0; i < n; i++) {
        settings.setArrayIndex(i);
        auto uuid = settings.value("uuid").toString();
        auto name = settings.value("name").toString();
        auto type = settings.value("type").toInt();
        auto subscribeUrl = settings.value("url").toString();
        ProviderData provider;
        provider.uuid = uuid;
        provider.name = name;
        provider.url = subscribeUrl;
        provider.proxyType = static_cast<ProxyType>(type);
        list.append(provider);
    }
    settings.endArray();
    return list;
}

void Config::setProvider(ProviderData &provider) {
    prepareConfigDir();
    QSettings settings(configFile(), QSettings::Format::IniFormat);

    if (provider.uuid.isEmpty()) {
        provider.uuid = QUuid::createUuid().toString();
    }

    // find exist item
    int lastIndex = -1;
    auto size = settings.beginReadArray(PROVIDER_KEY);
    for (auto i = 0; i < size; i++) {
        settings.setArrayIndex(i);
        auto v = settings.value("uuid").toString();
        if (v == provider.uuid) {
            lastIndex = i;
            break;
        }
    }
    settings.endArray();

    settings.beginWriteArray(PROVIDER_KEY);

    if (lastIndex >= 0) {
        settings.setArrayIndex(lastIndex);
    } else {
        settings.setArrayIndex(lastIndex + 1);
    }

    settings.setValue("uuid", provider.uuid);
    settings.setValue("name", provider.name);
    settings.setValue("url", provider.url);
    settings.setValue("type", provider.proxyType);

    settings.endArray();

    settings.sync();
}

void Config::setProxy(Proxy &proxy) {
    prepareConfigDir();
    QSettings settings(configFile(), QSettings::Format::IniFormat);

    if (proxy.uuid.isEmpty()) {
        proxy.uuid = QUuid::createUuid().toString();
    }
    // find exist item
    int lastIndex = -1;
    auto size = settings.beginReadArray(PROXY_KEY);
    for (auto i = 0; i < size; i++) {
        settings.setArrayIndex(i);
        auto v = settings.value("uuid").toString();
        if (v == proxy.uuid) {
            lastIndex = i;
            break;
        }
    }
    settings.endArray();

    //
    settings.beginWriteArray(PROXY_KEY);

    if (lastIndex >= 0) {
        settings.setArrayIndex(lastIndex);
    } else {
        settings.setArrayIndex(lastIndex + 1);
    }
    settings.setValue("json", proxy.toJson());

    settings.endArray();

    settings.sync();
}

QList<Proxy> Config::getProxies() {
    QSettings settings(configFile(), QSettings::Format::IniFormat);

    QList<Proxy> list;
    auto size = settings.beginReadArray(PROXY_KEY);
    for (auto i = 0; i < size; i++) {
        settings.setArrayIndex(i);
        auto json = settings.value("json").toByteArray();
        auto p = Proxy::from(json);
        list.append(p);
    }
    settings.endArray();
    return list;
}

void Config::set(const QString &key, const QString &value) {
    prepareConfigDir();

    QSettings settings(configFile(), QSettings::Format::IniFormat);
    settings.setValue(key, value);
    settings.sync();
}

void Config::set(const QString &group, const QString &key, const QString &value) {
    prepareConfigDir();

    QSettings settings(configFile(), QSettings::Format::IniFormat);
    settings.beginGroup(group);
    settings.setValue(key, value);
    settings.endGroup();
    settings.sync();
}

QString Config::configFile() {
    return configDir() + "/" + "bridge.ini";
}

void Config::prepareConfigDir() {
    QDir dir(configDir());
    if (!dir.exists()) {
        dir.mkdir(".");
    }
}

void Config::setConfigDir(const QString &dir) {
    preferConfigDir = dir;
}

QString Config::getProviderCacheFile(const QString &providerName) {
    return Config::configDir() + "/" + providerName + ".json";
}

QString Config::getClashConfigFile() {
    prepareConfigDir();
    auto clashConfigDir = configDir() + "/" + "generated";

    QDir dir(clashConfigDir);
    if (!dir.exists()) {
        if (!dir.mkpath(clashConfigDir)) {
            qCritical() << "cannot create dir " << clashConfigDir;
        }
    }
    return clashConfigDir + "/" + "clash.yaml";
}

void Config::deleteProvider(const QString &providerUuid) {
    prepareConfigDir();
    QSettings settings(configFile(), QSettings::Format::IniFormat);

    // find exist item
    int providerIndex = -1;
    auto size = settings.beginReadArray(PROVIDER_KEY);
    for (auto i = 0; i < size; i++) {
        settings.setArrayIndex(i);
        auto v = settings.value("uuid").toString();
        if (v == providerUuid) {
            providerIndex = i;
            break;
        }
    }
    settings.endArray();

    //
    settings.beginWriteArray(PROVIDER_KEY);

    if (providerIndex < 0) {
        qWarning() << "cannot find a provider whose uuid is " + providerUuid;
        return;
    }
    settings.setArrayIndex(providerIndex);
    settings.remove("uuid");
    settings.remove("name");
    settings.remove("url");
    settings.remove("type");

    settings.endArray();

    settings.sync();
}
