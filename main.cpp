#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Инициализация GDAL
    GDALAllRegister();

    MainWindow w;
    w.show();

    return app.exec();
}
