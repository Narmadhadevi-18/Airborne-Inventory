#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QFile styleFile("C:/Users/dell/Downloads/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        app.setStyleSheet(stream.readAll());
        styleFile.close();
    }

    MainWindow window;
    window.show();
    return app.exec();
}
