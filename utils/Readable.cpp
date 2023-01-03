#include "Readable.h"

QString Readable::bytes(int d) {
    const int _kb = 1024;
    const int _mb = _kb * 1024;
    const int _gb = _mb * 1024;
    if (d >= _gb) {
        return QString::asprintf("%.2fGB", (double) (d) / ((double(_gb))));
    }
    if (d >= _mb) {
        return QString::asprintf("%.2fMB", (double) (d) / ((double(_mb))));
    }
    if (d >= _kb) {
        return QString::asprintf("%.2fKB", (double) (d) / ((double(_kb))));
    }
    return QString("%1B").arg(d);
}
