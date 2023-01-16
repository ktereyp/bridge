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

class Clash;

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

    void clashStart();

    void clashFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void clashStdout(const QString &msg);

    void clashStderr(const QString &msg);

    void clashSpeed(int up, int down);

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
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;
    QList<Provider *> providerList;
    QList<Proxy> proxies;
    QScopedPointer<ProviderEditorWidget> providerEditor;
    QScopedPointer<NewProxyWidget> newProxyWidget;
    QScopedPointer<SettingsWidget> settingsWidget;
    QScopedPointer<Clash> clash;
    IpInfo ipInfo = {};
};


#endif //BRIDGE_MAINWINDOW_H
