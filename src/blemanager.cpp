#include "blemanager.h"

#include <QBluetoothUuid>
#include <QMetaEnum>
#include <QString>

BleManager::BleManager(QObject *parent)
    : QObject(parent)
    , m_discoveryAgent(new QBluetoothDeviceDiscoveryAgent(this))
{
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BleManager::onDeviceDiscovered);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
            this, &BleManager::onDiscoveryError);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BleManager::discoveryFinished);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled,
            this, &BleManager::discoveryFinished);
}

void BleManager::startDeviceDiscovery()
{
    if (m_discoveryAgent->isActive()) {
        m_discoveryAgent->stop();
    }

    emit logMessage(tr("Rozpoczynanie wyszukiwania urządzeń BLE..."));
    m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

void BleManager::stopDeviceDiscovery()
{
    if (m_discoveryAgent->isActive()) {
        emit logMessage(tr("Zatrzymywanie wyszukiwania urządzeń."));
        m_discoveryAgent->stop();
    }
}

void BleManager::connectToDevice(const QBluetoothDeviceInfo &info)
{
    emit logMessage(tr("Łączenie z %1 (%2)...").arg(info.name(), info.address().toString()));

    resetController();

    m_controller = QLowEnergyController::createCentral(info, this);
    m_controller->setRemoteAddressType(QLowEnergyController::PublicAddress);

    connect(m_controller, &QLowEnergyController::connected,
            this, &BleManager::onControllerConnected);
    connect(m_controller, &QLowEnergyController::disconnected,
            this, &BleManager::onControllerDisconnected);
    connect(m_controller, &QLowEnergyController::serviceDiscovered,
            this, &BleManager::onServiceDiscovered);
    connect(m_controller, &QLowEnergyController::discoveryFinished,
            this, &BleManager::onServiceScanDone);
    connect(m_controller, &QLowEnergyController::errorOccurred,
            this, &BleManager::onControllerError);

    m_controller->connectToDevice();
}

void BleManager::disconnectFromDevice()
{
    if (!m_controller) {
        return;
    }

    emit logMessage(tr("Rozłączanie z urządzeniem."));
    m_controller->disconnectFromDevice();
}

void BleManager::setTargetUuids(const QBluetoothUuid &serviceUuid,
                                const QBluetoothUuid &txCharacteristicUuid,
                                const QBluetoothUuid &rxCharacteristicUuid)
{
    m_serviceUuid = serviceUuid;
    m_txCharacteristicUuid = txCharacteristicUuid;
    m_rxCharacteristicUuid = rxCharacteristicUuid;
}

void BleManager::sendData(const QByteArray &data)
{
    if (!m_service || !m_txCharacteristic.isValid()) {
        emit logMessage(tr("Brak aktywnej usługi lub charakterystyki do wysłania danych."));
        return;
    }

    m_service->writeCharacteristic(m_txCharacteristic, data, QLowEnergyService::WriteWithResponse);
    emit logMessage(tr("Wysłano %1 bajtów do urządzenia.").arg(data.size()));
}

void BleManager::onDeviceDiscovered(const QBluetoothDeviceInfo &info)
{
    if (!(info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)) {
        return;
    }
    emit deviceDiscovered(info);
}

void BleManager::onDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    const auto meta = QMetaEnum::fromType<QBluetoothDeviceDiscoveryAgent::Error>();
    const char *key = meta.valueToKey(error);
    const QString errorName = key ? QString::fromLatin1(key) : tr("Nieznany");
    emit errorOccurred(tr("Błąd wyszukiwania urządzeń: %1 (%2)").arg(static_cast<int>(error)).arg(errorName));
}

void BleManager::onControllerConnected()
{
    emit controllerConnected(m_controller->remoteName());
    emit logMessage(tr("Połączono. Rozpoczynam skanowanie usług."));
    m_controller->discoverServices();
}

void BleManager::onControllerDisconnected()
{
    emit logMessage(tr("Urządzenie zostało rozłączone."));
    emit controllerDisconnected();
    resetController();
}

void BleManager::onControllerError(QLowEnergyController::Error error)
{
    const auto meta = QMetaEnum::fromType<QLowEnergyController::Error>();
    const char *key = meta.valueToKey(error);
    const QString errorName = key ? QString::fromLatin1(key) : tr("Nieznany");
    emit errorOccurred(tr("Błąd kontrolera: %1 (%2)").arg(static_cast<int>(error)).arg(errorName));
}

void BleManager::onServiceDiscovered(const QBluetoothUuid &uuid)
{
    emit serviceDiscovered(uuid);
    if (!m_serviceUuid.isNull() && uuid == m_serviceUuid) {
        emit logMessage(tr("Odnaleziono docelową usługę."));
    }
}

void BleManager::onServiceScanDone()
{
    emit serviceScanDone();

    if (m_serviceUuid.isNull()) {
        emit logMessage(tr("Nie ustawiono docelowego identyfikatora usługi BLE."));
        return;
    }

    if (m_service) {
        m_service->deleteLater();
        m_service = nullptr;
    }

    m_service = m_controller->createServiceObject(m_serviceUuid, this);
    if (!m_service) {
        emit errorOccurred(tr("Nie udało się utworzyć obiektu usługi."));
        return;
    }

    connect(m_service, &QLowEnergyService::stateChanged,
            this, &BleManager::onServiceStateChanged);
    connect(m_service, &QLowEnergyService::characteristicChanged,
            this, &BleManager::onCharacteristicChanged);
    connect(m_service, &QLowEnergyService::characteristicRead,
            this, &BleManager::onCharacteristicRead);

    m_service->discoverDetails();
}

void BleManager::onServiceStateChanged(QLowEnergyService::ServiceState state)
{
    switch (state) {
    case QLowEnergyService::DiscoveringServices:
        emit logMessage(tr("Trwa odkrywanie charakterystyk."));
        break;
    case QLowEnergyService::ServiceDiscovered:
        emit logMessage(tr("Odkryto wszystkie charakterystyki."));
        setupService();
        break;
    default:
        break;
    }
}

void BleManager::onCharacteristicChanged(const QLowEnergyCharacteristic &ch, const QByteArray &value)
{
    if (ch.uuid() == m_rxCharacteristicUuid) {
        emit dataReceived(value);
    }
}

void BleManager::onCharacteristicRead(const QLowEnergyCharacteristic &ch, const QByteArray &value)
{
    if (ch.uuid() == m_rxCharacteristicUuid) {
        emit dataReceived(value);
    }
}

void BleManager::resetController()
{
    if (m_service) {
        disconnect(m_service, nullptr, this, nullptr);
        m_service->deleteLater();
        m_service = nullptr;
    }

    m_txCharacteristic = QLowEnergyCharacteristic();
    m_rxCharacteristic = QLowEnergyCharacteristic();

    if (m_controller) {
        disconnect(m_controller, nullptr, this, nullptr);
        m_controller->deleteLater();
        m_controller = nullptr;
    }
}

void BleManager::setupService()
{
    if (!m_service) {
        return;
    }

    if (m_txCharacteristicUuid.isNull() || m_rxCharacteristicUuid.isNull()) {
        emit logMessage(tr("Nie ustawiono identyfikatorów charakterystyk."));
        return;
    }

    m_txCharacteristic = m_service->characteristic(m_txCharacteristicUuid);
    m_rxCharacteristic = m_service->characteristic(m_rxCharacteristicUuid);

    if (!m_txCharacteristic.isValid() || !m_rxCharacteristic.isValid()) {
        emit errorOccurred(tr("Nie odnaleziono wskazanych charakterystyk w usłudze."));
        return;
    }

    auto notificationDesc = m_rxCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
    if (notificationDesc.isValid()) {
        m_service->writeDescriptor(notificationDesc, QByteArray::fromHex("0100"));
    }

    emit logMessage(tr("Usługa BLE gotowa do wymiany danych."));
}
