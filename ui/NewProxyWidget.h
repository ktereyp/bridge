#ifndef BRIDGE_NEWPROXYWIDGET_H
#define BRIDGE_NEWPROXYWIDGET_H

#include <QWidget>
#include "../proxy/Proxy.h"


QT_BEGIN_NAMESPACE
namespace Ui { class NewProxyWidget; }
QT_END_NAMESPACE

class NewProxyWidget : public QWidget {
Q_OBJECT

public:
    explicit NewProxyWidget(QWidget *parent = nullptr);

    ~NewProxyWidget() override;

    void setProxy(Proxy proxy);

Q_SIGNALS:

    void saved(const QString &uuid);

private slots:

    void save();

    void proxyTypeChanged(int index);

    void trojanNetworkChanged(int);

    void shadowSocksPluginChanged(int);

private:
    void showErrmsg(const QString &msg);

private:
    Ui::NewProxyWidget *ui;
    ProxyType proxyType;
    QString trojanNetworkType;
    QString shadowSocksPlugin;
};


#endif //BRIDGE_NEWPROXYWIDGET_H
