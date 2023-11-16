#include "ProxyCmd.h"
#include <QDebug>
#include <QFile>
#include "../network/Http.h"
#include <QTimer>
#include <QNetworkProxy>
#include "../proxy/IpInfo.h"
#include "proxy/v2ray_stats_grpc.pb.h"
#include "proxy/v2ray_stats_grpc.grpc.pb.h"
#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>
#include <QThread>

using namespace xray::app::stats;

ProxyCmd::ProxyCmd(QObject *parent) :
        QObject(parent),
        process(nullptr) {
};

void ProxyCmd::run() {
    qDebug() << "starting run proxy cmd";
    if (!this->process.isNull()) {
        auto state = this->process->state();
        if (state == QProcess::ProcessState::NotRunning) {
            emit finished(0, QProcess::ExitStatus::NormalExit);
        } else {
            this->needRestart = true;
            this->process->kill();
            return;
        }
    }

    this->process.reset(new QProcess(this));
    connect(this->process.get(), &QProcess::started,
            this, &ProxyCmd::processStart);
    connect(this->process.get(), qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this, &ProxyCmd::processFinished);
    connect(this->process.get(), &QProcess::errorOccurred,
            this, &ProxyCmd::errorOccurred);

    connect(this->process.get(), &QProcess::readyReadStandardOutput,
            this, &ProxyCmd::readStdout);
    connect(this->process.get(), &QProcess::readyReadStandardError,
            this, &ProxyCmd::readStderr);

    QStringList args{"run", "-c", this->cmdConfigFile};
    auto cmdBinary = Config::get(Config::KEY_PROXY_CMD_BINARY);
    this->process->start(cmdBinary, args);
}

void ProxyCmd::stop() {
    if (!this->process.isNull()) {
        this->process->kill();
    }
}

bool ProxyCmd::setProxy(Proxy proxy, Proxy lastRelay) {
    auto proxyCmdConfig = Config::getProxyCmdConfig();
    this->cmdConfigData = proxyCmdConfig;
    QJsonObject json;
    // log
    {
        json["log"] = QJsonObject::fromVariantMap(
                {
                        {"loglevel", proxyCmdConfig.logLevel},
                });
    }
    // stats
    {
        json["stats"] = QJsonObject::fromVariantMap({});
    }
    // api
    {
        json["api"] = QJsonObject::fromVariantMap(
                {
                        {"tag",      "api"},
                        {"services", QJsonArray::fromVariantList(
                                {
                                        "HandlerService",
                                        "LoggerService",
                                        "StatsService"
                                })},
                }
        );
    }
    // routing
    {
        QJsonArray rules;
        rules.push_back(QJsonObject::fromVariantMap({
                                                            {"type",        "field"},
                                                            {"outboundTag", "api"},
                                                            {"inboundTag",  QJsonArray::fromStringList({"api"})},
                                                    }));
        rules.push_back(QJsonObject::fromVariantMap({
                                                            {"type",        "field"},
                                                            {"outboundTag", "direct"},
                                                            {"domain",      QJsonArray::fromStringList({"geosite:cn"})},
                                                    }));
        rules.push_back(QJsonObject::fromVariantMap({
                                                            {"type",        "field"},
                                                            {"outboundTag", "direct"},
                                                            {"ip",          QJsonArray::fromStringList({
                                                                                                               "geoip:cn",
                                                                                                               "geoip:private"
                                                                                                       })},
                                                    }));
        QJsonObject settings;
        settings["rules"] = rules;
        QJsonObject routing;
        routing["settings"] = settings;
        routing["strategy"] = "rules";
        json["routing"] = routing;
    }
    // policy
    {
        json["policy"] = QJsonObject::fromVariantMap(
                {
                        {"levels", QJsonObject::fromVariantMap(
                                {
                                        {"0", QJsonObject::fromVariantMap(
                                                {
                                                        {"statsUserUplink",   true},
                                                        {"statsUserDownlink", true}
                                                })}
                                })
                        },
                        {"system", QJsonObject::fromVariantMap(
                                {
                                        {"statsInboundUplink",    true},
                                        {"statsInboundDownlink",  true},
                                        {"statsOutboundUplink",   true},
                                        {"statsOutboundDownlink", true}
                                })
                        }
                });
    }
    // inbounds
    {
        QString listen = "127.0.0.1";
        if (proxyCmdConfig.allowLan) {
            listen = "0.0.0.0";
        }
        json["inbounds"] = QJsonArray::fromVariantList(
                {
                        QJsonObject::fromVariantMap(
                                {
                                        {"listen",   listen},
                                        {"port",     proxyCmdConfig.listenPort},
                                        {"protocol", "http"},
                                        {"settings", QJsonObject::fromVariantMap(
                                                {
                                                        {"udp", true}
                                                }
                                        )},
                                        {"sniffing", QJsonObject::fromVariantMap(
                                                {
                                                        {"enabled",      true},
                                                        {"destOverride", QJsonArray::fromStringList(
                                                                {"http", "tls", "quic"}
                                                        )},
                                                        {"routeOnly",    true},
                                                }
                                        )}
                                }),
                        QJsonObject::fromVariantMap(
                                {
                                        {"listen",   "127.0.0.1"},
                                        {"port",     proxyCmdConfig.socksPort},
                                        {"protocol", "socks"},
                                        {"settings", QJsonObject::fromVariantMap(
                                                {
                                                        {"udp", true}
                                                }
                                        )},
                                        {"sniffing", QJsonObject::fromVariantMap(
                                                {
                                                        {"enabled",      true},
                                                        {"destOverride", QJsonArray::fromStringList(
                                                                {"http", "tls", "quic"}
                                                        )},
                                                        {"routeOnly",    true},
                                                }
                                        )}
                                }),
                        QJsonObject::fromVariantMap(
                                {
                                        {"listen",   "127.0.0.1"},
                                        {"port",     proxyCmdConfig.controllerPort},
                                        {"protocol", "dokodemo-door"},
                                        {"settings", QJsonObject::fromVariantMap(
                                                {
                                                        {"address", "127.0.0.1"}
                                                }
                                        )},
                                        {"tag",      "api"}
                                })
                });
    }
    // outbounds
    {
        QJsonArray outbounds;
        if (lastRelay.isValid()) {
            QJsonObject lastRelayJson = lastRelay.toV2rayProxy("safe");
            lastRelayJson["proxySettings"] = QJsonObject::fromVariantMap(
                    {
                            {"tag", "unsafe"}
                    });
            outbounds.push_back(lastRelayJson);
        }
        outbounds.push_back(proxy.toV2rayProxy("unsafe"));

        // freedom
        outbounds.push_back(QJsonObject::fromVariantMap(
                {
                        {"protocol", "freedom"},
                        {"settings", QJsonObject::fromVariantMap({})},
                        {"tag",      "direct"}
                }
        ));
        json["outbounds"] = outbounds;
    }

    auto proxyConfigFile = Config::getProxyConfigFilePath();

    // create yaml
    QFile configFile(proxyConfigFile);
    if (!configFile.open(QFile::OpenModeFlag::WriteOnly)) {
        qCritical() << "cannot create proxy config file " << proxyConfigFile
                    << ", err: " << configFile.errorString();
        return false;
    }

    auto data = QJsonDocument(json).toJson(QJsonDocument::JsonFormat::Indented);
    configFile.write(data);

    configFile.close();
    this->cmdConfigFile = proxyConfigFile;
    this->proxy = proxy;
    return true;
}

void ProxyCmd::errorOccurred(QProcess::ProcessError error) {
    qDebug() << "proxy cmd process error: " << error;
}

void ProxyCmd::processStart() {
    emit started();
    // setup speed query
    QTimer::singleShot(1 * 1000, this, &ProxyCmd::doQuerySpeed);

    QTimer::singleShot(2 * 1000, this, &ProxyCmd::myIpInfo);
}

void ProxyCmd::processFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    readStdout();
    readStderr();
    emit finished(exitCode, exitStatus);
    if (this->needRestart) {
        qDebug() << "restart proxy cmd";
        this->needRestart = false;
        this->run();
    }
}

void ProxyCmd::readStdout() {
    QString msg = this->process->readAllStandardOutput();
    auto msgList = msg.split("\n");
    for (auto &msgItem: msgList) {
        msgItem = msgItem.trimmed();
        if (msgItem.isEmpty()) {
            continue;
        }
        int pos;
        if ((pos = msgItem.indexOf("[Warning]")) > 0) {
            msgItem = msgItem.left(pos) + QString("<span style=\"color:brown;white-space:pre\">%1</span>").
                    arg(msgItem.midRef(pos));
            emit cmdStdout(msgItem);
            continue;
        } else if ((pos = msgItem.indexOf("[Error]")) > 0) {
            msgItem = msgItem.left(pos) + QString("<span style=\"color:red;white-space:pre\">%1</span>").
                    arg(msgItem.midRef(pos));
            emit cmdStdout(msgItem);
            continue;
        }
        emit cmdStdout(msgItem);
    }
}

void ProxyCmd::readStderr() {
    QString msg("<span style=\"color:red;white-space:pre\">");
    msg += this->process->readAllStandardError();
    msg += "</span>";
    emit cmdStderr(msg);
}

void ProxyCmd::doQuerySpeed() {
    if (!this->process.isNull() && this->process->state() != QProcess::ProcessState::NotRunning) {
        QThread::create([this]() {
            auto server = QString("127.0.0.1:%1").arg(this->cmdConfigData.controllerPort);
            auto channel = grpc::CreateChannel(server.toStdString(), grpc::InsecureChannelCredentials());
            std::unique_ptr<command::StatsService::Stub> stub = command::StatsService::NewStub(channel);

            while (!this->process.isNull() && this->process->state() != QProcess::ProcessState::NotRunning) {
                grpc::ClientContext context;
                command::QueryStatsRequest request;
                command::QueryStatsResponse response;
                grpc::Status status = stub->QueryStats(&context, request, &response);
                if (status.ok()) {
                    qint64 up = 0;
                    qint64 down = 0;
                    for (auto stats: response.stat()) {
                        if (stats.name() == "outbound>>>unsafe>>>traffic>>>uplink" ||
                            stats.name() == "outbound>>>direct>>>traffic>>>uplink") {
                            up += stats.value();
                        } else if (stats.name() == "outbound>>>unsafe>>>traffic>>>downlink" ||
                                   stats.name() == "outbound>>>direct>>>traffic>>>downlink") {
                            down += stats.value();
                        }
                    }
                    emit cmdNetworkSpeed(up, down);
                } else {
                    emit cmdStderr(QString("grpc error: %1").arg(status.error_code()));
                    // rechecking
                    QTimer::singleShot(1000, this, &ProxyCmd::doQuerySpeed);
                    break;
                }
                sleep(1);
            }
        })->start();
    }
}

void ProxyCmd::myIpInfo() {
    if (process.isNull()) {
        return;
    }
    if (process->state() == QProcess::ProcessState::NotRunning) {
        return;
    }

    networkAccessManager.setProxy(QNetworkProxy(
            QNetworkProxy::ProxyType::HttpProxy,
            "127.0.0.1",
            this->cmdConfigData.listenPort)
    );
    Http::get(networkAccessManager, "https://ipinfo.io",
              [this](const QByteArray &msg, const HttpError &err) {
                  if (err.err != QNetworkReply::NoError) {
                      qWarning() << "cannot fetch ip info: " << err.msg;
                      emit ipInfoUpdate(IpInfo{}, err.msg);
                      QTimer::singleShot(20 * 1000, this, &ProxyCmd::myIpInfo);
                      return;
                  }
                  auto info = IpInfo::from(msg);
                  emit ipInfoUpdate(info, err.msg);

                  QTimer::singleShot(3600 * 1000, this, &ProxyCmd::myIpInfo);
              });
}
