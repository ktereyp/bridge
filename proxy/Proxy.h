#ifndef BRIDGE_PROXY_H
#define BRIDGE_PROXY_H

#include <QString>
#include <QMetaType>
#include <QMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

enum ProxyType {
    Trojan,
    ShadowSocks,
    ShadowSocksR,
    Http,
    Socks5,
    Vmess,
    Vless,
    Snell,
};

class ProxyShadowSocks {
public:
    QString server;
    QString port;
    QString cipher;
    QString password;
    QString plugin;

    QString pluginOptMode;
    bool pluginOptTls{};
    bool pluginOptSkipCertVerify{};
    QString pluginOptHost;
    QString pluginOptPath;
    bool pluginOptMux{};
    QMap<QString, QString> headers;

public:
    QJsonObject toJson() {
        QJsonObject json;
        json["server"] = this->server;
        json["port"] = this->port;
        json["cipher"] = this->cipher;
        json["password"] = this->password;
        json["plugin"] = this->plugin;
        json["pluginOptMode"] = this->pluginOptMode;
        json["pluginOptTls"] = this->pluginOptTls;
        json["pluginOptSkipCertVerify"] = this->pluginOptSkipCertVerify;
        json["pluginOptHost"] = this->pluginOptHost;
        json["pluginOptPath"] = this->pluginOptPath;
        json["pluginOptMux"] = this->pluginOptMux;

        QVariantMap headers;
        for (const auto &h: this->headers.keys()) {
            headers[h] = this->headers[h];
        }
        json["headers"] = QJsonObject::fromVariantMap(headers);
        return json;
    }

    static ProxyShadowSocks from(const QJsonObject &json) {
        ProxyShadowSocks p;
        p.server = json["server"].toString();
        p.port = json["port"].toString();
        p.cipher = json["cipher"].toString();
        p.password = json["password"].toString();
        p.plugin = json["plugin"].toString();
        p.pluginOptMode = json["pluginOptMode"].toString();
        p.pluginOptTls = json["pluginOptTls"].toBool();
        p.pluginOptSkipCertVerify = json["pluginOptSkipCertVerify"].toBool();
        p.pluginOptHost = json["pluginOptHost"].toString();
        p.pluginOptPath = json["pluginOptPath"].toString();
        p.pluginOptMux = json["pluginOptMux"].toBool();

        auto headers = json["headers"].toObject().toVariantMap();
        for (const auto &h: headers.keys()) {
            p.headers[h] = headers[h].toString();
        }
        return p;
    }
};

class ProxyTrojan {
public:
    QString server;
    QString port;
    QString password;
    bool udp{};
    QString sni;
    QList<QString> alpn;
    bool skipCertVerify{};
    QString network;

    QString grpcServiceName;

    QString wsPath;
    QMap<QString, QString> wsHeaders;

    QJsonObject toJson() {
        QJsonObject json;
        json["server"] = this->server;
        json["port"] = this->port;
        json["password"] = this->password;
        json["udp"] = this->udp;
        json["sni"] = this->sni;
        json["alpn"] = QJsonArray::fromStringList(this->alpn);
        json["skipCertVerify"] = this->skipCertVerify;
        json["network"] = this->network;
        json["grpcServiceName"] = this->grpcServiceName;
        json["wsPath"] = this->wsPath;
        QVariantMap headers;
        for (const auto &h: this->wsHeaders.keys()) {
            headers[h] = this->wsHeaders[h];
        }
        json["wsHeaders"] = QJsonObject::fromVariantMap(headers);
        return json;
    }

    static ProxyTrojan from(const QJsonObject &json) {
        ProxyTrojan p;
        p.server = json["server"].toString();
        p.port = json["port"].toString();
        p.password = json["password"].toString();
        p.udp = json["udp"].toBool();
        p.sni = json["sni"].toString();
        auto alpn = json["alpn"].toArray().toVariantList();
        for (const auto &item: alpn) {
            p.alpn.append(item.toString());
        }
        p.skipCertVerify = json["skipCertVerify"].toBool();
        p.network = json["network"].toString();
        p.grpcServiceName = json["grpcServiceName"].toString();
        p.wsPath = json["wsPath"].toString();

        auto headers = json["wsHeaders"].toObject().toVariantMap();
        for (const auto &h: headers.keys()) {
            p.wsHeaders[h] = headers[h].toString();
        }
        return p;
    }
};

class ProxyVless {
public:
    QString server;
    QString port;
    QString password;
    QString encryption;
    QString flow;
    QString network;
    QString security;
    QString realityFingerprint;
    QString realityServerName;
    QString realityPublicKey;

    QJsonObject toJson() {
        QJsonObject json;
        json["server"] = this->server;
        json["port"] = this->port;
        json["password"] = this->password;
        json["encryption"] = this->encryption;
        json["flow"] = this->flow;
        json["network"] = this->network;
        json["security"] = this->security;
        json["realityFingerprint"] = this->realityFingerprint;
        json["realityServerName"] = this->realityServerName;
        json["realityPublicKey"] = this->realityPublicKey;
        return json;
    }

    static ProxyVless from(const QJsonObject &json) {
        ProxyVless p;
        p.server = json["server"].toString();
        p.port = json["port"].toString();
        p.password = json["password"].toString();
        p.encryption = json["encryption"].toString();
        p.flow = json["flow"].toString();
        p.network = json["network"].toString();
        p.security = json["security"].toString();
        p.realityFingerprint = json["realityFingerprint"].toString();
        p.realityServerName = json["realityServerName"].toString();
        p.realityPublicKey = json["realityPublicKey"].toString();
        return p;
    }
};

class Proxy {
public:
    static Proxy parse(QString input);
    static Proxy trojan(QString input);
    static Proxy vless(QString input);

    static Proxy from(const QByteArray &data);

    QString verifyComplete() const;

    [[nodiscard]] bool isValid() const;

    [[nodiscard]] QString info() const;

    QString toClashProxy(const QString &name);

    QJsonObject toV2rayProxy(const QString &tag);

    QByteArray toJson();

public:
    QString uuid;
    ProxyType proxyType = {};
    QString name;
    bool lastRelay{};

    ProxyShadowSocks shadowSocksData;
    ProxyTrojan trojanData;
    ProxyVless vlessData;
};

Q_DECLARE_METATYPE(Proxy);

#endif //BRIDGE_PROXY_H
