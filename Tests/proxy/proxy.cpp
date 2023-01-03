#include <QObject>
#include <QtTest/QTest>
#include "../../proxy/Proxy.h"
#include <QDebug>

class TestProxy : public QObject {
Q_OBJECT
private slots:

    void parseTrojan();
};

void TestProxy::parseTrojan() {
    QString s = "trojan://abc@a.b.com:50037?sni=a.b.com#trojan";
    Proxy p = Proxy::trojan(s);
    QCOMPARE(p.trojanData.server, QString("a.b.com"));
    QCOMPARE(p.trojanData.password, QString("abc"));
    QCOMPARE(p.trojanData.sni, QString("a.b.com"));
    QCOMPARE(p.name,
             QString("trojan"));
}

QTEST_MAIN(TestProxy)

#include "proxy.moc"
