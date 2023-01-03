#ifndef BRIDGE_SETTINGSWIDGET_H
#define BRIDGE_SETTINGSWIDGET_H

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class SettingsWidget; }
QT_END_NAMESPACE

class SettingsWidget : public QWidget {
Q_OBJECT

public:
    explicit SettingsWidget(QWidget *parent = nullptr);

    ~SettingsWidget() override;

private slots:

    void chooseFile();

    void verifyClashBinary();

    void saveConfig();

private:
    Ui::SettingsWidget *ui;

};


#endif //BRIDGE_SETTINGSWIDGET_H
