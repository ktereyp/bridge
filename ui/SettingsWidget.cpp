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

    auto clashBinary = Config::get(Config::CLASH_BINARY_KEY);
    ui->clashBinaryFileInput->setText(clashBinary);
    ui->clashVerifyBtn->setEnabled(!clashBinary.isEmpty());

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
                    QMessageBox msgBox;
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
    auto file = ui->clashBinaryFileInput->text();
    Config::set(Config::CLASH_BINARY_KEY, file);
    close();
}