#ifndef BRIDGE_PROVIDEREDITORWIDGET_H
#define BRIDGE_PROVIDEREDITORWIDGET_H

#include <QWidget>
#include "../utils/Config.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ProviderEditorWidget; }
QT_END_NAMESPACE

class ProviderEditorWidget : public QWidget {
Q_OBJECT

public:
    explicit ProviderEditorWidget(QWidget *parent = nullptr);

    ~ProviderEditorWidget() override;

    void setProvider(const ProviderData &provider);

Q_SIGNALS:

    void providerEditFinish(const ProviderData &provider);

private slots:

    void save();

private:
    Ui::ProviderEditorWidget *ui;
    ProviderData providerData;
};


#endif //BRIDGE_PROVIDEREDITORWIDGET_H
