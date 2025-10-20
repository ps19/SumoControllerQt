#pragma once

#include <QMainWindow>
#include <QVector>

#include <QByteArray>

#include <QBluetoothDeviceInfo>
#include <QBluetoothUuid>

class BleManager;
class QListWidget;
class QPushButton;
class QTextEdit;
class QLineEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onScanDevices();
    void onStopScan();
    void onConnectToSelectedDevice();
    void onDisconnect();
    void onSendData();

    void addDeviceToList(const QBluetoothDeviceInfo &info);
    void onDiscoveryFinished();
    void onControllerConnected(const QString &name);
    void onControllerDisconnected();
    void onErrorOccurred(const QString &message);
    void onServiceDiscovered(const QBluetoothUuid &uuid);
    void onServiceScanDone();
    void onDataReceived(const QByteArray &data);
    void appendLog(const QString &message);

private:
    void setupUi();
    void updateUuidConfiguration();
    QString formatUuid(const QBluetoothUuid &uuid) const;
    void updateBuffersFromInput(const QString &text);
    void refreshReceiveDisplay();

    BleManager *m_bleManager = nullptr;

    QListWidget *m_deviceList = nullptr;
    QPushButton *m_scanButton = nullptr;
    QPushButton *m_stopScanButton = nullptr;
    QPushButton *m_connectButton = nullptr;
    QPushButton *m_disconnectButton = nullptr;
    QPushButton *m_sendButton = nullptr;

    QTextEdit *m_logView = nullptr;
    QTextEdit *m_receiveView = nullptr;
    QLineEdit *m_sendEdit = nullptr;
    QLineEdit *m_serviceUuidEdit = nullptr;
    QLineEdit *m_txUuidEdit = nullptr;
    QLineEdit *m_rxUuidEdit = nullptr;

    QVector<QBluetoothDeviceInfo> m_devices;
    QVector<quint8> m_txBuffer;
    QVector<quint8> m_rxBuffer;
};
