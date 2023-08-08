#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "../proxy/Provider.h"
#include <QFile>
#include <QDebug>
#include <QSettings>
#include <utility>
#include "../utils/Config.h"
#include "ProviderEditorWidget.h"
#include "NewProxyWidget.h"
#include "../proxy/Clash.h"
#include <QMessageBox>
#include "../utils/Readable.h"
#include "SettingsWidget.h"

const int ROLE_PROXY_DATA = Qt::UserRole;
const int ROLE_CONNECTING = Qt::UserRole + 1;
const int ROLE_CONNECTED = Qt::UserRole + 2;
const int ROLE_PROVIDER_UUID = Qt::UserRole + 3;

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->logView->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);
    ui->logView->setReadOnly(true);
    setWindowTitle("bridge");
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 2);
    // trayIcon
    auto *trayMenu = new QMenu(this);
    auto *showMainWindow = new QAction("show", this);
    connect(showMainWindow, &QAction::triggered, this, &MainWindow::showMainWindow);
    auto *exitMainWindow = new QAction("exit", this);
    connect(exitMainWindow, &QAction::triggered, this, &MainWindow::shutdown);
    trayMenu->addAction(showMainWindow);
    trayMenu->addAction(exitMainWindow);
    this->trayIcon = new QSystemTrayIcon(QIcon(":/assets/bridge.png"), this);
    this->trayIcon->setContextMenu(trayMenu);
    this->trayIcon->show();
    connect(this->trayIcon, &QSystemTrayIcon::activated,
            this, &MainWindow::iconActivated);

    // proxy list
    ui->proxyList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->proxyList, &QTreeWidget::customContextMenuRequested,
            this, &MainWindow::openProxyMenu);
    connect(ui->proxyList, &QTreeWidget::itemDoubleClicked,
            this, &MainWindow::proxyDoubleClicked);
    connect(ui->proxyList, &QTreeWidget::itemClicked,
            this, &MainWindow::proxyClicked);
    ui->proxyList->setHeaderHidden(true);
    // add new proxy
    connect(ui->addNewProxyBtn, &QPushButton::clicked,
            this, &MainWindow::addNewProxy);
    // clash
    this->clash.reset(new Clash);
    connect(this->clash.get(), &Clash::clashStdout,
            this, &MainWindow::clashStdout);
    connect(this->clash.get(), &Clash::clashStderr,
            this, &MainWindow::clashStderr);
    connect(this->clash.get(), &Clash::started,
            this, &MainWindow::clashStart);
    connect(this->clash.get(), &Clash::finished,
            this, &MainWindow::clashFinished);
    connect(this->clash.get(), &Clash::clashSpeed,
            this, &MainWindow::clashSpeed);
    connect(this->clash.get(), &Clash::ipInfoUpdate,
            this, &MainWindow::receiveMyIpInfo);


    // proxy editor
    newProxyWidget.reset(new NewProxyWidget());
    connect(newProxyWidget.get(), &NewProxyWidget::saved,
            this, &MainWindow::proxyEdited);


    // provider editor
    providerEditor.reset(new ProviderEditorWidget());
    connect(ui->addProviderBtn, &QPushButton::clicked,
            this, &MainWindow::newProviderEditor);
    connect(providerEditor.get(), &ProviderEditorWidget::providerEditFinish,
            this, &MainWindow::checkProvider);

    // settings
    settingsWidget.reset(new SettingsWidget());
    connect(ui->settingButton, &QPushButton::clicked,
            this, &MainWindow::openSettingsEditor);

    // clear log
    connect(ui->clearLogBtn, &QPushButton::clicked,
            this, &MainWindow::clearConnectLog);

    loadProxy();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::loadProxy() {
    auto providerDataList = Config::getProviders();
    for (auto &p: providerDataList) {
        addProvider(p);
    }

    this->proxies = Config::getProxies();
    receivedProviderProxyList(tr("Default"), this->proxies);
}

void MainWindow::addProvider(const ProviderData &p) {
    auto provider = new Provider(p);
    this->providerList.append(provider);
    connect(provider,
            &Provider::proxyList,
            this,
            &MainWindow::receivedProviderProxyList);
    connect(provider,
            &Provider::proxyListError,
            this,
            &MainWindow::receiveProxyListError);

    provider->fetchProxyList();
}

void MainWindow::receivedProviderProxyList(const QString &providerUuid, const QList<Proxy> &list) {
    QString providerName;
    if (providerUuid == tr("Default")) {
        providerName = tr("Default");
    } else {
        auto providerIt = std::find_if(this->providerList.begin(), this->providerList.end(), [providerUuid](auto i) {
            auto data = i->getProviderData();
            return data.uuid == providerUuid;
        });
        if (providerIt == this->providerList.end()) {
            qCritical() << "received proxy list from unknown provider whose uuid is " << providerUuid;
            return;
        }
        providerName = (*providerIt)->getProviderData().name;
    }

    qDebug() << "update proxy: provider: " << providerName
             << ", proxy count: " << list.size();

    log(QString("provider '%1' update: proxy count: %2").arg(providerName).arg(list.size()));

    auto rootItems = ui->proxyList->findItems(providerName, Qt::MatchFlag::MatchExactly);
    QTreeWidgetItem *rootItem;
    if (!rootItems.empty()) {
        rootItem = rootItems.value(0);
    } else {
        rootItem = new QTreeWidgetItem();
        rootItem->setText(0, providerName);
        rootItem->setData(0, ROLE_PROVIDER_UUID, providerUuid);
        ui->proxyList->addTopLevelItem(rootItem);
    }
    // last used proxy
    Proxy lastUsedProxy;
    // remove children
    for (auto i = rootItem->childCount() - 1; i >= 0; i--) {
        auto item = rootItem->child(i);
        if (item->data(0, ROLE_CONNECTED).toBool()) {
            lastUsedProxy = item->data(0, Qt::UserRole).value<Proxy>();
        }
        rootItem->removeChild(item);
    }

    QTreeWidgetItem *matchedItem = nullptr;
    for (auto proxy: list) {
        auto item = new QTreeWidgetItem();
        item->setText(0, proxy.name);
        item->setData(0, ROLE_PROXY_DATA, QVariant::fromValue(proxy));
        rootItem->addChild(item);
        if (proxy.name == lastUsedProxy.name) {
            matchedItem = item;
        }
    }
    if (matchedItem) {
        auto matchedProxy = matchedItem->data(0, Qt::UserRole).value<Proxy>();
        if (lastUsedProxy.toClashProxy("") == matchedProxy.toClashProxy("")) {
            matchedItem->setIcon(0, QIcon(":/assets/green_check.jpg"));
            matchedItem->setData(0, ROLE_CONNECTED, true);
            matchedItem->setData(0, ROLE_CONNECTING, false);
        } else {
            doConnect(matchedItem);
        }
    }
    rootItem->setExpanded(true);
}

void MainWindow::receiveProxyListError(QString providerUuid, const QString &msg) {
    log(msg, LOG::ERROR);
}

void MainWindow::newProviderEditor() {
    providerEditor->setProvider({});
    providerEditor->show();
}

void MainWindow::checkProvider(const ProviderData &data) {
    auto providerIt = std::find_if(this->providerList.begin(), this->providerList.end(), [data](auto i) {
        return i->getProviderData().uuid == data.uuid;
    });
    if (providerIt == this->providerList.end()) {
        // new
        addProvider(data);
        log("add new provider " + data.name);
        return;
    }
    auto oldData = (*providerIt)->getProviderData();
    (*providerIt)->setProviderData(data);

    auto rootItems = ui->proxyList->findItems(oldData.name, Qt::MatchFlag::MatchExactly);
    QTreeWidgetItem *rootItem = nullptr;
    for (auto item: rootItems) {
        auto uuid = item->data(0, ROLE_PROVIDER_UUID).toString();
        if (uuid == data.uuid) {
            rootItem = item;
            break;
        }
    }
    if (!rootItem) {
        log("cannot find provider with uuid " + data.uuid, LOG::ERROR);
        return;
    }

    if (oldData.name != data.name) {
        rootItem->setText(0, data.name);
        log(QString("provider name change: %1 -> %2").arg(oldData.name, data.name));
    }
    if (oldData.url != data.url) {
        (*providerIt)->fetchProxyList(true);
        log(QString("provider url changed, re-fetch data..."));
    }
}

void MainWindow::openSettingsEditor() {
    settingsWidget->show();
}

void MainWindow::addNewProxy() {
    if (newProxyWidget.isNull()) {
        newProxyWidget.reset(new NewProxyWidget());
    }
    newProxyWidget->show();
}

void MainWindow::editProxy(Proxy p) {
    if (newProxyWidget.isNull()) {
        newProxyWidget.reset(new NewProxyWidget());
    }
    newProxyWidget->setProxy(std::move(p));
    newProxyWidget->show();
}

void MainWindow::proxyEdited(const QString &uuid) {
    QList<Proxy> list = Config::getProxies();
    this->proxies = list;
    receivedProviderProxyList("Default", this->proxies);
}

void MainWindow::openProxyMenu(const QPoint &pos) {
    auto item = ui->proxyList->itemAt(pos);
    // provider group
    if (!item->data(0, ROLE_PROVIDER_UUID).toString().isEmpty()) {
        if (item->text(0) == tr("Default")) {
            return;
        }
        auto actionEdit = new QAction(tr("edit"), this);
        actionEdit->setData(pos);
        connect(actionEdit, &QAction::triggered,
                this, &MainWindow::menuOpenProviderEditor);

        auto actionUpdate = new QAction(tr("update"), this);
        actionEdit->setData(pos);
        connect(actionUpdate, &QAction::triggered,
                this, &MainWindow::menuUpdateProxyList);

        auto actionDelete = new QAction(tr("delete"), this);
        actionDelete->setData(pos);
        connect(actionDelete, &QAction::triggered,
                this, &MainWindow::menuDeleteProvider);

        QMenu menu(ui->proxyList);
        menu.addAction(actionEdit);
        menu.addAction(actionUpdate);
        menu.addAction(actionDelete);
        menu.exec(ui->proxyList->mapToGlobal(pos));
        return;
    }
    auto proxy = item->data(0, Qt::UserRole).value<Proxy>();
    if (!proxy.isValid()) {
        return;
    }
    auto parentItem = item->parent();
    if (parentItem->text(0) == tr("Default")) {
        auto actionEdit = new QAction(tr("edit"), this);
        actionEdit->setData(pos);
        connect(actionEdit, &QAction::triggered,
                this, &MainWindow::menuEdit);

        QMenu menu(ui->proxyList);
        menu.addAction(actionEdit);
        menu.exec(ui->proxyList->mapToGlobal(pos));
        return;
    }
    auto actionConnect = new QAction(tr("connect"), this);
    actionConnect->setData(pos);
    connect(actionConnect, &QAction::triggered,
            this, &MainWindow::menuConnect);

    QMenu menu(ui->proxyList);
    menu.addAction(actionConnect);
    menu.exec(ui->proxyList->mapToGlobal(pos));
};

void MainWindow::proxyDoubleClicked(QTreeWidgetItem *item, int column) {
    auto proxy = item->data(0, Qt::UserRole).value<Proxy>();
    if (!proxy.isValid()) {
        return;
    }
    doConnect(item);
}

void MainWindow::proxyClicked(QTreeWidgetItem *item, int column) {
    auto proxy = item->data(0, Qt::UserRole).value<Proxy>();
    if (!proxy.isValid()) {
        return;
    }
    this->ui->logView->appendHtml("Proxy: " + proxy.info());
}

void MainWindow::menuConnect() {
    auto *action = qobject_cast<QAction *>(sender());
    auto pos = action->data().toPoint();
    auto item = ui->proxyList->itemAt(pos);
    doConnect(item);
}

void MainWindow::menuEdit() {
    auto *action = qobject_cast<QAction *>(sender());
    auto pos = action->data().toPoint();
    auto item = ui->proxyList->itemAt(pos);
    auto proxy = item->data(0, Qt::UserRole).value<Proxy>();
    editProxy(proxy);
}

void MainWindow::menuOpenProviderEditor() {
    auto *action = qobject_cast<QAction *>(sender());
    auto pos = action->data().toPoint();
    auto item = ui->proxyList->itemAt(pos);
    auto providerUUid = item->data(0, ROLE_PROVIDER_UUID).toString();
    auto providerIt = std::find_if(this->providerList.begin(), this->providerList.end(), [providerUUid](auto i) {
        return i->getProviderData().uuid == providerUUid;
    });
    if (providerIt != this->providerList.end()) {
        this->providerEditor->setProvider((*providerIt)->getProviderData());
        providerEditor->show();
    }
}

void MainWindow::menuUpdateProxyList() {
    auto *action = qobject_cast<QAction *>(sender());
    auto pos = action->data().toPoint();
    auto item = ui->proxyList->itemAt(pos);
    auto providerUUid = item->data(0, ROLE_PROVIDER_UUID).toString();
    auto providerIt = std::find_if(this->providerList.begin(), this->providerList.end(), [providerUUid](auto i) {
        return i->getProviderData().uuid == providerUUid;
    });
    if (providerIt != this->providerList.end()) {
        (*providerIt)->fetchProxyList(true);
    }
}

void MainWindow::menuDeleteProvider() {
    auto *action = qobject_cast<QAction *>(sender());
    auto pos = action->data().toPoint();
    auto rootItem = ui->proxyList->itemAt(pos);
    auto providerUUid = rootItem->data(0, ROLE_PROVIDER_UUID).toString();
    auto providerIt = std::find_if(this->providerList.begin(), this->providerList.end(), [providerUUid](auto i) {
        return i->getProviderData().uuid == providerUUid;
    });
    if (providerIt != this->providerList.end()) {
        auto data = (*providerIt)->getProviderData();
        auto choose = QMessageBox::warning(this, "Delete provider", "Do your want to delete " + data.name + "?");
        if (choose == QMessageBox::StandardButton::Ok) {
            Config::deleteProvider(data.uuid);
            qInfo() << "provider " + data.name + " has been deleted";
            this->providerList.removeOne(*providerIt);
            for (auto i = rootItem->childCount() - 1; i >= 0; i--) {
                rootItem->removeChild(rootItem->child(i));
            }
            delete rootItem;
        }
    }
}

void MainWindow::doConnect(QTreeWidgetItem *item) {
    auto proxy = item->data(0, Qt::UserRole).value<Proxy>();

    qDebug() << "do connect " << proxy.info();
    ui->logView->appendPlainText("connecting to: " + proxy.info());

    Proxy lastRelay;
    for (auto &p: this->proxies) {
        if (p.lastRelay) {
            lastRelay = p;
            break;
        }
    }

    if (!this->clash->setProxy(proxy, lastRelay)) {
        qDebug() << "cannot set proxy";
        return;
    }
    item->setData(0, ROLE_CONNECTING, true);
    this->clash->run();
}

void MainWindow::clashStart() {
    QString msg = "<span style=\"color:green;white-space:pre\">"
                  +
                  QString("clash is running")
                  +
                  "</span>";
    log(msg);
    // remove last connected icon
    auto topLevelItemCount = ui->proxyList->topLevelItemCount();
    for (auto i = 0; i < topLevelItemCount; i++) {
        auto item = ui->proxyList->topLevelItem(i);
        for (auto ci = 0; ci < item->childCount(); ci++) {
            auto proxyItem = item->child(ci);
            if (proxyItem->data(0, ROLE_CONNECTED).toBool()) {
                proxyItem->setIcon(0, QIcon());
                proxyItem->setData(0, ROLE_CONNECTED, false);
            } else if (proxyItem->data(0, ROLE_CONNECTING).toBool()) {
                proxyItem->setIcon(0, QIcon(":/assets/green_check.jpg"));
                proxyItem->setData(0, ROLE_CONNECTED, true);
                proxyItem->setData(0, ROLE_CONNECTING, false);
            }
        }
    }
}

void MainWindow::clashFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitStatus == QProcess::ExitStatus::CrashExit) {
        QString msg = "<span style=\"color:red;white-space:pre\">"
                      +
                      QString("clash crashed: exit code: %1").arg(exitCode)
                      +
                      "</span>";
        clashStderr(msg);
    } else {
        QString msg = "<span style=\"color:red;white-space:pre\">"
                      +
                      QString("clash exited: exit code: %1").arg(exitCode)
                      +
                      "</span>";
        clashStderr(msg);
    }

    auto topLevelItemCount = ui->proxyList->topLevelItemCount();
    for (auto i = 0; i < topLevelItemCount; i++) {
        auto item = ui->proxyList->topLevelItem(i);
        for (auto ci = 0; ci < item->childCount(); ci++) {
            auto proxyItem = item->child(ci);
            if (proxyItem->data(0, ROLE_CONNECTED).toBool()) {
                proxyItem->setIcon(0, QIcon());
                return;
            }
        }
    }
}

void MainWindow::clashStdout(const QString &msg) {
    this->ui->logView->appendHtml(msg);
}

void MainWindow::clashStderr(const QString &msg) {
    this->ui->logView->appendHtml(msg);
}

void MainWindow::clashSpeed(int up, int down) {
    auto upSpeed = QString("%1 ↑").arg(Readable::bytes(up));
    auto downSpeed = QString("%2 ↓").arg(Readable::bytes(down));

    if (this->ipInfo.ip.isEmpty()) {
        ui->statusbar->showMessage(upSpeed + " " + downSpeed);
    } else {
        ui->statusbar->showMessage(
                this->ipInfo.city + ", " + this->ipInfo.ip + "         " + upSpeed + " " + downSpeed);
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    event->ignore();
    hide();
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
        case QSystemTrayIcon::Trigger:
            this->show();
        case QSystemTrayIcon::DoubleClick:
            this->show();
            break;
        case QSystemTrayIcon::MiddleClick:
            this->show();
            break;
        default:;
    }
}

void MainWindow::showMainWindow() {
    qDebug() << "show main window";
    this->show();
}

void MainWindow::shutdown() {
    qDebug() << "shutdown";
    this->close();
    QApplication::exit(0);
}

void MainWindow::receiveMyIpInfo(const IpInfo &info, const QString &msg) {
    this->ipInfo = info;
    if (!msg.isEmpty()) {
        log(msg, LOG::ERROR);
    }
}

void MainWindow::clearConnectLog() {
    ui->logView->clear();
}

void MainWindow::log(const QString &msg, LOG level) {
    if (level == LOG::ERROR) {
        QString m = "<span style=\"color:red;white-space:pre\">"
                    +
                    msg
                    +
                    "</span>";
        clashStderr(msg);
        ui->logView->appendHtml(m);
    } else {
        ui->logView->appendHtml(msg);
    }
}
