#include <QObject>
#include <QtTest/QTest>
#include "../../utils/Config.h"

class TestConfig : public QObject {
Q_OBJECT
private slots:

    void config();
};

void TestConfig::config() {
    QTemporaryDir tmp;
    Config::setConfigDir(tmp.path());
    Config::set("myGroup", "myKey", "myValue");
    auto v = Config::get("myGroup", "myKey");
    QCOMPARE(v, "myValue");
}

QTEST_MAIN(TestConfig)

#include "config.moc"
