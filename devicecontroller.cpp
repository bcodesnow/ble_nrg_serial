#include "devicecontroller.h"
#include "ble_uart.h"
#include "deviceinfo.h"
#include <QtEndian>
#include <QRandomGenerator>
#include <QDebug>
#include <QtGlobal>
#include <QLowEnergyConnectionParameters>
#include <QThread>

// correct parsed length
// add a catch drop button dialog
// add at startup the question to use sd card

/*
 *  This is the real deal, it creates a BLE Controller, ment to run in its own thread and communicate via signals and slots.
 *  void DeviceHandler::serviceDiscovered(const QBluetoothUuid &gatt) - this slot is called during service discovery,
 *  Here is the place to filter for service uuid.
 *  Sometimes an additional discovery is needed!
 *  connectToPeripheral -> serviceDiscovered -> serviceScanDone
 */

DeviceController::DeviceController(int idx, QString identifier, QObject *parent) :
    QObject(parent),
    m_ident_idx(idx),
    m_ident_str(identifier),
    m_control(nullptr),
    m_service(nullptr),
    m_currentDevice(nullptr),
    m_bleUartServiceFound(false)
{
    m_dev_conn_param_info.requested_mode = UNKNOWN;
}

void DeviceController::setIdentifier(QString str, quint8 idx)
{
    m_ident_str = str;
    m_ident_idx = idx;
}

void DeviceController::connectToPeripheral(QBluetoothDeviceInfo *device)
{
    m_currentDevice = device;

    // Disconnect and delete old connection
    if (m_control) {
        m_control->disconnectFromDevice();
        delete m_control;
        m_control = nullptr;
    }

    // Create new controller and connect it if device available
    if (m_currentDevice) {
        // Make connections
        if (m_adapterAddress.isNull())
        {
            // Create default connection
            m_control = new QLowEnergyController( m_currentDevice->address() , this);
        }
        else
        {
            // Connect with specified bt adapter
            m_control = new QLowEnergyController(m_currentDevice->address(), m_adapterAddress);
            qDebug()<<m_ident_str<<"connecting with adapter:"<<m_adapterAddress;
        }

        m_control->setRemoteAddressType(QLowEnergyController::RandomAddress); //We are using fixed RandomAddressType
        qDebug()<<"DeviceController -> Connecting signals.. prepare to launch..";

        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &DeviceController::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &DeviceController::serviceScanDone);
        connect(m_control, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error), this, [this](QLowEnergyController::Error error) {Q_UNUSED(error) Q_UNUSED(this) qDebug()<<"Cannot connect to remote device.";});
        connect(m_control, &QLowEnergyController::connected, this, &DeviceController::onConnected);
        connect(m_control, &QLowEnergyController::disconnected, this, &DeviceController::onDisconnected);
        connect(m_control, &QLowEnergyController::connectionUpdated, this, &DeviceController::onCentralConnectionUpdated);

        // Connect
        m_control->connectToDevice();
    }
}

void DeviceController::serviceDiscovered(const QBluetoothUuid &gatt)
{
    if (gatt == QBluetoothUuid(BLE_UART_SERVICE))
    {
        qDebug()<<"BLE UART Service discovered...";
        m_bleUartServiceFound = true;
    }
    qDebug()<<"Discovered Service: "<<gatt.toString();
}


void DeviceController::serviceScanDone()
{
    qDebug("Service scan done.");

    // Delete old service if available
    if (m_service)
    {
        delete m_service;
        m_service = 0;
    }

    // If BLE UART Service found, create new service
    if (m_bleUartServiceFound)
        m_service = m_control->createServiceObject(QBluetoothUuid(BLE_UART_SERVICE), this);

    if (m_service)
    {
        updateCurrentService();
        qDebug("SERVICE CREATED, SIGNALS Connected");
    }
    else
    {
        qCritical()<<"BLE UART NOT FOUND";
    }

}

//! [Find BLE UART characteristic]
void DeviceController::serviceStateChanged(QLowEnergyService::ServiceState s)
{
    qDebug()<<"Service State changed...";
    switch (s)
    {
    case QLowEnergyService::DiscoveringServices:
        qDebug("Discovering services...");
        break;
    case QLowEnergyService::ServiceDiscovered:
        searchCharacteristic();
        break;

    default:
        qDebug()<<"NOT USED SERVICE STATE : "<<s;
        break;
    }
}


void DeviceController::confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value)
{
    qWarning()<<"confirmedDescriptorWrite.. if this needs to be used, we need to keep track of our m_notificationDescriptors..";
    if (d.isValid() && d == m_notificationDescriptor && value == QByteArray::fromHex("0000"))
    {
        //disabled notifications -> assume disconnect intent
        m_control->disconnectFromDevice();
        delete m_service;
        m_service = 0;
    }
}

void DeviceController::disconnectService()
{
    m_bleUartServiceFound = false;

    //disable notifications
    if (m_notificationDescriptor.isValid() && m_service && m_notificationDescriptor.value() == QByteArray::fromHex("0100"))
    {
        m_service->writeDescriptor(m_notificationDescriptor, QByteArray::fromHex("0000"));
    }
    else
    {
        if (m_control)
            m_control->disconnectFromDevice();

        delete m_service;
        m_service = nullptr;
    }
    emit connectionAlive(false);
}

void DeviceController::onCentralConnectionUpdated(const QLowEnergyConnectionParameters &newParameters)
{
    qDebug()<<"QLowEnergyController::connectionUpdated!!"<<
              newParameters.latency()<<
              newParameters.minimumInterval()<<
              newParameters.maximumInterval()<<
              newParameters.supervisionTimeout();
}

//void DeviceHandler::onCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value)
//{
//    Q_UNUSED(c)
//    qDebug() << "SIGNAL: Characteristic Changed! " << value;
//}
// this should not be needed as a central

void DeviceController::onCharacteristicRead(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    Q_UNUSED(c)
    qDebug() << "SIGNAL: Characteristic Read Value! " << value << " UUID: "<< c.uuid();

}

void DeviceController::onCharacteristicWritten(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
#ifdef USE_DEBUG
    qInfo() << "Characteristic Written! - Payload: " << value;
#endif
    const quint8 *data;
    Q_UNUSED(c)
    data = reinterpret_cast<const quint8 *>(value.constData());
    // todo add a state and it state instead of reading every first byte first we send...
    if ( data[0] == TS_MSG)
    {
        emit timeSyncMsgSent(value, m_ident_idx);
    }
}


void DeviceController::onConnected()
{
    emit connectionAlive(true); // todo move this to the point where all the characteristics are registered...
    m_control->discoverServices();
}

void DeviceController::onDisconnected()
{
    emit connectionAlive( false );
}

// this is something one can query to make sure we have a connection
//m_service->state() == QLowEnergyService::ServiceDiscovered;

void DeviceController::updateCurrentService()
{
    connect(m_service, &QLowEnergyService::stateChanged, this, &DeviceController::serviceStateChanged);
    connect(m_service, &QLowEnergyService::characteristicChanged, this, &DeviceController::ble_uart_rx);
    connect(m_service, &QLowEnergyService::descriptorWritten, this, &DeviceController::confirmedDescriptorWrite);
    connect(m_service, QOverload<QLowEnergyService::ServiceError>::of(&QLowEnergyService::error),
            [=](QLowEnergyService::ServiceError newError){ qCritical()<<"ERR - QlowEnergyServiceError!"; Q_UNUSED(newError);});
    connect(m_service, SIGNAL(characteristicRead(QLowEnergyCharacteristic,QByteArray)), this, SLOT(onCharacteristicRead(QLowEnergyCharacteristic,QByteArray)));
    connect(m_service, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)),this, SLOT(onCharacteristicWritten(QLowEnergyCharacteristic,QByteArray)));

    if(m_service->state() == QLowEnergyService::DiscoveryRequired)
    {
        qWarning("Additional Discovery Required! Continue in StateChanged...");
        m_service->discoverDetails();
    }
    else
        searchCharacteristic();
}

void DeviceController::searchCharacteristic()
{
    if(m_service){
        foreach (QLowEnergyCharacteristic c, m_service->characteristics())
        {
            if( c.isValid() )
            {
                qDebug()<<"Characteristic UUID:"<<c.uuid();
#ifdef DEBUG_ALL_OUT
                printProperties(c.properties());
#endif
                if ( c.uuid() == QBluetoothUuid( BLE_UART_RX_CHAR) && ( c.properties() & QLowEnergyCharacteristic::WriteNoResponse || c.properties() & QLowEnergyCharacteristic::Write) )
                {
                    qDebug()<<"Write Characteristic Registered";
                    m_writeCharacteristic = c;
                    //emit writeValidChanged();
                    if(c.properties() & QLowEnergyCharacteristic::WriteNoResponse)
                    {
                        qInfo()<<"QLowEnergyService::WriteWithoutResponse";
                        m_writeMode = QLowEnergyService::WriteWithoutResponse;
                    }
                    else
                    {
                        qInfo()<<"QLowEnergyService::WriteWithResponse";
                        m_writeMode = QLowEnergyService::WriteWithResponse;
                    }
                }
                else if ( c.uuid() == QBluetoothUuid( BLE_UART_TX_CHAR ) )
                {
#ifdef DEBUG_ALL_OUT
                    qDebug()<<"Read (Notify) Characteristic Registered";
#endif
                    m_readCharacteristic = c;
                    m_notificationDescriptor = c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);

                    if (m_notificationDescriptor.isValid()) {
                        qDebug()<<"Characteristic Descriptor: ClientCharacteristicConfiguration, writing 0100 to descriptor";
                        m_service->writeDescriptor(m_notificationDescriptor, QByteArray::fromHex("0100"));
                    }
                    else
                    {
                        qDebug()<<"Could not attach notification service to Read Characteristic.";
                    }
                }
                else
                {
                    qDebug()<<"Looking for others";
                    // Look for TX Pool Characteristics
                    for (int i=0; i < ( BLE_UART_TX_POOL.size() ); i++)
                    {
                        //qDebug()<<"CHECKING IN BLE_POOL"<<c.uuid()<<"   ==   "<<BLE_UART_TX_POOL.at(i);
                        if ( c.uuid() == QBluetoothUuid( BLE_UART_TX_POOL.at(i) ))
                        {
#ifdef DEBUG_ALL_OUT
                            qDebug()<<"We have a match, id found:"<<i;
#endif
                            m_rxCharacteriscitPool[i] =  c;
                            m_rxCharacteriscitPoolDescriptors[i] = c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);

                            if (m_rxCharacteriscitPoolDescriptors[i].isValid()) {
#ifdef DEBUG_ALL_OUT
                                qDebug()<<"Characteristic Descriptor: ClientCharacteristicConfiguration, writing 0100 to descriptor"<< "TXPOOL MEMBER: "<<i;
#endif
                                m_service->writeDescriptor(m_rxCharacteriscitPoolDescriptors[i], QByteArray::fromHex("0100"));
                            }
                            else
                            {
                                qDebug()<<"Could not attach notification service to Read Characteristic.";
                            }
                        }
                    }

                }
            }
        }
    }
}


void DeviceController::sendRequestSensorData()
{
    QByteArray tba;
    tba.resize(2);
    tba[0] = CMD_REQUEST_SENSORDATA;
    tba[1] = 0xFF;
    bleUartSendCmdWithResp(tba);

}

void DeviceController::startConnModeChangeProcedure(quint8 mode)
{

    m_dev_requested_conn_mode = mode;

    if (isDeviceInRequestedConnState())
    {
        emit requestedConnModeReached(true, m_dev_conn_param_info.requested_mode);
    }
    else
    {
        retries_remaining = 3;
        setRequestedConnParamsOnDevice(m_dev_requested_conn_mode);
        setConnParamsOnCentral(m_dev_requested_conn_mode);
        m_connParamTimer.singleShot(200, this, &DeviceController::onConnParamTimerExpired);
    }
}

void DeviceController::setRequestedConnParamsOnDevice(uint8_t mode)
{
    QByteArray tba;
    tba.resize(2);
    tba[0] = CMD_SET_CONN_MODE;
    switch (mode)
    {
    case SLOW:
        tba[1] = SLOW;
        break;

    case MID:
        tba[1] = MID;
        break;

    case FAST:
        tba[1] = FAST;
        break;
    }
    m_dev_requested_conn_mode = mode;
    this->bleUartSendCmdWithResp(tba);
}

void DeviceController::initializeDevice(QBluetoothHostInfo* hostInfo, QBluetoothDeviceInfo* deviceInfo)
{
    qDebug()<<"device will be initialized";
    qDebug()<<"received adapter add"<<hostInfo->address();
    qDebug()<<"received device name"<<deviceInfo->name();
    m_adapterAddress = hostInfo->address();
    this->connectToPeripheral(deviceInfo);
}

void DeviceController::writeReceivedChunkToFile (uint16_t tmp_write_pointer, uint8_t type )
{

    if (m_last_hc_payload_ptr != nullptr)
        m_last_hc_payload_ptr->~QByteArray();
    m_last_hc_payload_ptr = new QByteArray();

    for (int i = 0; i < m_hc_vec.size(); i++)
    {
        m_last_hc_payload_ptr->append( m_hc_vec.at(i).barr);
    }
    qDebug()<<"Received" << m_last_hc_payload_ptr->size() <<"bytes raw data after filling it to a qbyterray:";

    emit invokeWriteTypeToFile(m_ident_str, m_last_hc_payload_ptr, type, tmp_write_pointer);
}

void DeviceController::sendSetShutUp(bool mode)
{
    qDebug()<<"Turning DEV"<<m_ident_idx<<m_ident_str<<"in shutupMode:"<<mode;
    QByteArray tba;
    tba.resize(2);
    tba[0] = CMD_SET_SHUT_UP;
    tba[1] = mode;
    this->bleUartSendCmdWithResp(tba);
}

void DeviceController::setConnParamsOnCentral(uint8_t mode)
{
    QLowEnergyConnectionParameters para;
    m_dev_requested_conn_mode = mode;

    switch (mode)
    {
    case SLOW:
        para.setLatency(S_LAT);
        para.setIntervalRange(S_MIN, S_MAX);
        para.setSupervisionTimeout(S_SUP);
        break;

    case MID:
        para.setLatency(M_LAT);
        para.setIntervalRange(M_MIN, M_MAX);
        para.setSupervisionTimeout(M_SUP);
        break;

    case FAST:
        para.setLatency(F_LAT);
        para.setIntervalRange(F_MIN, F_MAX);
        para.setSupervisionTimeout(F_SUP);
        break;
    }

    m_control->requestConnectionUpdate(para);
}
