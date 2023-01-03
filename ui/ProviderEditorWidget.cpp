#include "ProviderEditorWidget.h"
#include "ui_ProviderEditorWidget.h"
#include "../utils/Config.h"
#include "../proxy/Provider.h"

ProviderEditorWidget::ProviderEditorWidget(QWidget *parent) :
        QWidget(parent), ui(new Ui::ProviderEditorWidget) {
    ui->setupUi(this);
    connect(ui->saveBtn, &QPushButton::clicked, this, &ProviderEditorWidget::save);
}

ProviderEditorWidget::~ProviderEditorWidget() {
    delete ui;
}

void ProviderEditorWidget::save() {
    this->providerData.name = ui->providerNameInput->text();
    this->providerData.url = ui->subscribeInput->text();
    auto proxyType = ui->proxyTypeInput->currentText();
    if (proxyType == "Trojan") {
        this->providerData.proxyType = ProxyType::Trojan;
    }
    Config::setProvider(this->providerData);

    close();
}

void ProviderEditorWidget::setProvider(const ProviderData &provider) {
    this->providerData = provider;
    ui->providerNameInput->setText(provider.name);
    ui->subscribeInput->setText(provider.url);
    if (provider.proxyType == ProxyType::Trojan) {
        ui->proxyTypeInput->setCurrentText("Trojan");
    }
}
