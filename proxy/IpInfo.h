#ifndef BRIDGE_IPINFO_H
#define BRIDGE_IPINFO_H

#include <QString>

class IpInfo {
public :
    static IpInfo from(const QByteArray&msg);
public:
    QString ip;
    QString city;
    QString region;
    QString country;
    QString loc;
    QString org;
    QString timezone;
};


#endif //BRIDGE_IPINFO_H
