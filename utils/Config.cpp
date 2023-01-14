#include "Config.h"
#include <QProcessEnvironment>
#include <QSettings>
#include <QDir>
#include <QList>
#include <QUuid>

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)

const QString Config::KEY_PROVIDER = "providers";
const QString Config::KEY_PROXY = "proxies";
const QString Config::KEY_CLASH_BINARY = "clash-binary";
const QString Config::KEY_CLASH_LISTEN_PORT = "clash-listen-port";
const QString Config::KEY_CLASH_CONTROLLER_PORT = "clash-controller-port";
const QString Config::KEY_CLASH_SOCKS_PORT = "clash-socks-port";
const QString Config::KEY_CLASH_ALLOW_LAN = "clash-allow-lan";
const QString Config::KEY_CLASH_BIND_ADDRESS = "clash-bind-address";
const QString Config::KEY_CLASH_LOG_LEVEL = "clash-log-level";

QString Config::preferConfigDir = "";

QString Config::configDir() {
    if (preferConfigDir.length() > 0) {
        return preferConfigDir;
    }
    auto env = QProcessEnvironment::systemEnvironment();
    auto home = env.value("HOME");
    auto name = QString("bridge");
#ifdef CMAKE_BUILD_TYPE
    auto buildType = QString(STR(CMAKE_BUILD_TYPE));
    if (buildType.compare("Debug") == 0) {
        name += "-debug";
    }
#endif
    return home + "/.config/" + name;
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
    int n = settings.beginReadArray(KEY_PROVIDER);

    QList<ProviderData> list;
    for (auto i = 0; i < n; i++) {
        settings.setArrayIndex(i);
        auto uuid = settings.value("uuid").toString();
        auto name = settings.value("name").toString();
        auto type = settings.value("type").toInt();
        auto subscribeUrl = settings.value("url").toString();
        if (uuid.isEmpty()) {
            continue;
        }
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
    auto size = settings.beginReadArray(KEY_PROVIDER);
    for (auto i = 0; i < size; i++) {
        settings.setArrayIndex(i);
        auto v = settings.value("uuid").toString();
        if (v == provider.uuid) {
            lastIndex = i;
            break;
        }
    }
    settings.endArray();

    settings.beginWriteArray(KEY_PROVIDER);

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
    auto size = settings.beginReadArray(KEY_PROXY);
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
    settings.beginWriteArray(KEY_PROXY);

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
    auto size = settings.beginReadArray(KEY_PROXY);
    for (auto i = 0; i < size; i++) {
        settings.setArrayIndex(i);
        auto json = settings.value("json").toByteArray();
        auto p = Proxy::from(json);
        list.append(p);
    }
    settings.endArray();
    return list;
}

ClashConfigData Config::getClashConfig() {
    ClashConfigData cfg;
    cfg.binaryPath = get(KEY_CLASH_BINARY);
    auto setPort = [](ushort *port, const QString &key) {
        bool ok;
        auto v = get(key).toUShort(&ok);
        if (ok) {
            *port = v;
        }
    };
    setPort(&cfg.listenPort, KEY_CLASH_LISTEN_PORT);
    setPort(&cfg.controllerPort, KEY_CLASH_CONTROLLER_PORT);
    setPort(&cfg.socksPort, KEY_CLASH_SOCKS_PORT);
    cfg.allowLan = get(KEY_CLASH_ALLOW_LAN).toInt();
    auto bindAddress = get(KEY_CLASH_BIND_ADDRESS);
    if (bindAddress.isEmpty()) {
        bindAddress = "*";
    }
    cfg.bindAddress = bindAddress;
    cfg.logLevel = get(KEY_CLASH_LOG_LEVEL);

    return cfg;
}

void Config::setClashConfig(ClashConfigData &cfg) {
    set({
                {KEY_CLASH_LISTEN_PORT,     QString::number(cfg.listenPort)},
                {KEY_CLASH_CONTROLLER_PORT, QString::number(cfg.controllerPort)},
                {KEY_CLASH_SOCKS_PORT,      QString::number(cfg.socksPort)},
                {KEY_CLASH_ALLOW_LAN,       QString::number(cfg.allowLan)},
                {KEY_CLASH_BIND_ADDRESS,    cfg.bindAddress},
                {KEY_CLASH_LOG_LEVEL,       cfg.logLevel},
        });
}

void Config::set(const QString &key, const QString &value) {
    prepareConfigDir();

    QSettings settings(configFile(), QSettings::Format::IniFormat);
    settings.setValue(key, value);
    settings.sync();
}

void Config::set(const QMap<QString, QString> &kvs) {
    prepareConfigDir();

    QSettings settings(configFile(), QSettings::Format::IniFormat);
    for (auto &key: kvs.keys()) {
        settings.setValue(key, kvs[key]);
    }
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

QString Config::getClashYamlPath() {
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
    auto size = settings.beginReadArray(KEY_PROVIDER);
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
    settings.beginWriteArray(KEY_PROVIDER);

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

    settings.beginWriteArray(KEY_PROVIDER, size - 1);
    settings.endArray();

    settings.sync();
}
