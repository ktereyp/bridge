#include "Http.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QObject>
#include <functional>

QNetworkAccessManager Http::networkAccessManager;

void Http::get(const QString &url, const std::function<void(QByteArray &)> &slot) {
    qDebug() << "HTTP get: " << url;
    QNetworkRequest req((QUrl(url)));

    auto reply = networkAccessManager.get(req);

    QObject::connect(reply, &QNetworkReply::finished, [reply, slot]() {
        auto all = reply->readAll();
        slot(all);
        reply->deleteLater();
    });
    QObject::connect(reply, &QNetworkReply::readyRead, [reply, slot]() {
    });
    QObject::connect(reply, &QNetworkReply::sslErrors, [reply](const QList<QSslError> &errors) {
        QString errorString;
        for (const QSslError &error: errors) {
            if (!errorString.isEmpty())
                errorString += '\n';
            errorString += error.errorString();
        }
        qDebug() << errorString;
        reply->ignoreSslErrors();
    });
}

void Http::get(const QString &url,
               const std::function<void(QByteArray &)> &read,
               const std::function<void(QString &)> &finished) {

    qDebug() << "HTTP get: " << url;
    QNetworkRequest req((QUrl(url)));

    auto reply = networkAccessManager.get(req);

    QObject::connect(reply, &QNetworkReply::finished, [reply, read, finished]() {
        auto data = reply->readAll();
        read(data);

        QString msg = reply->errorString();
        finished(msg);
        reply->deleteLater();
    });
    QObject::connect(reply, &QNetworkReply::readyRead, [reply, read]() {
        auto data = reply->readAll();
        read(data);
    });
    QObject::connect(reply, &QNetworkReply::sslErrors, [reply](const QList<QSslError> &errors) {
        QString errorString;
        for (const QSslError &error: errors) {
            if (!errorString.isEmpty())
                errorString += '\n';
            errorString += error.errorString();
        }
        qDebug() << errorString;
        reply->ignoreSslErrors();
    });
}
