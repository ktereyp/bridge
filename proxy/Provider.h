#ifndef BRIDGE_PROVIDER_H
#define BRIDGE_PROVIDER_H

#include <QObject>
#include <QString>
#include <utility>
#include "Proxy.h"
#include "../utils/Config.h"
#include <QTimer>

enum EncodeType {
    PlainText,
    Base64,
};

class Provider : public QObject {
Q_OBJECT
public:
    explicit Provider(ProviderData data, QObject *parent = nullptr);

    void fetchProxyList(bool loadFromNetwork = false);

    ProviderData getProviderData() { return providerData; };

    void setProviderData(ProviderData data) { this->providerData = std::move(data); };

Q_SIGNALS:

    void proxyList(QString providerUuid, QList<Proxy>);

    void proxyListError(QString providerUuid, const QString &msg);

private:

    QString loadFromFs();

    bool processData(const QString &);

    void storeData(const QString &);

private slots:

    void updateProxy();

private:
    QTimer timer;
    ProviderData providerData;
    qlonglong lastTime{};

};


#endif //BRIDGE_PROVIDER_H
