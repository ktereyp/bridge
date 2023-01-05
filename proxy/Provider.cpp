#include "Provider.h"
#include "../network/Http.h"
#include "../utils/Config.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

Provider::Provider(ProviderData data, QObject *parent) :
        QObject(parent),
        providerData(std::move(data)),
        timer(this) {

    // update period, at least 10 minutes
    qDebug() << "update period of provider " << providerData.name
             << ": " << providerData.updatePeriod << " days";
    auto period = (long) (providerData.updatePeriod * 3600 * 24);
    if (period < 60 * 10) {
        period = 60 * 10;
    }
    timer.setInterval(period * 1000);
    timer.start();
    connect(&timer, &QTimer::timeout, this, &Provider::updateProxy);
}

void Provider::fetchProxyList(bool loadFromNetwork) {
    QString data;
    if (!loadFromNetwork) {
        data = loadFromFs();
    }
    if (!data.isEmpty()) {
        processData(data);
        auto elapseSinceLastUpdate = QDateTime::currentSecsSinceEpoch() - this->lastTime;
        auto updatePeriod = (qint64) (this->providerData.updatePeriod * 24 * 3600);
        if (elapseSinceLastUpdate < updatePeriod) {
            // do nothing
            return;
        }
    }

    qDebug() << QString("load data of %1 from network").arg(this->providerData.name);

    Http::get(this->providerData.url, [this](const QByteArray &bytes, const HttpError &err) {
        if (err.err != QNetworkReply::NoError) {
            qDebug() << QString("cannot load data of %1 from network").arg(this->providerData.name)
                     << ": " << this->providerData.url
                     << ", error: " << err.msg;
            emit proxyListError(this->providerData.uuid, err.msg);
            return;
        }

        if (bytes.isEmpty()) {
            qDebug() << QString("cannot load data of %1 from network").arg(this->providerData.name)
                     << ": " << this->providerData.url;
            emit proxyListError(this->providerData.uuid, "no proxy found");
            return;
        }
        // store
        QString data(bytes);
        if (processData(data)) {
            storeData(data);
        }
    });
}

QString Provider::loadFromFs() {
    auto cacheFile = Config::getProviderCacheFile(this->providerData.name);
    QFile file(cacheFile);
    if (!file.exists()) {
        qDebug() << "no cache found for provider " << this->providerData.name
                 << ": " << cacheFile;
        return {};
    }
    if (!file.open(QFile::OpenModeFlag::ReadOnly)) {
        qWarning() <<
                   QString("cannot open cache file %1 of provider %2")
                           .arg(cacheFile, this->providerData.name);
        return {};
    }
    auto data = file.readAll();
    auto json = QJsonDocument::fromJson(data);
    auto jsonObj = json.object();
    this->lastTime = jsonObj.value("last-time").toString().toLongLong();
    auto rawData = jsonObj.value("raw-data").toString();

    qDebug() << QString("load cache data of %1 from local fs").arg(this->providerData.name);
    return rawData;
}

bool Provider::processData(const QString &data) {
    auto decoded = QByteArray::fromBase64(data.toUtf8(), QByteArray::AbortOnBase64DecodingErrors);
    if (decoded.isEmpty()) {
        return false;
    }
    QString str(decoded);

    auto list = str.split("\n");
    QList<Proxy> out;
    for (auto &i: list) {
        if (this->providerData.proxyType == ProxyType::Trojan) {
            auto proxy = Proxy::trojan(i);
            if (proxy.isValid() > 0) {
                out.append(proxy);
            }
        }
    }
    emit proxyList(this->providerData.uuid, out);
    return !out.empty();
}

void Provider::storeData(const QString &data) {
    this->lastTime = QDateTime::currentSecsSinceEpoch();

    QJsonObject json;
    json["last-time"] = QString("%1").arg(this->lastTime);
    json["raw-data"] = data;

    auto jsonData = QJsonDocument(json).toJson();
    QFile file(Config::getProviderCacheFile(this->providerData.name));
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        qCritical() << "cannot write provider cache file " << file.fileName()
                    << ", err: " << file.errorString();
        return;
    }
    file.write(jsonData);
}

void Provider::updateProxy() {
    qInfo() << "fetch proxy list from provider " << this->providerData.name;
    fetchProxyList(true);
}
