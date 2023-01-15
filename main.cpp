#include <QApplication>
#include <QPushButton>

#include "ui/MainWindow.h"

int main(int argc, char *argv[]) {
    qSetMessagePattern("[%{time yyyy-MM-dd h:mm:ss.zzz t} %{file}(%{line}): %{if-category}%{category}: %{endif}%{message}");
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/assets/bridge.png"));

    MainWindow window;
    window.show();

    return QApplication::exec();
}
