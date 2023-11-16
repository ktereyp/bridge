#ifndef BRIDGE_MAINWINDOW_H
#define BRIDGE_MAINWINDOW_H

#include <QMainWindow>
#include <QScopedPointer>
#include "../proxy/Provider.h"
#include "../proxy/Proxy.h"
#include "../proxy/IpInfo.h"
#include <QList>
#include <QSystemTrayIcon>
#include <QProcess>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ProviderEditorWidget;

class NewProxyWidget;

class SettingsWidget;

class ProxyCmd;

class QTreeWidgetItem;

class MainWindow : public QMainWindow {
Q_OBJECT

    enum LOG {
        INFO,
        ERROR,
    };

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private slots:

    void newProviderEditor();

    void checkProvider(const ProviderData &data);

    void openSettingsEditor();

    void addNewProxy();

    void proxyEdited(const QString &uuid);

    void openProxyMenu(const QPoint &pos);

    void proxyDoubleClicked(QTreeWidgetItem *item, int column);

    void proxyClicked(QTreeWidgetItem *item, int column);

    void menuConnect();

    void menuEdit();

    void menuOpenProviderEditor();

    void menuUpdateProxyList();

    void menuDeleteProvider();

    void receivedProviderProxyList(const QString &providerUuid, const QList<Proxy> &);

    void receiveProxyListError(QString providerUuid, const QString &msg);

    void proxyCmdStart();

    void proxyCmdFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void proxyCmdStdout(const QString &msg);

    void proxyCmdStderr(const QString &msg);

    void proxyCmdNetworkTraffic(qint64 up, qint64 down);

    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    void showMainWindow();

    void shutdown();

    void receiveMyIpInfo(const IpInfo &info, const QString &msg);

    void clearConnectLog();

private:
    void closeEvent(QCloseEvent *event);

private:
    void loadProxy();

    void addProvider(const ProviderData &provider);

    void doConnect(QTreeWidgetItem *item);

    void editProxy(Proxy p);

    void log(const QString &msg, LOG level = LOG::INFO);
private:
    struct Traffic {
        qint64 uplink;
        qint64 downlink;
        qint64 lastTime;
        qint64 uplinkRatio;
        qint64 downlinkRatio;
    };
private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;
    QList<Provider *> providerList;
    QList<Proxy> proxies;
    QScopedPointer<ProviderEditorWidget> providerEditor;
    QScopedPointer<NewProxyWidget> newProxyWidget;
    QScopedPointer<SettingsWidget> settingsWidget;
    QScopedPointer<ProxyCmd> proxyCmd;
    IpInfo ipInfo = {};
    Traffic traffic = {};
};


#endif //BRIDGE_MAINWINDOW_H
