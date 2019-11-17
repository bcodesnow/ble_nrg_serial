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
    connectionValidSent = 0;
    connect(this, &DeviceController::connParamInfoArrived, this, &DeviceController::onConnParamInfoArrived);
    m_last_hc_payload_ptr = nullptr;
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
        }

        m_control->setRemoteAddressType(QLowEnergyController::RandomAddress); //We are using fixed RandomAddressType
#if (VERBOSITY_LEVEL >= 3)
        qDebug()<<"DeviceController -> Connecting signals.. prepare to launch..";
#endif
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &DeviceController::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &DeviceController::serviceScanDone);
        connect(m_control, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>
                (&QLowEnergyController::error), this, [this](QLowEnergyController::Error error)
        {Q_UNUSED(error) Q_UNUSED(this) qDebug()<<"Cannot connect to remote device.";});
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
#if (VERBOSITY_LEVEL >= 2)
    qDebug()<<"confirmedDescriptorWrite"<<d.name();
#endif
    if (d.isValid() && d == m_notificationDescriptor && value == QByteArray::fromHex("0000"))
    {
        //disabled notifications -> assume disconnect intent
        m_control->disconnectFromDevice();
        delete m_service;
        m_service = nullptr;
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
    emit connectionAlive(false, m_ident_idx);
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
#if (VERBOSITY_LEVEL >= 2 )
    qInfo() << "Characteristic Written! - Payload: " << value;
#endif
    const quint8 *data;
    Q_UNUSED(c)
    data = reinterpret_cast<const quint8 *>(value.constData());
    // todo add a state and it state instead of reading every first byte first we send...
    if ( data[0] == TS_MSG)
    {
        emit timeSyncMsgSent(m_ident_idx);
    }
}


void DeviceController::onConnected()
{
    m_control->discoverServices();
}

void DeviceController::onDisconnected()
{
    emit connectionAlive( false, m_ident_idx );
}

// this is something one can query to make sure we have a connection
//m_service->state() == QLowEnergyService::ServiceDiscovered;

void DeviceController::updateCurrentService()
{
    connect(m_service, &QLowEnergyService::stateChanged, this, &DeviceController::serviceStateChanged);
    connect(m_service, &QLowEnergyService::characteristicChanged, this, &DeviceController::bleUartRx);

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
#if (VERBOSITY_LEVEL >= 2)
                qDebug()<<"Characteristic UUID:"<<c.uuid();
                qDebug()<<"undefined reference: printProperties(c.properties())";
               // printProperties(c.properties());
#endif
                if ( c.uuid() == QBluetoothUuid( BLE_UART_RX_CHAR) && ( c.properties() & QLowEnergyCharacteristic::WriteNoResponse || c.properties() & QLowEnergyCharacteristic::Write) )
                {
#if (VERBOSITY_LEVEL >= 2)
                    qDebug()<<"Write Characteristic Registered";
#endif
                    m_writeCharacteristic = c;
                    if(c.properties() & QLowEnergyCharacteristic::WriteNoResponse)
                    {
#if (VERBOSITY_LEVEL >= 2)
                        qInfo()<<"QLowEnergyService::WriteWithoutResponse";
#endif
                        m_writeMode = QLowEnergyService::WriteWithoutResponse;
                    }
                    else
                    {
#if (VERBOSITY_LEVEL >= 2)
                        qInfo()<<"QLowEnergyService::WriteWithResponse";
#endif
                        m_writeMode = QLowEnergyService::WriteWithResponse;
                    }
                }
                else if ( c.uuid() == QBluetoothUuid( BLE_UART_TX_CHAR ) )
                {
#if (VERBOSITY_LEVEL >= 2)
                    qDebug()<<"Read (Notify) Characteristic Registered";
#endif
                    m_readCharacteristic = c;
                    m_notificationDescriptor = c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);

                    if (m_notificationDescriptor.isValid()) {
#if (VERBOSITY_LEVEL >= 2)
                        qDebug()<<"Characteristic Descriptor: ClientCharacteristicConfiguration, writing 0100 to descriptor";
#endif
                        m_service->writeDescriptor(m_notificationDescriptor, QByteArray::fromHex("0100"));
                    }
                    else
                    {
                        qDebug()<<"Could not attach notification service to Read Characteristic.";
                    }
                }
                else
                {
                    #if (VERBOSITY_LEVEL >= 2)
                    qDebug()<<"Looking for others";
                    #endif
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
            if ( m_writeCharacteristic.isValid() && m_readCharacteristic.isValid() && !connectionValidSent && c == m_service->characteristics().last() )
            {
                QByteArray ba;
                ba.append(CMD_CONNECTION_VALID);
                bleUartSendCmdWithResp(ba);
                connectionValidSent = 1;
                qDebug()<<"READ WRITE REGISTERED, SENDING CONN VALID!";
                emit connectionAlive(true, m_ident_idx); // todo move this to the point where all the characteristics are registered...
                // todo -> this is much more relevant than ble connected-> pass this to catch controller
                connect(m_service, &QLowEnergyService::descriptorWritten, this, &DeviceController::confirmedDescriptorWrite);
            }
        }
    }
}


void DeviceController::sendRequestSensorData(quint8 catchSuccess)
{
    QByteArray tba;
    tba.resize(2);
    tba[0] = CMD_REQUEST_SENSORDATA;
    tba[1] = catchSuccess;
    bleUartSendCmdWithResp(tba);
#if (VERBOSITY_LEVEL >= 1)
    qDebug()<<"Sending Request Sensor Data and in the first message also the Catch Success!";
#endif
}


//void DeviceController::sendStartToDevice()
//{
////#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID))
////    qDebug()<<"Sending START";
////    QByteArray tba;
////    tba.resize(1);
////    tba[0] = CMD_START;
////    this->bleUartSendCmdWithResp(tba);
////#endif

//#if (defined(Q_OS_ANDROID))
//// one of the devices might be in a very slow connection mode, so sending a command to it requires first changing the conn parameters

////    setConnParamsOnCentral(FAST);
////    setRequestedConnParamsOnDevice(FAST);
////    m_nextRequest = NEXT_REQ_SEND_START;
////    m_nextRequestTimer->setInterval(1000);
////    m_nextRequestTimer->start();
////    qDebug()<<"Starting Timer!";
//#endif
//}

void DeviceController::startConnModeChangeProcedure(quint8 mode)
{
    qDebug()<<"startConnModeChangeProcedure(quint8 mode)";
    m_dev_requested_conn_mode = mode;

    if (isDeviceInRequestedConnState())
    {
        emit requestedConnModeReached(true, m_ident_idx); // todo, it was passing mode - thats why type conv probs
    }
    else
    {
        qDebug()<<"startConnModeChangeProcedure(quint8 mode)"<<"retry!";
        retries_remaining = 4;

        setRequestedConnParamsOnDevice(m_dev_requested_conn_mode);
        setConnParamsOnCentral(m_dev_requested_conn_mode);
        m_connParamTimer->setInterval(CONN_MODE_CHANGE_DELAY);
        m_connParamTimer->start();
    }
}

void DeviceController::setRequestedConnParamsOnDevice(uint8_t mode)
{
    qDebug()<<"setRequestedConnParamsOnDevice(uint8_t mode)";
    QByteArray tba;
    tba.resize(2);
    tba[0] = CMD_SET_CONN_MODE;
    tba[1] = mode;

    m_dev_requested_conn_mode = mode;
    this->bleUartSendCmdWithResp(tba);
}

void DeviceController::initializeDevice(QBluetoothHostInfo* hostInfo, QBluetoothDeviceInfo* deviceInfo)
{
    m_debugTimer = new QElapsedTimer();
    m_connParamTimer= new QTimer();

    m_cmdTimer = new QTimer();
    m_nextRequestTimer = new QTimer();
    m_nextRequestTimer->setSingleShot(true);


    m_bleUartSendCmdWithRespBacklogTimer = new QTimer();
    m_bleUartSendCmdWithRespBacklogTimer->setInterval(250);
    m_bleUartSendCmdWithRespBacklogTimer->setSingleShot(true);

    qInfo()<<"| INIT"<<"Device: "<<m_ident_str<<"-"<<m_ident_idx<<"| Adapter: "<<hostInfo->address()<<"|";

    connect(m_cmdTimer, &QTimer::timeout, this, &DeviceController::onCmdTimerExpired);
    connect(m_connParamTimer, &QTimer::timeout, this, &DeviceController::onConnParamTimerExpired);
    connect(this, &DeviceController::startHugeChunkArrived, this, &DeviceController::onStartHugeChunkArrived);
    connect(this, &DeviceController::noChunkAvailableArrived, this, &DeviceController::onNoChunkAvailableArrived);
    connect(this, &DeviceController::startHugeChunkAckProcArrived, this, &DeviceController::onStartHugeChunkAckProcArrived);
    connect(this, &DeviceController::replyMissingPackageArrived, this, &DeviceController::onReplyMissingPackageArrived);
    connect(m_nextRequestTimer, &QTimer::timeout, this, &DeviceController::onNextRequestTimerExpired);

    connect(m_bleUartSendCmdWithRespBacklogTimer, &QTimer::timeout, this, &DeviceController::onBleUartSendCmdWithRespTimerExpired);

    // TODO: trigger request sensor data if it did not work and missing packages were requested!!! (nextRequestTimer->singleShot!!)


    m_adapterAddress = hostInfo->address();
    this->connectToPeripheral(deviceInfo);
}

void DeviceController::writeReceivedChunkToFile (uint16_t tmp_write_pointer, uint8_t type )
{
    if (m_last_hc_payload_ptr != nullptr)
    {
        m_last_hc_payload_ptr->~QByteArray();
    }
    m_last_hc_payload_ptr = new QByteArray();

    for (int i = 0; i < m_hc_vec.size(); i++)
    {
        m_last_hc_payload_ptr->append( m_hc_vec.at(i).barr);
    }
    qInfo()<<"Received" << m_last_hc_payload_ptr->size() <<"bytes, handling over to fileHandler!";

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
    qDebug()<<"setConnParamsOnCentral(uint8_t mode)"<<"mode"<<mode;
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



///
///
///
///
///
///


bool DeviceController::bleUartSendCmdWithResp(const QByteArray &value, quint16 timeout, quint8 retry)
{
#if ( VERBOSITY_LEVEL >= 1)
    qDebug()<<"bleUartSendCmdWithResp(const QByteArray &value, quint16 timeout, quint8 retry)"<<"Timeout"<<timeout;
#endif

    if (m_cmdTimer->isActive())
    {
        qWarning()<<"!!! bleUartSendCmdWithResp() -- Resend needed!"<<cmd_resp_struct.last_cmd;
        m_bleUartSendCmdWithRespBacklogRetries = 3;
        m_bleUartSendCmdWithRespBacklog.append( new QByteArray (value) );
        m_bleUartSendCmdWithRespBacklogTimer->start();

        return true;
    }
    cmd_resp_struct.last_cmd.clear();
    cmd_resp_struct.last_cmd.append(value);

    cmd_resp_struct.timeout = timeout;
    cmd_resp_struct.retry = retry;

    m_cmdTimer->setInterval(timeout);
    m_cmdTimer->start();

    if (value.size())
        m_service->writeCharacteristic(m_writeCharacteristic, value, QLowEnergyService::WriteWithoutResponse); /*  m_writeMode */

    return false;
}

void DeviceController::onCmdTimerExpired()
{
    if ( cmd_resp_struct.retry )
    {
        cmd_resp_struct.retry--;
        m_cmdTimer->setInterval(cmd_resp_struct.timeout);
        m_cmdTimer->start();
        m_service->writeCharacteristic(m_writeCharacteristic, cmd_resp_struct.last_cmd, QLowEnergyService::WriteWithoutResponse); /*  m_writeMode */
    }
    else
    {
        // ERROR
        // is this a moment where the devil should appear?
        qWarning()<<"CMD Timeout!";
        if (cmd_resp_struct.last_cmd.at(0) == CMD_REQUEST_SENSORDATA)
            emit allDataDownloaded(false, m_ident_idx);
    }
}



void DeviceController::bleUartSendCmdOk()
{
#if (VERBOSITY_LEVEL >= 1)
    qDebug()<<"Sending CMD OK!";
#endif
    QByteArray tba;
    tba.resize(2);
    tba[0] = CMD_OK;
    tba[1] = 0xFF;
    bleUartTx(tba);
}

void DeviceController::bleUartTx(const QByteArray &value)
{

#if (VERBOSITY_LEVEL >= 2 )
    qDebug()<<"bleUartTx(const QByteArray &value)"<<*value;
#endif
    if (value.size())
        m_service->writeCharacteristic(m_writeCharacteristic, value, QLowEnergyService::WriteWithResponse); /*  m_writeMode todo -> I JUST CHANGED IT TO FIXED WITHOUT */
}

inline bool DeviceController::isDeviceInRequestedConnState()
{
#if (VERBOSITY_LEVEL >= 1)
    qDebug()<<"isDeviceInRequestedConnState()";
#endif
    // todo: the required conn mode is always true in the catch controller...
    return ( ( m_dev_conn_param_info.current_mode == m_dev_requested_conn_mode ) && ( m_dev_conn_param_info.requested_mode == m_dev_requested_conn_mode ));
}

void DeviceController::bleUartRx(const QLowEnergyCharacteristic &c, const QByteArray &value)
{

    // it would have been the best to only move the send receive functions to an own thread.. with this solution we will have a bit faster dispatch of time sync and receive..

    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());
    static const QBluetoothUuid ble_uart_receive = QBluetoothUuid(BLE_UART_TX_CHAR); // TX CHAR OF THE SERVER

    static uint64_t rxPoolPkgCount = 0;
    uint16_t tidx;
    huge_chunk_indexed_byterray_t hc_tmp_iba_struct; // todo -> there are some very bad namings..

    QString hex= QString("%1").arg(data[0] , 0, 16);

    if ( c.uuid() == ble_uart_receive )
    {
#if (VERBOSITY_LEVEL >= 2 )
        qDebug()<<"BLE UART RX FIRST BYTE:"<<hex;
#endif
        switch ( data[0] )
        {
        case CMD_OK:
            m_cmdTimer->stop();
#if (VERBOSITY_LEVEL >= 1)
            qDebug()<<"CMD OK Received..!"<<m_ident_idx;
#endif
            break;
        case TRIGGERED:

            //inform the other device
            emit requestDispatchToOtherDevices(value, m_ident_idx);
            //ble_uart_send_cmd_ok(); // Should we also ACK this to the device?!
            emit triggeredArrived(value); // TODO this is handled by the deviceinterface
            this->bleUartSendCmdOk();

            break;

            //        case DATA_SAVED_TO_SD:

            //            // todo!!!
            //            // confirm if it was a catch or drop -> we should give it to a class "above" -> catch controller
            //            qDebug()<<"Writing data to SD finished, waiting for confimation!";
            //            // send CMD_WRITE_CATCH_SUCC
            //            //setInfo("Waiting for Confirmation!");

            //            break;

        case ALIVE:
            emit aliveArrived(value);
            break;
        case REPLY_START_HUGE_CHUNK:
            huge_chunk_start_t* hc_transfer_struct_ptr;
            hc_transfer_struct_ptr = (huge_chunk_start_t*) &data[1];
            hc_transfer_struct = *hc_transfer_struct_ptr;
            emit startHugeChunkArrived();
            break;
        case REPLY_NO_CHUNK_AVAILABLE:
            emit noChunkAvailableArrived();
            break;
        case CMD_SENSORDATA_AVAILABLE:
            qDebug()<<"Sensor Data avialable for download!";
            emit sensorDataAvailableArrived(m_ident_idx);
            bleUartSendCmdOk(); //todo, the catch controller also sends ok?!
            break;
        case TS_MSG:
            emit timeSyncMessageArrived(value);
            break;
        case CMD_START_HUGE_CHUNK_ACK_PROC:
            emit startHugeChunkAckProcArrived(value);
            break;
        case REPLY_MISSED_PACKAGE:
            qDebug()<<"missed arrived, calling slot";
            emit replyMissingPackageArrived(value);
            break;
        case DIAG_INFO:
            if (data[1] == DIAG_1_TYPE_LENGTH_TEST)
            {
                qDebug()<<"MESS LEN:::"<<value.size();
            }
            else
            {
                qDebug()<<"Unimplemented diag msg";
            }
            break;
        case CONN_PARAM_INFO:
            // i should have implemented all packages this way..
            conn_param_info_t* cp_ptr;
            cp_ptr = (conn_param_info_t*) &data[1];
            m_dev_conn_param_info = *cp_ptr;
            bleUartSendCmdOk();
            emit connParamInfoArrived();
            //            m_cmdTimer->stop();

            break;
            //
        default:
            qWarning()<<"Unknown MSG: "<<value;
            break; //technically not needed
        }

        // todo if cmd id -> send ack.. this could save us from some headache
    }

    else //(c.uuid() != QBluetoothUuid(BLE_UART_TX_CHAR)) -> One of the TX_POOL Characteristics
    {
        // this is the ble rx pooled part.. it could be also moved to an own handler
        tidx = data[0];

        if (hc_helper_struct.first_multi_chunk && !tidx )
        {
            tidx = 0;
            hc_helper_struct.first_multi_chunk = false;
            hc_helper_struct.last_idx = 0;

            m_debugTimer->start();
//            m_progress = 0;
//            m_transfSpeed = 0;
        }
        else if ( tidx > hc_helper_struct.hc_highest_index )
        {
            hc_helper_struct.hc_highest_index = tidx;
        }
        else
        {
            do {
                tidx += ( 0xFF + 1 );
            } while (tidx < ( hc_helper_struct.hc_highest_index - 100 ));
            // this is not as bullet proof as hell, but it would work as long the messages are aligned within 100
            hc_helper_struct.hc_highest_index = tidx;
        }
#if (VERBOSITY_LEVEL >= 3 )
        qDebug()<<"HC -> Received IDX" << data[0] << "Calculated IDX" << tidx << "  package size" << value.size()-1;
#endif
        // THAT WAS A DIRTY BUG
        // hc_tmp_iba_struct.barr.append(value.size()-1, data[1]); // add the 19 bytes -> it looks like it could work... but :/
        hc_tmp_iba_struct.barr = value.right(value.size()-1);
        hc_tmp_iba_struct.received = 1;

#if (VERBOSITY_LEVEL >= 3 )
        QByteArray testArr = value.right(value.size()-1);
        qDebug()<<"\n";
        qDebug()<<testArr;
        qDebug()<<"\n";
#endif
        if (tidx < hc_transfer_struct.incoming_package_count)
            m_hc_vec.replace(tidx,hc_tmp_iba_struct);
        else
        {
            // We might show the devil
            qWarning()<<"HC -> Received idx too high, would leak out of the vector and crash the system!";
        }
        if (tidx > hc_helper_struct.last_idx++)
        {
            qDebug()<<"HC -> Skipped !!! "<<hc_helper_struct.last_idx+1;
            hc_helper_struct.skipped = true;
        }

        if ( tidx == hc_transfer_struct.incoming_package_count - 1 )
        {
#if (VERBOSITY_LEVEL >= 1)
            qDebug()<<"HC -> Last PKG!";
#endif
#if (PRINT_THROUGHPUT == 1)
            printThroughput();
#endif
            hc_helper_struct.last_received = true;
        }
        else if (tidx > hc_transfer_struct.incoming_package_count )
        {
            qWarning()<<"HC -> Too many?!";
        }
    }
}



void DeviceController::sendRequestMissingPackage()
{
    request_missing_pkg_t req;
    hc_helper_struct.missed_in_request = req.pkg_id = m_hc_missed.at(0);
    m_hc_missed.removeAt(0);

    QByteArray tba;
    tba[0] = CMD_REQUEST_MISSING_PACKAGE;
    tba.append((const char*) &req, sizeof(request_missing_pkg_t));
    qDebug()<<"HC -> Requesting missed : "<<req.pkg_id;
    this->bleUartSendCmdWithResp(tba);
}


void DeviceController::onReplyMissingPackageArrived(QByteArray value)
{
    qDebug()<<"Missed Package arrived";
    m_cmdTimer->stop();

    huge_chunk_indexed_byterray_t tmp;
    tmp.barr.append( value.right(value.size() - 1) );
    tmp.received = 1;
    m_hc_vec.replace( hc_helper_struct.missed_in_request , tmp );
    qDebug()<<"Missed in Request"<<hc_helper_struct.missed_in_request;

    if (m_hc_missed.size())
    {
        sendRequestMissingPackage();
    }
    else
    {
        this->sendAckOkHugeChunk();
        writeReceivedChunkToFile( hc_transfer_struct.write_pointer, hc_transfer_struct.incoming_type );
        hugeChunkDownloadFinished();
        // reset variables
    }
}


void DeviceController::onStartHugeChunkArrived()
{
    m_cmdTimer->stop();
#if (VERBOSITY_LEVEL >= 1)
    qDebug()<<"onStartHugeChunkArrived()";
    if ( hc_transfer_struct.incoming_byte_count )
    {
        qDebug()<<"#HC -> Multi Chunk Transfer Starts! "<< hc_transfer_struct.incoming_byte_count << " Bytes to Receive!";
    }
    else
    {
        // Todo: Show the Devil!
    }
    qDebug()<<"HC -> Pkg Count"<< hc_transfer_struct.incoming_package_count;
#endif

    m_hc_vec = QVector<huge_chunk_indexed_byterray_t> ( hc_transfer_struct.incoming_package_count );
    hc_helper_struct.hc_highest_index = 0;
    hc_helper_struct.first_multi_chunk = true;
    hc_helper_struct.missed_pkg_cnt_to_request = 0;
    hc_helper_struct.last_received = false;

    emit invokeAddToLogFile(m_ident_str, QString("Type"), stateToString(hc_transfer_struct.incoming_type));
    emit invokeAddToLogFile(m_ident_str, QString("ByteCountToReceive"), QString::number(hc_transfer_struct.incoming_byte_count));
  //  emit invokeAddToLogFile(m_ident_str, QString("WritePointer"), QString::number(hc_transfer_struct.write_pointer));

    sendStartOkHugeChunk();

}


void DeviceController::onStartHugeChunkAckProcArrived(QByteArray value)
{
    Q_UNUSED(value)
    //const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());

#if (VERBOSITY_LEVEL >= 3)
    qDebug()<<"onStartHugeChunkAckProcArrived : started!";
#endif
    if ( hc_helper_struct.skipped | !hc_helper_struct.last_received )
    {
#if (VERBOSITY_LEVEL >= 3)
        qDebug()<<"onStartHugeChunkAckProcArrived : arrived_package_count != incoming_package_count!";
#endif
        m_hc_missed.clear();
        for ( int i = 0; i < m_hc_vec.size(); i++)
        {
            if ( !m_hc_vec.at(i).received )
            {
                qDebug()<<"Adding"<<i;
                hc_helper_struct.missed_pkg_cnt_to_request++;
                m_hc_missed.append(i); //to rst later
            }
        }
        hc_helper_struct.skipped = false;
#if (VERBOSITY_LEVEL>=1)
        qDebug()<<"HUGE_CHUNK_ACK_PROC : need to request"<<m_hc_missed.size()<<"packages ;( ";
#endif
    }

#if (VERBOSITY_LEVEL >= 1)
    qDebug()<<"m_missed_to_request"<<hc_helper_struct.missed_pkg_cnt_to_request;
#endif
    if ( hc_helper_struct.missed_pkg_cnt_to_request == 0 )
    {
        hugeChunkDownloadFinished();
    }
    else
    {
#if (VERBOSITY_LEVEL >= 1)
        qDebug()<<"Requesting missed";
#endif
        sendRequestMissingPackage();
    }
}

void DeviceController::onNoChunkAvailableArrived()
{
    qInfo()<<"onNoChunkAvailableArrived()";
    m_cmdTimer->stop(); //todo dont forget to add this on all replies!
    bleUartSendCmdOk();
    emit allDataDownloaded(true, m_ident_idx);
}

void DeviceController::startDownloadAllDataProcedure( quint8 catchSuccess )
{
    sendRequestSensorData(catchSuccess);
}

void DeviceController::sendAckOkHugeChunk()
{
    // add timeout timer..
    QByteArray tba;
    tba.resize(2);
    tba[0] = CMD_HC_ACK_OK;
#if (VERBOSITY_LEVEL >= 1)
    qInfo()<<"ackHugeChunk()";
#endif
    this->bleUartSendCmdWithResp(tba);
}

void DeviceController::sendStartOkHugeChunk()
{
    // add timeout timer..
    QByteArray tba;
    tba.resize(2);
    tba[0] = CMD_HC_START_OK;
#if (VERBOSITY_LEVEL >= 1)
    qInfo()<<"sendStartOkHugeChunk()";
#endif
    this->bleUartSendCmdWithResp(tba);
}

void DeviceController::hugeChunkDownloadFinished()
{
#if (VERBOSITY_LEVEL >= 1)
    qDebug()<<"All Received";
#endif
    emit invokeAddToLogFile(m_ident_str, QString("AllArrived"), QString("TRUE"));

    sendAckOkHugeChunk();
    writeReceivedChunkToFile( hc_transfer_struct.write_pointer, hc_transfer_struct.incoming_type);

    m_nextRequest = NEXT_REQ_SEND_SENS_DATA;
    m_nextRequestTimer->setInterval(WAIT_X_MS_BETWEEN_CHUNKS);
    m_nextRequestTimer->start();
#if (VERBOSITY_LEVEL >= 1)
    qDebug()<<"Next HC has been requested";
#endif

}


void DeviceController::onNextRequestTimerExpired()
{
    switch ( m_nextRequest )
    {
    //    case NEXT_REQ_SEND_START:
    //        qDebug()<<"Sending START"<<m_ident_idx;
    //        tba.resize(1);
    //        tba[0] = CMD_START;
    //        this->bleUartSendCmdWithResp(tba, 250, 10);
    //           break;
    case NEXT_REQ_SEND_SENS_DATA:
        sendRequestSensorData();
        break;
    }
}



void DeviceController::onBleUartSendCmdWithRespTimerExpired()
{
    if (m_bleUartSendCmdWithRespBacklog.size())
    {
        QByteArray tba;
        tba.append(*m_bleUartSendCmdWithRespBacklog.last());
        qDebug()<<"COPIED:"<<tba;
        if ( bleUartSendCmdWithResp(tba) )
        {
            qWarning()<<"Resend Failed!";
            if (m_bleUartSendCmdWithRespBacklogRetries)
            {
                m_bleUartSendCmdWithRespBacklogRetries--;
                m_bleUartSendCmdWithRespBacklogTimer->start();
            }
            else
            {
                qWarning()<<"REALLY SHOW THE DEVIL!";
            }
        }
        else
        {
            qDebug()<<"Successfully resent!!";
            qDebug()<<"BARR SIZE BEFOR"<<                m_bleUartSendCmdWithRespBacklog.size();

            m_bleUartSendCmdWithRespBacklog.last()->~QByteArray();
            m_bleUartSendCmdWithRespBacklog.pop_back();
            qDebug()<<"BARR SIZE AFTER"<<                m_bleUartSendCmdWithRespBacklog.size();

        }


    }
}

void DeviceController::onConnParamInfoArrived()
{

    qDebug()<<">>>  CONN PARAM INFO:"<<m_ident_idx<<m_ident_str<<"from dev:"<<
              "interval:"<<m_dev_conn_param_info.interval<<"latency:"<<m_dev_conn_param_info.latency<<
              "current mode:"<<m_dev_conn_param_info.current_mode<<"requested mode:"<<m_dev_conn_param_info.requested_mode<<"callcount"<<m_dev_conn_param_info.reserved<<"   <<<";

    if (isDeviceInRequestedConnState() && m_connParamTimer->isActive())
    {
        m_connParamTimer->stop();
        qDebug()<<"requestedModeReached!"<<m_ident_idx;
        emit requestedConnModeReached(true, m_ident_idx); // todo modified
    }
}


void DeviceController::onConnParamTimerExpired()
{
    if ( retries_remaining )
    {
        retries_remaining--;
        setConnParamsOnCentral(m_dev_requested_conn_mode);


        //        todo added, not really wanted..
        //        setRequestedConnParamsOnDevice(m_dev_requested_conn_mode);
        m_connParamTimer->stop();
        m_connParamTimer->setInterval(CONN_MODE_CHANGE_DELAY);
        m_connParamTimer->start();
    }
    else
    {
        m_connParamTimer->stop();
        emit requestedConnModeReached(false, m_ident_idx);
        qDebug()<<"connparamupdate failed after x retries.."<<m_ident_idx;
        //show the devil or reply back to catch controller that the action failed
    }
}


