#include "NewProxyWidget.h"
#include "ui_NewProxyWidget.h"
#include "../utils/Config.h"
#include "../proxy/Proxy.h"
#include <QMessageBox>
#include <QDebug>
#include <QList>

NewProxyWidget::NewProxyWidget(QWidget *parent) :
        QWidget(parent), ui(new Ui::NewProxyWidget) {
    ui->setupUi(this);

    connect(ui->saveBtn, &QPushButton::clicked, this, &NewProxyWidget::save);
    connect(ui->proxyTypeInput, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &NewProxyWidget::proxyTypeChanged);
    connect(ui->trojanNetworkInput, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &NewProxyWidget::trojanNetworkChanged);
    connect(ui->shadowSocksPluginInput, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &NewProxyWidget::shadowSocksPluginChanged);
}

NewProxyWidget::~NewProxyWidget() {
    delete ui;
}

void NewProxyWidget::proxyTypeChanged(int index) {
    auto current = ui->proxyTypeInput->currentText();
    if (current == "Trojan") {
        ui->stackedWidget->setCurrentWidget(ui->pageTrojan);
        this->proxyType = ProxyType::Trojan;
    } else if (current == "ShadowSocks") {
        ui->stackedWidget->setCurrentWidget(ui->pageShadowSocks);
        this->proxyType = ProxyType::ShadowSocks;
    } else if (current == "ShadowSocksR") {
        ui->stackedWidget->setCurrentWidget(ui->pageShadowSocksR);
        this->proxyType = ProxyType::ShadowSocksR;
    } else if (current == "Http") {
        ui->stackedWidget->setCurrentWidget(ui->pageHttp);
        this->proxyType = ProxyType::Http;
    } else if (current == "Socks5") {
        ui->stackedWidget->setCurrentWidget(ui->pageSocks5);
        this->proxyType = ProxyType::Socks5;
    } else if (current == "Vmess") {
        ui->stackedWidget->setCurrentWidget(ui->pageVmess);
        this->proxyType = ProxyType::Vmess;
    } else if (current == "Snell") {
        ui->stackedWidget->setCurrentWidget(ui->pageSnell);
        this->proxyType = ProxyType::Snell;
    } else {
        qDebug() << "unsupported proxy type: " << current;
    }
}

void NewProxyWidget::trojanNetworkChanged(int) {
    auto current = ui->trojanNetworkInput->currentText();
    if (current == tr("default")) {
        ui->trojanNetwrokStackedWidget->setCurrentWidget(ui->pageTrojanNetworkDefault);
        this->trojanNetworkType = "";
    } else if (current == "grpc") {
        ui->trojanNetwrokStackedWidget->setCurrentWidget(ui->pageTrojanNetworkGrpc);
        this->trojanNetworkType = "grpc";
    } else if (current == "ws") {
        ui->trojanNetwrokStackedWidget->setCurrentWidget(ui->pageTrojanNetworkWebsockets);
        this->trojanNetworkType = "ws";
    }
}

void NewProxyWidget::shadowSocksPluginChanged(int) {
    auto current = ui->shadowSocksPluginInput->currentText();
    if (current == tr("default")) {
        ui->shadowSocksPluginOptWidget->setCurrentWidget(ui->pageShadowSocksDefault);
        this->shadowSocksPlugin = "";
    } else if (current == "obfs") {
        ui->shadowSocksPluginOptWidget->setCurrentWidget(ui->pageShadowSocksObfs);
        this->shadowSocksPlugin = "obfs";
    } else if (current == "v2ray-plugin") {
        ui->shadowSocksPluginOptWidget->setCurrentWidget(ui->pageShadowSocksV2ray);
        this->shadowSocksPlugin = "v2ray-plugin";
    }
}

void NewProxyWidget::save() {
    Proxy p;
    p.name = ui->nameInput->text();
    if (p.name.isEmpty()) {
        showErrmsg("name cannot be empty");
        return;
    }
    p.lastRelay = ui->lastRelayInput->isChecked();
    p.proxyType = this->proxyType;
    // trojan
    if (this->proxyType == ProxyType::Trojan) {
        p.trojanData.server = ui->trojanServerInput->text();
        p.trojanData.port = ui->trojanPortInput->text();
        p.trojanData.password = ui->trojanPasswordInput->text();
        p.trojanData.udp = ui->trojanUdpInput->isChecked();
        p.trojanData.sni = ui->trojanSniInput->text();
        p.trojanData.alpn = ui->trojanAlpnInput->text().split(",");
        p.trojanData.skipCertVerify = ui->trojanSkipCertVerifyInput->isChecked();
        p.trojanData.network = ui->trojanNetworkInput->currentText();
        if (p.trojanData.network == "default") {
            p.trojanData.network = "";
        } else if (p.trojanData.network == "grpc") {
            p.trojanData.grpcServiceName = ui->trojanGrpcServiceNameInput->text();
        } else if (p.trojanData.network == "ws") {
            p.trojanData.wsPath = ui->trojanWebsocketsPathInput->text();
            auto headers = ui->trojanWebSocketsHeadersInput->text().split(",");
            for (auto h: headers) {
                h = h.trimmed();
                if (h.isEmpty()) {
                    continue;
                }
                auto components = h.split("=");
                if (components.length() == 2) {
                    p.trojanData.wsHeaders[components[0]] = components[1];
                } else {
                    showErrmsg("header is invalid: " + h);
                    return;
                }
            }
        }
    } else if (this->proxyType == ProxyType::ShadowSocks) {
        p.shadowSocksData.server = ui->shadowSocksServerInput->text();
        p.shadowSocksData.port = ui->shadowSocksPortInput->text();
        p.shadowSocksData.cipher = ui->shadowSocksCihperInput->currentText();
        p.shadowSocksData.password = ui->shadowSocksPasswordInput->text();
        p.shadowSocksData.plugin = ui->shadowSocksPluginInput->currentText();
        if (p.shadowSocksData.plugin == "default") {
            p.shadowSocksData.plugin = "";
        } else if (p.shadowSocksData.plugin == "obfs") {
            p.shadowSocksData.pluginOptMode = ui->shadowSocksObfsModeInput->currentText();
            p.shadowSocksData.pluginOptHost = ui->shadowSocksObfsHostInput->text();
        } else if (p.shadowSocksData.plugin == "v2ray-plugin") {
            p.shadowSocksData.pluginOptMode = ui->shadowSocksV2rayPluginModeInput->currentText();
            p.shadowSocksData.pluginOptTls = ui->shadowSocksV2rayPluginTlsInput->isChecked();
            p.shadowSocksData.pluginOptSkipCertVerify = ui->shadowSocksV2rayPluginSkipCertVerifyInput->isChecked();
            p.shadowSocksData.pluginOptHost = ui->shadowSocksV2rayPluginHostInput->text();
            p.shadowSocksData.pluginOptPath = ui->shadowSocksV2rayPluginPathInput->text();
            p.shadowSocksData.pluginOptMux = ui->shadowSocksV2rayPluginMuxInput->isChecked();

            auto headers = ui->shadowSocksV2rayPluginHeadersInput->text().split(",");
            for (auto h: headers) {
                h = h.trimmed();
                if (h.isEmpty()) {
                    continue;
                }
                auto components = h.split("=");
                if (components.length() == 2) {
                    p.shadowSocksData.headers[components[0]] = components[1];
                } else {
                    showErrmsg("header is invalid: " + h);
                    return;
                }
            }
        }
    }
    auto msg = p.verifyComplete();
    if (!msg.isEmpty()) {
        showErrmsg(msg);
        return;
    }

    Config::setProxy(p);
    emit saved(p.uuid);

    close();
}

void NewProxyWidget::showErrmsg(const QString &msg) {
    QMessageBox msgBox;
    msgBox.setText(msg);
    msgBox.exec();
}

void NewProxyWidget::setProxy(Proxy p) {
    this->proxyType = p.proxyType;
    ui->nameInput->setText(p.name);
    ui->lastRelayInput->setChecked(p.lastRelay);
    if (p.proxyType == ProxyType::Trojan) {
        ui->stackedWidget->setCurrentWidget(ui->pageTrojan);
        ui->proxyTypeInput->setCurrentText("Trojan");
        this->trojanNetworkType = p.trojanData.network;
        this->shadowSocksPlugin = "";
        ui->trojanServerInput->setText(p.trojanData.server);
        ui->trojanPortInput->setText(p.trojanData.port);
        ui->trojanPasswordInput->setText(p.trojanData.password);
        ui->trojanUdpInput->setChecked(p.trojanData.udp);
        ui->trojanSniInput->setText(p.trojanData.sni);
        ui->trojanAlpnInput->setText(p.trojanData.alpn.join(","));
        ui->trojanSkipCertVerifyInput->setChecked(p.trojanData.skipCertVerify);
        if (p.trojanData.network == "default") {
            ui->trojanNetworkInput->setCurrentText("default");
            ui->trojanNetwrokStackedWidget->setCurrentWidget(ui->pageTrojanNetworkDefault);
        } else if (p.trojanData.network == "grpc") {
            ui->trojanNetworkInput->setCurrentText("grpc");
            ui->trojanGrpcServiceNameInput->setText(p.trojanData.grpcServiceName);
            ui->trojanNetwrokStackedWidget->setCurrentWidget(ui->pageTrojanNetworkGrpc);
        } else if (p.trojanData.network == "ws") {
            ui->trojanNetworkInput->setCurrentText("ws");
            ui->trojanNetwrokStackedWidget->setCurrentWidget(ui->pageTrojanNetworkWebsockets);
            ui->trojanWebsocketsPathInput->setText(p.trojanData.wsPath);
            QList<QString> headers;
            for (auto h: p.trojanData.wsHeaders.keys()) {
                headers.append(h + "=" + p.trojanData.wsHeaders[h]);
            }
            ui->trojanWebSocketsHeadersInput->setText(headers.join(","));
        }
    } else if (p.proxyType == ProxyType::ShadowSocks) {
        ui->stackedWidget->setCurrentWidget(ui->pageShadowSocks);
        ui->proxyTypeInput->setCurrentText("ShadowSocks");
        this->trojanNetworkType = "";
        this->shadowSocksPlugin = p.shadowSocksData.plugin;

        ui->shadowSocksServerInput->setText(p.shadowSocksData.server);
        ui->shadowSocksPortInput->setText(p.shadowSocksData.port);
        ui->shadowSocksCihperInput->setCurrentText(p.shadowSocksData.cipher);
        ui->shadowSocksPasswordInput->setText(p.shadowSocksData.password);
        if (p.shadowSocksData.plugin == "default") {
            ui->shadowSocksPluginInput->setCurrentText("default");
            ui->shadowSocksPluginOptWidget->setCurrentWidget(ui->pageShadowSocksDefault);
        } else if (p.shadowSocksData.plugin == "obfs") {
            ui->shadowSocksPluginInput->setCurrentText("obfs");
            ui->shadowSocksPluginOptWidget->setCurrentWidget(ui->pageShadowSocksObfs);
            ui->shadowSocksObfsModeInput->setCurrentText(p.shadowSocksData.pluginOptMode);
            ui->shadowSocksObfsHostInput->setText(p.shadowSocksData.pluginOptHost);
        } else if (p.shadowSocksData.plugin == "v2ray-plugin") {
            ui->shadowSocksPluginInput->setCurrentText("v2ray-plugin");
            ui->shadowSocksPluginOptWidget->setCurrentWidget(ui->pageShadowSocksV2ray);
            ui->shadowSocksV2rayPluginTlsInput->setChecked(p.shadowSocksData.pluginOptTls);
            ui->shadowSocksV2rayPluginSkipCertVerifyInput->setChecked(p.shadowSocksData.pluginOptSkipCertVerify);
            ui->shadowSocksV2rayPluginHostInput->setText(p.shadowSocksData.pluginOptHost);
            ui->shadowSocksV2rayPluginPathInput->setText(p.shadowSocksData.pluginOptPath);
            ui->shadowSocksV2rayPluginMuxInput->setChecked(p.shadowSocksData.pluginOptMux);

            QList<QString> headers;
            for (auto h: p.shadowSocksData.headers.keys()) {
                headers.append(h + "=" + p.shadowSocksData.headers[h]);
            }
            ui->shadowSocksV2rayPluginHeadersInput->setText(headers.join(","));
        }
    }
}

