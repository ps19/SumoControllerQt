#include "mainwindow.h"

#include "blemanager.h"

#include <QBluetoothUuid>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>

namespace {
constexpr auto kDefaultServiceUuid = "0000ffe0-0000-1000-8000-00805f9b34fb";
constexpr auto kDefaultTxUuid = "0000ffe1-0000-1000-8000-00805f9b34fb";
constexpr auto kDefaultRxUuid = "0000ffe1-0000-1000-8000-00805f9b34fb";
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_bleManager(new BleManager(this))
{
    setupUi();

    connect(m_bleManager, &BleManager::deviceDiscovered,
            this, &MainWindow::addDeviceToList);
    connect(m_bleManager, &BleManager::discoveryFinished,
            this, &MainWindow::onDiscoveryFinished);
    connect(m_bleManager, &BleManager::controllerConnected,
            this, &MainWindow::onControllerConnected);
    connect(m_bleManager, &BleManager::controllerDisconnected,
            this, &MainWindow::onControllerDisconnected);
    connect(m_bleManager, &BleManager::errorOccurred,
            this, &MainWindow::onErrorOccurred);
    connect(m_bleManager, &BleManager::serviceDiscovered,
            this, &MainWindow::onServiceDiscovered);
    connect(m_bleManager, &BleManager::serviceScanDone,
            this, &MainWindow::onServiceScanDone);
    connect(m_bleManager, &BleManager::dataReceived,
            this, &MainWindow::onDataReceived);
    connect(m_bleManager, &BleManager::logMessage,
            this, &MainWindow::appendLog);

    updateUuidConfiguration();
}

void MainWindow::setupUi()
{
    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *mainLayout = new QHBoxLayout(central);

    // Device list and actions
    auto *deviceLayout = new QVBoxLayout();
    m_deviceList = new QListWidget(this);
    m_scanButton = new QPushButton(tr("Skanuj"), this);
    m_stopScanButton = new QPushButton(tr("Stop"), this);
    m_connectButton = new QPushButton(tr("Połącz"), this);
    m_disconnectButton = new QPushButton(tr("Rozłącz"), this);

    deviceLayout->addWidget(new QLabel(tr("Urządzenia BLE"), this));
    deviceLayout->addWidget(m_deviceList);

    auto *deviceButtonLayout = new QHBoxLayout();
    deviceButtonLayout->addWidget(m_scanButton);
    deviceButtonLayout->addWidget(m_stopScanButton);
    deviceLayout->addLayout(deviceButtonLayout);

    deviceLayout->addWidget(m_connectButton);
    deviceLayout->addWidget(m_disconnectButton);

    mainLayout->addLayout(deviceLayout, 1);

    // Communication panel
    auto *commLayout = new QVBoxLayout();

    auto *uuidForm = new QFormLayout();
    m_serviceUuidEdit = new QLineEdit(QString::fromLatin1(kDefaultServiceUuid), this);
    m_txUuidEdit = new QLineEdit(QString::fromLatin1(kDefaultTxUuid), this);
    m_rxUuidEdit = new QLineEdit(QString::fromLatin1(kDefaultRxUuid), this);

    uuidForm->addRow(tr("UUID usługi"), m_serviceUuidEdit);
    uuidForm->addRow(tr("UUID TX"), m_txUuidEdit);
    uuidForm->addRow(tr("UUID RX"), m_rxUuidEdit);

    commLayout->addLayout(uuidForm);

    m_sendEdit = new QLineEdit(QStringLiteral("1,2,3,4"), this);
    m_sendButton = new QPushButton(tr("Wyślij"), this);

    auto *sendLayout = new QHBoxLayout();
    sendLayout->addWidget(new QLabel(tr("Dane do wysłania (CSV)"), this));
    sendLayout->addWidget(m_sendEdit);
    sendLayout->addWidget(m_sendButton);

    commLayout->addLayout(sendLayout);

    m_logView = new QTextEdit(this);
    m_logView->setReadOnly(true);
    commLayout->addWidget(new QLabel(tr("Log"), this));
    commLayout->addWidget(m_logView, 1);

    m_receiveView = new QTextEdit(this);
    m_receiveView->setReadOnly(true);
    commLayout->addWidget(new QLabel(tr("Odebrane dane"), this));
    commLayout->addWidget(m_receiveView, 1);

    mainLayout->addLayout(commLayout, 2);

    connect(m_scanButton, &QPushButton::clicked, this, &MainWindow::onScanDevices);
    connect(m_stopScanButton, &QPushButton::clicked, this, &MainWindow::onStopScan);
    connect(m_connectButton, &QPushButton::clicked, this, &MainWindow::onConnectToSelectedDevice);
    connect(m_disconnectButton, &QPushButton::clicked, this, &MainWindow::onDisconnect);
    connect(m_sendButton, &QPushButton::clicked, this, &MainWindow::onSendData);

    setWindowTitle(tr("SumoController BLE"));
    resize(900, 600);
}

void MainWindow::onScanDevices()
{
    m_devices.clear();
    m_deviceList->clear();
    m_bleManager->startDeviceDiscovery();
    appendLog(tr("Rozpoczęto skanowanie."));
}

void MainWindow::onStopScan()
{
    m_bleManager->stopDeviceDiscovery();
}

void MainWindow::onConnectToSelectedDevice()
{
    const auto row = m_deviceList->currentRow();
    if (row < 0 || row >= m_devices.size()) {
        QMessageBox::warning(this, tr("Połączenie"), tr("Wybierz urządzenie z listy."));
        return;
    }

    updateUuidConfiguration();
    m_bleManager->connectToDevice(m_devices.at(row));
}

void MainWindow::onDisconnect()
{
    m_bleManager->disconnectFromDevice();
}

void MainWindow::onSendData()
{
    const auto text = m_sendEdit->text();
    updateBuffersFromInput(text);

    if (m_txBuffer.isEmpty()) {
        appendLog(tr("Nie wprowadzono prawidłowych danych do wysłania."));
        return;
    }

    QByteArray payload;
    payload.resize(m_txBuffer.size());
    for (int i = 0; i < m_txBuffer.size(); ++i) {
        payload[i] = static_cast<char>(m_txBuffer.at(i));
    }

    m_bleManager->sendData(payload);
}

void MainWindow::addDeviceToList(const QBluetoothDeviceInfo &info)
{
    m_devices.append(info);
    const auto name = info.name().isEmpty() ? tr("(bez nazwy)") : info.name();
    const auto itemText = tr("%1 (%2)").arg(name, info.address().toString());
    m_deviceList->addItem(itemText);
}

void MainWindow::onDiscoveryFinished()
{
    appendLog(tr("Skanowanie zakończone."));
}

void MainWindow::onControllerConnected(const QString &name)
{
    appendLog(tr("Połączono z %1.").arg(name));
}

void MainWindow::onControllerDisconnected()
{
    appendLog(tr("Rozłączono."));
}

void MainWindow::onErrorOccurred(const QString &message)
{
    appendLog(message);
}

void MainWindow::onServiceDiscovered(const QBluetoothUuid &uuid)
{
    appendLog(tr("Odnaleziono usługę %1.").arg(formatUuid(uuid)));
}

void MainWindow::onServiceScanDone()
{
    appendLog(tr("Zakończono skanowanie usług."));
}

void MainWindow::onDataReceived(const QByteArray &data)
{
    m_rxBuffer.clear();
    m_rxBuffer.reserve(data.size());
    for (const auto byte : data) {
        m_rxBuffer.append(static_cast<quint8>(byte));
    }
    appendLog(tr("Odebrano %1 bajtów.").arg(data.size()));
    refreshReceiveDisplay();
}

void MainWindow::appendLog(const QString &message)
{
    m_logView->append(message);
}

void MainWindow::updateUuidConfiguration()
{
    const auto serviceUuid = QBluetoothUuid(m_serviceUuidEdit->text());
    const auto txUuid = QBluetoothUuid(m_txUuidEdit->text());
    const auto rxUuid = QBluetoothUuid(m_rxUuidEdit->text());
    m_bleManager->setTargetUuids(serviceUuid, txUuid, rxUuid);
}

QString MainWindow::formatUuid(const QBluetoothUuid &uuid) const
{
    return uuid.toString(QUuid::WithoutBraces);
}

void MainWindow::updateBuffersFromInput(const QString &text)
{
    m_txBuffer.clear();

    const auto parts = text.split(',', Qt::SkipEmptyParts);
    for (const auto &part : parts) {
        bool ok = false;
        const auto value = part.trimmed().toUInt(&ok);
        if (ok) {
            m_txBuffer.append(static_cast<quint8>(value & 0xFF));
        }
    }
}

void MainWindow::refreshReceiveDisplay()
{
    QStringList list;
    list.reserve(m_rxBuffer.size());
    for (auto it = m_rxBuffer.cbegin(); it != m_rxBuffer.cend(); ++it) {
        list.append(QString::number(*it));
    }
    m_receiveView->setText(list.join(',') );
}
