#include "Clash.h"
#include <QDebug>
#include "../utils/Config.h"
#include <QFile>
#include "../network/Http.h"
#include <QTimer>

void Clash::run() {
    qDebug() << "starting run clash";
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
            this, &Clash::processStart);
    connect(this->process.get(), qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this, &Clash::processFinished);
    connect(this->process.get(), &QProcess::errorOccurred,
            this, &Clash::errorOccurred);

    connect(this->process.get(), &QProcess::readyReadStandardOutput,
            this, &Clash::readStdout);
    connect(this->process.get(), &QProcess::readyReadStandardError,
            this, &Clash::readStderr);

    QStringList args{"-f", this->clashConfigFile};
    auto clashBinary = Config::get(Config::CLASH_BINARY_KEY);
    this->process->start(clashBinary, args);
}

void Clash::stop() {
    if (!this->process.isNull()) {
        this->process->kill();
    }
}

bool Clash::setProxy(Proxy proxy, Proxy lastRelay) {
    auto clashConfig = Config::getClashConfigFile();
    // create yaml
    QFile configFile(clashConfig);
    if (!configFile.open(QFile::OpenModeFlag::WriteOnly)) {
        qCritical() << "cannot create clash config file " << clashConfig
                    << ", err: " << configFile.errorString();
        return false;
    }
    configFile.write("port: 8889\n");
    configFile.write("socks-port: 1099\n");
    configFile.write("allow-lan: true\n");
    configFile.write("bind-address: '*'\n");
    configFile.write("mode: rule\n");
    configFile.write("log-level: info\n");
    configFile.write("ipv6: false\n");
    configFile.write("external-controller: 127.0.0.1:9091\n");

    auto proxyStr = QString("proxies:\n")
                    + proxy.toClashProxy("trojan");
    auto proxiesList = QString("") +
                       "      - trojan\n";
    if (lastRelay.isValid()) {
        proxyStr += lastRelay.toClashProxy("my-private-ss");
        proxiesList += "      - my-private-ss\n";
    }
    configFile.write(proxyStr.toUtf8());

    configFile.write(""
                     "proxy-groups:\n"
                     "  - name: \"relay\"\n"
                     "    type: relay\n"
                     "    proxies:\n"
    );
    configFile.write(proxiesList.toUtf8());

    configFile.write(""
                     "rules:\n"
                     "  - DOMAIN-SUFFIX,ad.com,REJECT\n"
                     "  - IP-CIDR,192.168.0.0/16,DIRECT\n"
                     "  - IP-CIDR,127.0.0.0/8,DIRECT\n"
                     "  - IP-CIDR,10.0.0.0/8,DIRECT\n"
                     "  - GEOIP,CN,DIRECT\n"
                     "  - MATCH,relay\n"
    );
    configFile.close();
    this->clashConfigFile = clashConfig;
    this->proxy = proxy;
    return true;
}

void Clash::errorOccurred(QProcess::ProcessError error) {
    qDebug() << "clash process error: " << error;
}

void Clash::processStart() {
    emit started();
    // setup speed query
    doQuerySpeed();
}

void Clash::processFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    emit finished(exitCode, exitStatus);
    if (this->needRestart) {
        qDebug() << "restart clash";
        this->needRestart = false;
        this->run();
    }
}

void Clash::readStdout() {
    // time="2022-12-31T16:56:59+08:00" level=error msg="xxxxx"
    QString msg = this->process->readAllStandardOutput();
    auto msgList = msg.split("\n");
    for (auto &msgItem: msgList) {
        if (msgItem.isEmpty()) {
            continue;
        }
        QString finalMsg;
        QString appendMsg;
        // time
        auto pos = msgItem.indexOf("time=");
        if (pos >= 0) {
            pos = pos + QString("time=\"").length();
            auto end = msgItem.indexOf("\"", pos);
            auto time = msgItem.mid(pos, end - pos);
            if ((pos = time.indexOf("+")) > 0) {
                time = time.left(pos);
            }
            time.replace("T", " ");
            finalMsg = "[" + time + "] ";
        }
        // info
        if ((pos = msgItem.indexOf("level=")) > 0) {
            pos = pos + QString("level=").length();
            auto p2 = msgItem.indexOf(" ", pos);
            auto level = msgItem.mid(pos, p2 - pos);
            if (level == "error") {
                finalMsg += "<span style=\"color:red;white-space:pre\">";
                appendMsg = "</span>";
            }

            // msg
            if ((pos = msgItem.indexOf("msg=\"")) > 0) {
                pos = pos + QString("msg=\"").length();
                finalMsg += msgItem.midRef(pos, msgItem.length() - pos - 1);
            }
            finalMsg += appendMsg;
        }

        emit clashStdout(finalMsg);
    }
}

void Clash::readStderr() {
    QString msg("<span style=\"color:red;white-space:pre\">");
    msg += this->process->readAllStandardError();
    msg += "</span>";
    emit clashStderr(msg);
}

void Clash::doQuerySpeed() {
    if (!this->process.isNull()) {
        if (this->process->state() != QProcess::ProcessState::NotRunning) {
            class Http http;
            http.get("http://127.0.0.1:9091/traffic",
                     [this](QByteArray &data) {
                         auto doc = QJsonDocument::fromJson(data);
                         if (doc.isObject()) {
                             auto json = doc.object();
                             auto up = json["up"].toInt();
                             auto down = json["down"].toInt();
                             emit clashSpeed(up, down);
                         }
                     },
                     [this](QString &msg) {
                         qDebug() << "clash traffic query ends: " << msg;
                         QTimer::singleShot(1000, this, &Clash::doQuerySpeed);
                     });
        }
    }
}
