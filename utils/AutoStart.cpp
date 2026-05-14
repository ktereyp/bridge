#include "AutoStart.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#ifdef Q_OS_WIN
#include <QSettings>

static const char *RUN_KEY =
    "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const char *RUN_VALUE = "bridge";
#endif

#ifdef Q_OS_LINUX
static QString autostartFilePath() {
    return QDir::homePath() + "/.config/autostart/bridge.desktop";
}
#endif

bool AutoStart::isEnabled() {
#ifdef Q_OS_WIN
    QSettings reg(RUN_KEY, QSettings::NativeFormat);
    return !reg.value(RUN_VALUE).toString().isEmpty();
#elif defined(Q_OS_LINUX)
    return QFile::exists(autostartFilePath());
#else
    return false;
#endif
}

void AutoStart::setEnabled(bool enabled) {
    auto exe = QCoreApplication::applicationFilePath();
#ifdef Q_OS_WIN
    QSettings reg(RUN_KEY, QSettings::NativeFormat);
    if (enabled) {
        auto command = QString("\"%1\"").arg(QDir::toNativeSeparators(exe));
        reg.setValue(RUN_VALUE, command);
    } else {
        reg.remove(RUN_VALUE);
    }
#elif defined(Q_OS_LINUX)
    auto path = autostartFilePath();
    if (enabled) {
        QDir().mkpath(QFileInfo(path).path());
        QFile file(path);
        if (file.open(QFile::WriteOnly | QFile::Truncate)) {
            QTextStream out(&file);
            out << "[Desktop Entry]\n"
                << "Type=Application\n"
                << "Name=bridge\n"
                << "Exec=" << exe << "\n"
                << "X-GNOME-Autostart-enabled=true\n";
        }
    } else {
        QFile::remove(path);
    }
#else
    (void) exe;
    (void) enabled;
#endif
}
