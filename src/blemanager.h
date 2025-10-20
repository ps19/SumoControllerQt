#pragma once

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QBluetoothUuid>
#include <QLowEnergyCharacteristic>
#include <QLowEnergyController>
#include <QLowEnergyDescriptor>
#include <QLowEnergyService>
#include <QString>

class BleManager : public QObject
{
    Q_OBJECT
public:
    explicit BleManager(QObject *parent = nullptr);

    void startDeviceDiscovery();
    void stopDeviceDiscovery();
    void connectToDevice(const QBluetoothDeviceInfo &info);
    void disconnectFromDevice();

    void setTargetUuids(const QBluetoothUuid &serviceUuid,
                        const QBluetoothUuid &txCharacteristicUuid,
                        const QBluetoothUuid &rxCharacteristicUuid);

    void sendData(const QByteArray &data);

signals:
    void deviceDiscovered(const QBluetoothDeviceInfo &info);
    void discoveryFinished();
    void controllerConnected(const QString &name);
    void controllerDisconnected();
    void errorOccurred(const QString &message);
    void serviceDiscovered(const QBluetoothUuid &uuid);
    void serviceScanDone();
    void dataReceived(const QByteArray &data);
    void logMessage(const QString &message);

private slots:
    void onDeviceDiscovered(const QBluetoothDeviceInfo &info);
    void onDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error);
    void onControllerConnected();
    void onControllerDisconnected();
    void onControllerError(QLowEnergyController::Error error);
    void onServiceDiscovered(const QBluetoothUuid &uuid);
    void onServiceScanDone();
    void onServiceStateChanged(QLowEnergyService::ServiceState state);
    void onCharacteristicChanged(const QLowEnergyCharacteristic &ch, const QByteArray &value);
    void onCharacteristicRead(const QLowEnergyCharacteristic &ch, const QByteArray &value);

private:
    void resetController();
    void setupService();

    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent = nullptr;
    QLowEnergyController *m_controller = nullptr;
    QLowEnergyService *m_service = nullptr;

    QBluetoothUuid m_serviceUuid;
    QBluetoothUuid m_txCharacteristicUuid;
    QBluetoothUuid m_rxCharacteristicUuid;

    QLowEnergyCharacteristic m_txCharacteristic;
    QLowEnergyCharacteristic m_rxCharacteristic;
};
