#ifndef BRIDGE_CLASH_H
#define BRIDGE_CLASH_H

#include <QObject>
#include <QScopedPointer>
#include <QProcess>
#include "../proxy/Proxy.h"

class Clash : public QObject {
Q_OBJECT
public:
    explicit Clash(QObject *parent = nullptr) :
            QObject(parent),
            process(nullptr) {};

    bool setProxy(Proxy proxy, Proxy lastRelay);

    Proxy getProxy() { return this->proxy; }

    void run();

    void stop();

Q_SIGNALS:

    void clashStdout(const QString &msg);

    void clashStderr(const QString &msg);

    void started();

    void finished(int exitCode, QProcess::ExitStatus exitStatus);

    void clashSpeed(int up, int down);

private slots:

    void readStdout();

    void readStderr();

    void errorOccurred(QProcess::ProcessError error);

    void processStart();

    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void doQuerySpeed();

private:
    QScopedPointer<QProcess> process;

    QString clashConfigFile;
    Proxy proxy;
    bool needRestart = {};

};


#endif //BRIDGE_CLASH_H
