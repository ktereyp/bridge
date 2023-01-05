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

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private slots:

    void openProviderEditor();

    void openSettingsEditor();

    void addNewProxy();

    void proxyEdited(const QString &uuid);

    void openProxyMenu(const QPoint &pos);

    void proxyDoubleClicked(QTreeWidgetItem *item, int column);

    void proxyClicked(QTreeWidgetItem *item, int column);

    void menuConnect();

    void menuEdit();

    void menuOpenProviderEditor();

    void menuDeleteProvider();

    void receivedProviderProxyList(const QString &providerUuid, const QList<Proxy> &);

    void clashStart();

    void clashFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void clashStdout(const QString &msg);

    void clashStderr(const QString &msg);

    void clashSpeed(int up, int down);

    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    void showMainWindow();

    void shutdown();

    void receiveMyIpInfo(const IpInfo &info, const QString &msg);

private:
    void closeEvent(QCloseEvent *event);

private:
    void loadProxy();

    void doConnect(QTreeWidgetItem *item);

    void editProxy(Proxy p);

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;
    QList<Provider *> providerList;
    QList<Proxy> proxies;
    QScopedPointer<ProviderEditorWidget> providerEditor;
    QScopedPointer<NewProxyWidget> newProxyWidget;
    QScopedPointer<SettingsWidget> settingsWidget;
    QScopedPointer<Clash> clash;
};


#endif //BRIDGE_MAINWINDOW_H
