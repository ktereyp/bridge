#include "IpInfo.h"
#include <QJsonDocument>
#include <QJsonObject>

IpInfo IpInfo::from(const QByteArray &msg) {
    IpInfo info;
    auto json = QJsonDocument::fromJson(msg).object();
    info.ip = json["ip"].toString();
    info.city = json["city"].toString();
    info.region = json["region"].toString();
    info.country = json["country"].toString();
    info.loc = json["loc"].toString();
    info.org = json["org"].toString();
    info.timezone = json["timezone"].toString();
    return info;
}
