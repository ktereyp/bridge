#ifndef BRIDGE_HTTP_H
#define BRIDGE_HTTP_H

#include <QNetworkAccessManager>
#include <QNetworkReply>

class HttpError {
public:
    QNetworkReply::NetworkError err;
    QString msg;
};

class Http {

public:
    static void get(const QString &url, const std::function<void(QByteArray &, const HttpError &err)> &callback);

    static void get(const QString &url,
                    const std::function<void(QByteArray &)> &read,
                    const std::function<void(QString &)> &finished
    );

    static void get(QNetworkAccessManager &networkAccessManager,
                    const QString &url,
                    const std::function<void(QByteArray &, const HttpError &err)> &slot);

    static void get(QNetworkAccessManager &networkAccessManager,
                    const QString &url,
                    const std::function<void(QByteArray &)> &read,
                    const std::function<void(QString &)> &finished
    );

private:
    static QNetworkAccessManager _networkAccessManager;
};


#endif //BRIDGE_HTTP_H
