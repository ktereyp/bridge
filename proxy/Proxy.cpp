//
// Created by try on 12/29/22.
//

#include "Proxy.h"
#include <QDebug>
#include <QUrl>
#include <QUrlQuery>

Proxy Proxy::parse(QString input) {
    if (input.startsWith("trojan")) {
        return trojan(input);
    }
    if (input.startsWith("vless")) {
        return vless(input);
    }
    return {};
}

// "trojan://cd70c3cd-d532-3966-a292-c3c057321680@aaaa.aaaa.gay:50037?sni=v2ru.doubledou.icu#%E2%91%A0Trojan%E4%BF%84%E7%BD%97%E6%96%AF%E7%94%B5%E4%BF%A1%E7%A7%BB%E5%8A%A8%E4%BC%98%E5%8C%96%E2%98%80"
Proxy Proxy::trojan(QString input) {
    const QString schema("trojan://");
    if (!input.startsWith(schema)) {
        qDebug() << "malformed trojan url";
        return {};
    }

    QUrl url(input);
    Proxy p;
    p.proxyType = ProxyType::Trojan;
    p.name = "[trojan] " + url.fragment(QUrl::FullyDecoded);
    p.trojanData.server = url.host();
    p.trojanData.port = QString("%1").arg(url.port());
    p.trojanData.password = url.userName();
    QUrlQuery urlQuery(url.query());
    p.trojanData.sni = urlQuery.queryItemValue("sni");
    return p;
}

Proxy Proxy::vless(QString input) {
    const QString schema("vless://");
    if (!input.startsWith(schema)) {
        qDebug() << "malformed trojan url";
        return {};
    }

    QUrl url(input);
    Proxy p;
    p.proxyType = ProxyType::Vless;
    p.name = "[vless] " + url.fragment(QUrl::FullyDecoded);
    p.vlessData.server = url.host();
    p.vlessData.port = QString("%1").arg(url.port());
    p.vlessData.password = url.userName();
    QUrlQuery urlQuery(url.query());
    p.vlessData.security = urlQuery.queryItemValue("security");
    p.vlessData.encryption = urlQuery.queryItemValue("encryption");
    p.vlessData.realityPublicKey = urlQuery.queryItemValue("pbk");
    p.vlessData.realityFingerprint = urlQuery.queryItemValue("fp");
    p.vlessData.realityFingerprint = urlQuery.queryItemValue("fp");
    p.vlessData.network = urlQuery.queryItemValue("type");
    p.vlessData.flow = urlQuery.queryItemValue("flow");
    p.vlessData.realityServerName = urlQuery.queryItemValue("sni");
    p.vlessData.realityServerName = urlQuery.queryItemValue("sni");
    return p;
}

QString Proxy::verifyComplete() const {
    if (this->proxyType == ProxyType::Trojan) {
        if (trojanData.server.isEmpty()) {
            return "server cannot be empty";
        }
        if (trojanData.port.isEmpty()) {
            return "port cannot be empty";
        }
        if (trojanData.password.isEmpty()) {
            return "password cannot be empty";
        }
    } else if (this->proxyType == ProxyType::ShadowSocks) {
        if (shadowSocksData.server.isEmpty()) {
            return "server cannot be empty";
        }
        if (shadowSocksData.port.isEmpty()) {
            return "port cannot be empty";
        }
        if (shadowSocksData.password.isEmpty()) {
            return "password cannot be empty";
        }
        if (shadowSocksData.cipher.isEmpty()) {
            return "cipher cannot be empty";
        }
    }
    return {};
}

QByteArray Proxy::toJson() {
    QJsonObject json;
    json["uuid"] = this->uuid;
    json["proxyType"] = this->proxyType;
    json["name"] = this->name;
    json["lastRelay"] = this->lastRelay;
    json["shadowSocksData"] = shadowSocksData.toJson();
    json["trojanData"] = trojanData.toJson();
    return QJsonDocument(json).toJson();
}

Proxy Proxy::from(const QByteArray &data) {
    auto json = QJsonDocument::fromJson(data).object();
    Proxy p;
    p.uuid = json["uuid"].toString();
    p.proxyType = static_cast<ProxyType>(json["proxyType"].toInt());
    p.name = json["name"].toString();
    p.lastRelay = json["lastRelay"].toBool();
    p.shadowSocksData = ProxyShadowSocks::from(json["shadowSocksData"].toObject());
    p.trojanData = ProxyTrojan::from(json["trojanData"].toObject());
    return p;
}

bool Proxy::isValid() const {
    if (proxyType == ProxyType::Trojan) {
        return !trojanData.server.isEmpty();
    } else if (proxyType == ProxyType::ShadowSocks) {
        return !shadowSocksData.server.isEmpty();
    } else if (proxyType == ProxyType::Vless) {
        return !vlessData.server.isEmpty();
    }
    return false;
}

QString Proxy::info() const {
    QString msg(name);
    if (proxyType == ProxyType::Trojan) {
        msg += " - " + trojanData.server;
    } else if (proxyType == ProxyType::ShadowSocks) {
        msg += " - " + shadowSocksData.server;
    }
    return msg;
}

QString Proxy::toClashProxy(const QString &name) {
    if (proxyType == ProxyType::Trojan) {
        auto cfg = QString(
                "  - name: \"%1\"\n"
                "    type: trojan\n"
                "    server: %2\n"
                "    port: %3\n"
                "    password: %4\n"
                "    sni: %5\n"
                "    skip-cert-verify: %6\n"
                "    udp: %7\n"
        ).arg(name, trojanData.server, trojanData.port, trojanData.password, trojanData.sni)
                .arg(trojanData.skipCertVerify)
                .arg(trojanData.udp);
        if (!trojanData.network.isEmpty()) {
            cfg += QString(""
                           "    network: %1\n"
            ).arg(trojanData.network);
            if (trojanData.network == "grpc") {
                cfg += QString(""
                               "    grpc-opts:\n"
                               "      grpc-service-name: \"%1\"\n"
                ).arg(trojanData.grpcServiceName);
            }
            if (trojanData.network == "ws") {
                cfg += QString(""
                               "    ws-opts:\n"
                ).arg(trojanData.grpcServiceName);
                if (!trojanData.wsPath.isEmpty()) {
                    cfg += QString(""
                                   "      path: \"%1\"\n"
                    ).arg(trojanData.wsPath);
                }
                if (!trojanData.wsHeaders.isEmpty()) {
                    cfg += QString(""
                                   "      headers:\n"
                    );
                    for (auto k: trojanData.wsHeaders.keys()) {
                        cfg += QString(""
                                       "        %1: %2\n"
                        ).arg(k, trojanData.wsHeaders[k]);
                    }
                }
            }
        }
        return cfg;
    }

    if (proxyType == ProxyType::ShadowSocks) {
        auto cfg = QString(
                "  - name: \"%1\"\n"
                "    type: ss\n"
                "    server: %2\n"
                "    port: %3\n"
                "    password: %4\n"
                "    cipher: %5\n"
                "    udp: %6\n"
        ).arg(name, shadowSocksData.server, shadowSocksData.port, shadowSocksData.password, shadowSocksData.cipher)
                .arg(trojanData.udp);
        if (!shadowSocksData.plugin.isEmpty()) {
            cfg += QString(""
                           "    plugin: %1\n"
            ).arg(shadowSocksData.plugin);
            if (shadowSocksData.plugin == "obfs") {
                cfg += QString(""
                               "    plugin-opts:\n"
                               "      mode: %1\n"
                ).arg(shadowSocksData.pluginOptMode);
                if (!shadowSocksData.pluginOptHost.isEmpty()) {
                    cfg += QString(""
                                   "      host: %1\n"
                    ).arg(shadowSocksData.pluginOptHost);
                }
            }
            if (shadowSocksData.plugin == "v2ray-plugin") {
                cfg += QString(""
                               "    plugin-opts:\n"
                               "      mode: %1\n"
                ).arg(shadowSocksData.pluginOptMode);
                cfg += QString(""
                               "      tls: %1\n"
                ).arg(shadowSocksData.pluginOptTls);
                cfg += QString(""
                               "      skip-cert-verify: %2\n"
                ).arg(shadowSocksData.pluginOptSkipCertVerify);
                cfg += QString(""
                               "      mux: %3\n"
                ).arg(shadowSocksData.pluginOptMux);
                if (!shadowSocksData.pluginOptHost.isEmpty()) {
                    cfg += QString(""
                                   "      host: %1\n"
                    ).arg(shadowSocksData.pluginOptHost);
                }
                if (!shadowSocksData.pluginOptPath.isEmpty()) {
                    cfg += QString(""
                                   "      path: %1\n"
                    ).arg(shadowSocksData.pluginOptPath);
                }
                if (!shadowSocksData.headers.isEmpty()) {
                    cfg += QString(""
                                   "      headers:\n"
                    );
                    for (auto k: shadowSocksData.headers.keys()) {
                        cfg += QString(""
                                       "        %1: %2\n"
                        ).arg(k, shadowSocksData.headers[k]);
                    }
                }
            }
        }
        return cfg;
    }
    return {};
}

QJsonObject Proxy::toV2rayProxy(const QString &tag) {
    if (proxyType == ProxyType::ShadowSocks) {
        return QJsonObject::fromVariantMap(
                {
                        {"tag",      tag},
                        {"protocol", "shadowsocks"},
                        {
                         "settings", QJsonObject::fromVariantMap(
                                {
                                        {"servers", QJsonArray::fromVariantList(
                                                {
                                                        QJsonObject::fromVariantMap(
                                                                {
                                                                        {"address",  shadowSocksData.server},
                                                                        {"method",   shadowSocksData.cipher},
                                                                        {"ota",      false},
                                                                        {"password", shadowSocksData.password},
                                                                        {"port",     shadowSocksData.port.toInt()}
                                                                }
                                                        )}
                                        )},
                                })
                        },
                }
        );
    }
    if (proxyType == ProxyType::Vless) {
        return QJsonObject::fromVariantMap(
                {
                        {"tag",            tag},
                        {"protocol",       "vless"},
                        {"settings",       QJsonObject::fromVariantMap(
                                {
                                        {"vnext", QJsonArray::fromVariantList(
                                                {
                                                        QJsonObject::fromVariantMap(
                                                                {
                                                                        {"address", vlessData.server},
                                                                        {"port",    vlessData.port.toInt()},
                                                                        {"users",   QJsonArray::fromVariantList(
                                                                                {
                                                                                        QJsonObject::fromVariantMap(
                                                                                                {
                                                                                                        {"id",         vlessData.password},
                                                                                                        {"encryption", vlessData.encryption},
                                                                                                        {"flow",       vlessData.flow}
                                                                                                })
                                                                                })
                                                                        }
                                                                }
                                                        )}
                                        )},
                                })
                        },
                        {
                         "streamSettings", QJsonObject::fromVariantMap(
                                {
                                        {"network",         "tcp"},
                                        {"security",        "reality"},
                                        {"realitySettings", QJsonObject::fromVariantMap(
                                                {
                                                        {"fingerprint", vlessData.realityFingerprint},
                                                        {"serverName",  vlessData.realityServerName},
                                                        {"publicKey",   vlessData.realityPublicKey},
                                                })},
                                })
                        }
                }
        );
    }
    if (proxyType == ProxyType::Trojan) {
        return QJsonObject::fromVariantMap(
                {
                        {"tag",            tag},
                        {"protocol",       "trojan"},
                        {"settings",       QJsonObject::fromVariantMap(
                                {
                                        {"servers", QJsonArray::fromVariantList(
                                                {
                                                        QJsonObject::fromVariantMap(
                                                                {
                                                                        {"address",  trojanData.server},
                                                                        {"port",     trojanData.port.toInt()},
                                                                        {"password", trojanData.password},
                                                                }
                                                        )}
                                        )},
                                })
                        },
                        {
                         "streamSettings", QJsonObject::fromVariantMap(
                                {
                                        {"network",     "tcp"},
                                        {"security",    "tls"},
                                        {"tlsSettings", QJsonObject::fromVariantMap(
                                                {
                                                        {"serverName",           trojanData.sni},
                                                        {"allowInsecureCiphers", true},
                                                        {"allowInsecure",        true},
                                                })},
                                })
                        }
                }
        );
    }
    return {};
}

