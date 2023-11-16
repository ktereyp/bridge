#ifndef BRIDGE_READABLE_H
#define BRIDGE_READABLE_H

#include <QString>

class Readable {
public:
    static QString bytes(qint64 d);
};

#endif //BRIDGE_READABLE_H
