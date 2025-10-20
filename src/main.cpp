#include "mainwindow.h"

#include <QApplication>
#include <QBluetoothDeviceInfo>
#include <QBluetoothUuid>
#include <QByteArray>
#include <QMetaType>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qRegisterMetaType<QBluetoothDeviceInfo>("QBluetoothDeviceInfo");
    qRegisterMetaType<QBluetoothUuid>("QBluetoothUuid");
    qRegisterMetaType<QByteArray>("QByteArray");

    MainWindow window;
    window.show();

    return app.exec();
}
