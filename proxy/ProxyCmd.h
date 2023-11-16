#ifndef BRIDGE_PROXYCMD_H
#define BRIDGE_PROXYCMD_H

#include <QObject>
#include <QScopedPointer>
#include <QProcess>
#include <QNetworkAccessManager>
#include "../proxy/Proxy.h"
#include "../utils/Config.h"

class IpInfo;
class ProxyCmdConfigData;

class ProxyCmd : public QObject {
Q_OBJECT
public:
    explicit ProxyCmd(QObject *parent = nullptr);

    bool setProxy(Proxy proxy, Proxy lastRelay);

    Proxy getProxy() { return this->proxy; }

    void run();

    void stop();

Q_SIGNALS:

    void cmdStdout(const QString &msg);

    void cmdStderr(const QString &msg);

    void started();

    void finished(int exitCode, QProcess::ExitStatus exitStatus);

    void cmdNetworkSpeed(qint64 up, qint64 down);

    void ipInfoUpdate(const IpInfo &info, const QString &msg);

private slots:

    void readStdout();

    void readStderr();

    void errorOccurred(QProcess::ProcessError error);

    void processStart();

    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void doQuerySpeed();

    void myIpInfo();

private:
    QScopedPointer<QProcess> process;

    QString cmdConfigFile;
    Proxy proxy;
    ProxyCmdConfigData cmdConfigData;
    bool needRestart = {};

    QNetworkAccessManager networkAccessManager;
};


#endif //BRIDGE_PROXYCMD_H
