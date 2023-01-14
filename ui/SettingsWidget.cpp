#include "SettingsWidget.h"
#include "ui_SettingsWidget.h"
#include "../utils/Config.h"
#include <QFileDialog>
#include <QProcess>
#include <QStringList>
#include <QMessageBox>

SettingsWidget::SettingsWidget(QWidget *parent) :
        QWidget(parent), ui(new Ui::SettingsWidget) {
    ui->setupUi(this);

    auto clashConfig = Config::getClashConfig();
    ui->clashBinaryFileInput->setText(clashConfig.binaryPath);
    ui->clashVerifyBtn->setEnabled(!clashConfig.binaryPath.isEmpty());
    ui->clashListenPortInput->setText(QString::number(clashConfig.listenPort));
    ui->clashControllerPortInput->setText(QString::number(clashConfig.controllerPort));
    ui->socksPortInput->setText(QString::number(clashConfig.socksPort));
    ui->bindAddressInput->setCurrentIndex(0);
    if (clashConfig.allowLan) {
        ui->allowLanInput->setCheckState(Qt::CheckState::Checked);
    }
    ui->logLevelInput->setCurrentText(clashConfig.logLevel);

    connect(ui->choseFileBtn, &QPushButton::clicked,
            this, &SettingsWidget::chooseFile);
    connect(ui->clashVerifyBtn, &QPushButton::clicked,
            this, &SettingsWidget::verifyClashBinary);
    connect(ui->saveBtn, &QPushButton::clicked,
            this, &SettingsWidget::saveConfig);
}

SettingsWidget::~SettingsWidget() {
    delete ui;
}

void SettingsWidget::chooseFile() {
    auto file = QFileDialog::getOpenFileName(this, "Choose clash binary");
    ui->clashBinaryFileInput->setText(file);
    ui->clashVerifyBtn->setEnabled(!file.isEmpty());
}

void SettingsWidget::verifyClashBinary() {
    auto file = ui->clashBinaryFileInput->text();
    auto p = new QProcess(this);
    connect(p, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            [p, this](int exitCode, QProcess::ExitStatus) {
                auto msg = p->readAll();
                if (exitCode != 0) {
                    QMessageBox::warning(this, "Invalid clash", QString(msg));
                } else if (msg.contains("Clash")) {
                    QMessageBox::information(this, "Clash", msg);
                } else {
                    QMessageBox::warning(this, "Invalid clash", QString(msg));
                }
                qDebug() << msg;
                p->deleteLater();
            });
    QStringList args{"-v"};
    p->start(file, args);
}

void SettingsWidget::saveConfig() {
    auto clashConfig = Config::getClashConfig();

    auto clashBinaryFile = ui->clashBinaryFileInput->text();
    if (clashBinaryFile.isEmpty()) {
        QMessageBox::warning(this, "Invalid Configuration", "clash binary missing");
        return;
    }
    clashConfig.binaryPath = clashBinaryFile;

    QSet<ushort> usedPorts = {};
    QString msg;
    auto setPort = [&usedPorts, &msg](ushort *port, const QString &input) -> QString {
        bool ok;
        auto v = input.toUShort(&ok);
        if (ok) {
            if (usedPorts.contains(v)) {
                return "port has been used";
            }
            *port = v;
            usedPorts.insert(v);
            return {};
        } else {
            return "invalid value";
        }
    };
    if (auto p = setPort(&clashConfig.listenPort, ui->clashListenPortInput->text()); !p.isEmpty()) {
        QMessageBox::warning(this, "Invalid Configuration", "listen port is not valid: " + msg);
    }
    if (auto p = setPort(&clashConfig.controllerPort, ui->clashControllerPortInput->text()); !p.isEmpty()) {
        QMessageBox::warning(this, "Invalid Configuration", "controller port is not valid: " + msg);
    }
    if (auto p = setPort(&clashConfig.socksPort, ui->socksPortInput->text()); !p.isEmpty()) {
        QMessageBox::warning(this, "Invalid Configuration", "socks port is not valid: " + msg);
    }
    auto bindAddress = ui->bindAddressInput->currentText();
    if (bindAddress == "all") {
        clashConfig.bindAddress = "*";
    }
    clashConfig.allowLan = ui->allowLanInput->isChecked();
    clashConfig.logLevel = ui->logLevelInput->currentText();
    Config::setClashConfig(clashConfig);

    close();
}