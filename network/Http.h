#ifndef BRIDGE_HTTP_H
#define BRIDGE_HTTP_H

#include <QNetworkAccessManager>
#include <QNetworkReply>

class Http {

public:
    void get(const QString &url, const std::function<void(QByteArray &)> &slot);

    void get(const QString &url,
             const std::function<void(QByteArray &)> &read,
             const std::function<void(QString &)> &finished
    );

private:
    static QNetworkAccessManager networkAccessManager;
};


#endif //BRIDGE_HTTP_H
