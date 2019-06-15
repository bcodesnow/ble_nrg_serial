/***************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "devicehandler.h"
#include "ble_uart.h"
#include "deviceinfo.h"
#include <QtEndian>
#include <QRandomGenerator>
#include <QDebug>
#include <QtGlobal>

/*
 *  This is the real deal, it creates a BLE Controller
 *  setDevice connects the device
 *  void DeviceHandler::serviceDiscovered(const QBluetoothUuid &gatt) - this slot is called during service discovery, here is the place to filter for uuid.
 *  sometimes additional discovery is needed!
 *  setDevice -> serviceDiscovered -> serviceScanDone
 */

DeviceHandler::DeviceHandler(QObject *parent) :
    BluetoothBaseClass(parent),
    m_control(0),
    m_service(0),
    m_currentDevice(0),
    m_refToOtherDevice(0),
    m_refToFileHandler(0),
    m_found_BLE_UART_Service(false),
    m_fileIndexOnDevice(0),
    m_sdEnabled(0)
{

}

void DeviceHandler::sendCMDStringFromTerminal(const QString &str)
{
    if (!alive())
    {
        qCritical()<<"Shit is fkd up";
        return;
    }

    QByteArray tba;
    if ( str == QString("get_state()"))
    {
        tba.resize(1);
        tba[0] = GET_STATE;
        qInfo()<<"Fetching State";
    }
    else if ( str == QString("start()"))
    {
        tba.resize(1);
        tba[0] = START;
        qInfo()<<"Sending Start";
    }
    else if ( str == QString("stop()"))
    {
        tba.resize(1);
        tba[0] = STOP;
        qInfo()<<"Sending Stop";
    }

    else if (str == "confirm(1)" )
    {
        tba.resize(2);
        tba[0] = WRITE_CATCH_SUCCESS;
        tba[1] = 1;
        qInfo()<<"Sending Catch Confrim";
    }

    else if (str == "confirm(0)" )
    {
        tba.resize(0);
        tba[0] = WRITE_CATCH_SUCCESS;
        tba[1] = 0;
        qInfo()<<"Sending Catch Confrim";
    }
    else if (str == "ts()" )
    {
        m_refToTimeStampler->start_time_sync(m_ident_idx);
        qInfo()<<"TimeSync in Test..";
    }
    else
    {
        qCritical()<<"Unknown Command!";
    }

    if (tba.size())
        m_service->writeCharacteristic(m_writeCharacteristic, tba, QLowEnergyService::WriteWithResponse); /*  m_writeMode */
}

void DeviceHandler::requestBLESensorData()
{
        QByteArray tba;
        tba.resize(2);
        tba[0] = REQUEST_SENSORDATA;
        tba[1] = 0xFF;

        if (tba.size())
            m_service->writeCharacteristic(m_writeCharacteristic, tba, QLowEnergyService::WriteWithResponse); /*  m_writeMode */
}

void DeviceHandler::setAddressType(AddressType type)
{
    switch (type) {
    case DeviceHandler::AddressType::PublicAddress:
        m_addressType = QLowEnergyController::PublicAddress;
        break;
    case DeviceHandler::AddressType::RandomAddress:
        m_addressType = QLowEnergyController::RandomAddress;
        break;
    }
}

DeviceHandler::AddressType DeviceHandler::addressType() const
{
    if (m_addressType == QLowEnergyController::RandomAddress)
        return DeviceHandler::AddressType::RandomAddress;

    return DeviceHandler::AddressType::PublicAddress;
}

void DeviceHandler::setRefToOtherDevice(DeviceHandler *t_dev_handler)
{
    m_refToOtherDevice = t_dev_handler;
}

void DeviceHandler::setRefToFileHandler(LogFileHandler *t_fil_helper)
{
    m_refToFileHandler = t_fil_helper;
}

void DeviceHandler::setRefToTimeStampler(TimeStampler *t_time_stampler)
{
    m_refToTimeStampler = t_time_stampler;
}

void DeviceHandler::setIdentifier(QString str, quint8 idx)
{
    m_ident_str = str;
    m_ident_idx = idx;
}


void DeviceHandler::setDevice(DeviceInfo *device)
{
    clearMessages();
    m_currentDevice = device;
    if (device != NULL)
    {
        m_deviceAddress = device->getAddress();
        emit deviceAddressChanged();
    }

    // Disconnect and delete old connection
    if (m_control) {
        m_control->disconnectFromDevice();
        delete m_control;
        m_control = 0;
    }

    // Create new controller and connect it if device available
    if (m_currentDevice) {
        // We are using fixed RandomADdressType)
        m_addressType = QLowEnergyController::RandomAddress;

        // Make connections
        m_control = new QLowEnergyController(m_currentDevice->getDevice(), this);

        m_control->setRemoteAddressType(QLowEnergyController::RandomAddress); //changed to Random Address

        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &DeviceHandler::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &DeviceHandler::serviceScanDone);

        connect(m_control, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error), this, [this](QLowEnergyController::Error error) {Q_UNUSED(error);setError("Cannot connect to remote device.");});
        connect(m_control, &QLowEnergyController::connected, this, [this]() { setInfo("Controller connected. Search services..."); m_control->discoverServices();});
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() { setError("BLE controller disconnected!");});

        // Connect
        m_control->connectToDevice();
    }
}

void DeviceHandler::serviceDiscovered(const QBluetoothUuid &gatt)
{
    if (gatt == QBluetoothUuid(BLE_UART_SERVICE)) {
        setInfo("BLE UART Service discovered...");
        qDebug()<<"BLE UART Service discovered...";
        m_found_BLE_UART_Service = true;
    }
    qDebug()<<"Discovered Service: "<<gatt.toString();
}


void DeviceHandler::serviceScanDone()
{
    setInfo("Service scan done.");
    qDebug("Service scan done.");

    // Delete old service if available
    if (m_service) {
        delete m_service;
        m_service = 0;
    }

    // If BLE UART Service found, create new service
    if (m_found_BLE_UART_Service)
        m_service = m_control->createServiceObject(QBluetoothUuid(BLE_UART_SERVICE), this);

    if (m_service) {

        update_currentService();
        qDebug("SERVICE CREATED, SIGNALS Connected");
    } else {
        setError("BLE UART NOT FOUND");
        qCritical()<<"BLE UART NOT FOUND";
    }
}

// Service functions
//! [Find BLE UART characteristic]
void DeviceHandler::serviceStateChanged(QLowEnergyService::ServiceState s)
{
    qDebug()<<"Service State changed...";
    switch (s) {
    case QLowEnergyService::DiscoveringServices:
        qDebug("Discovering services...");
        setInfo(tr("Discovering services..."));
        break;
    case QLowEnergyService::ServiceDiscovered:
    {
        searchCharacteristic();
        break;
    }
    default:
        //nothing for now
        break;
    }

    emit aliveChanged();
}

QString DeviceHandler::state_to_string(uint8_t tmp)
{
    switch (tmp)
    {
    case CDSM_STATE_INIT:
        return QString("Initializing");
    case CDSM_STATE_RUNNING:
        return QString("Running");
    case CDSM_STATE_READY_TO_BE_TRIGGERED:
        return QString("Ready to Trigger");
    case CDSM_STATE_TRIGGERED:
        return QString("Triggered");
    case CDSM_STATE_STOPPING:
        return QString("Stopping");
    case CDSM_STATE_STOPPED:
        return QString("Stopped");
    case CDSM_STATE_RESTARTING:
        return QString("Restarting");
    case CDSM_STATE_POST_TRIGGER_DATA_COLLECTED:
        return QString("Data Collected");
    case CDSM_STATE_ERROR:
        return QString("Error");
    default:
        return QString("Unknown");
    }
}

void DeviceHandler::ble_uart_rx(const QLowEnergyCharacteristic &c, const QByteArray &value)
{

#define CMD_STATE    0x00
#define HUGE_CHUNK_STATE 0x01

    static uint8_t state = CMD_STATE;
    static uint16_t incoming_byte_count;
    static uint8_t incoming_type;
    static uint32_t rec_ts;

    if (c.uuid() != QBluetoothUuid(BLE_UART_TX_CHAR)) // TX CHAR OF THE SERVER
        return;

    // ignore any other characteristic change -> shouldn't really happen though
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());

    if (state == CMD_STATE)
    {
        switch ( data[0] )
        {
        case TRIGGERED:
            //inform the other device
            if (m_refToOtherDevice != NULL)
                m_refToOtherDevice->ble_uart_tx(value); // 1-1 forward it!
            //
            if (m_sdEnabled)
                m_refToFileHandler->add_to_log_fil(m_ident_str,"File ID on Device", QString::number(data[1]));
            //
//            rec_ts = ( ( uint32_t ) ( rec_ts | data[2] ) ) << 8;
//            rec_ts = ( ( uint32_t ) ( rec_ts | data[3] ) ) << 8;
//            rec_ts = ( ( uint32_t ) ( rec_ts | data[4] ) ) << 8;
//            rec_ts =   ( uint32_t ) ( rec_ts | data[5] );
            rec_ts = 0;
            rec_ts = ( (uint32_t) data[2] ) << 24;
            rec_ts |=( (uint32_t) data[3] )<< 16;
            rec_ts |=( (uint32_t) data[4] )<< 8;
            rec_ts |= ( (uint32_t) data[5] );

            m_refToFileHandler->add_to_log_fil(m_ident_str,"TS in Trigger MSG", QString::number(rec_ts));
            //
            if (data[6] == 1<<1)
                m_refToFileHandler->add_to_log_fil(m_ident_str,"Trigger Source", "MAG");
            else if (data[6] == 1<<2)
                m_refToFileHandler->add_to_log_fil(m_ident_str,"Trigger Source", "ACC");
            else
                m_refToFileHandler->add_to_log_fil(m_ident_str,"Trigger Source", "Things got messed up..");
            //
            m_refToFileHandler->add_to_log_fil(m_ident_str,"Trigger MSG Received", QString::number(m_refToTimeStampler->get_timestamp_us()));
            break;

        case DATA_COLLECTED:
            //confirm if it was a catch or drop -> we should give it to a class above iE devFinder
            setInfo("Waiting for Confirmation!");
            break;

        case ALIVE:
            //show current state and file index.. here we should start a timer, and if no more msg arrives to alive for 3secs we know we lsot the sensor.
            m_deviceState = state_to_string(data[1]);
            m_fileIndexOnDevice = data[3];
            m_deviceSubState = data[2];
            m_deviceLastError = data[4];

            if (m_deviceLastError)
                m_refToFileHandler->add_to_log_fil(m_ident_str,"Last Error", QString(data[4]));

            qDebug()<<m_ident_str<<" --- ALIVE: -STATE- "<<m_deviceState<<" -SUB STATE- "<<m_deviceSubState<<" -LAST ERROR- "<<m_deviceLastError;

            // TODO: if SD
            m_sdEnabled = data[5];
            emit sdEnabledChanged();
            emit fileIndexOnDeviceChanged();
            emit deviceStateChanged();
            emit aliveArrived();
            setInfo("Alive!");
            break;

        case HUGE_CHUNK_START:
            state = HUGE_CHUNK_STATE;
            incoming_byte_count =  data[1] << 8;
            incoming_byte_count |=  data[2];
            incoming_type = data[3];

            uint16_t tmp_write_pointer;
            tmp_write_pointer =  data[4] << 8;
            tmp_write_pointer |=  data[5];

            m_refToFileHandler->add_to_log_fil(m_ident_str, QString("Type"), QString::number(incoming_type));
            m_refToFileHandler->add_to_log_fil(m_ident_str, QString("ByteCountToReceive"), QString::number(incoming_byte_count));
            m_refToFileHandler->add_to_log_fil(m_ident_str, QString("WritePointer"), QString::number(tmp_write_pointer));

            setInfo("Incoming!");

            break;

        case HUGE_CHUNK_FINISH:
            //m_refToFileHandler->write_type_to_file(m_ident_str, m_huge_chunk, incoming_type);

            if (m_huge_chunk.size() == incoming_byte_count)
            {
                setInfo("All Received!");
                qDebug()<<"All Received";
                m_refToFileHandler->add_to_log_fil(m_ident_str, QString("AllArrived"), QString("TRUE"));
            }
            else
            {
                setError("Some got lost!");
                qCritical()<<"Some got lost!";
                qCritical()<<"huge_chunk.size()"<<m_huge_chunk.size()<<"incoming_byte_count"<<incoming_byte_count;
                m_refToFileHandler->add_to_log_fil(m_ident_str,"AllArrived", QString("FALSE"));
            }
            m_huge_chunk.clear();
            break;
            //
        case SENSORDATA_AVAILABLE:
            qDebug()<<"Sensor Data can be downloaded!";
            setInfo("Sensordata available!");
            emit sensorDataAvailable();
            break;
            //
        case SENDING_SENSORDATA_FINISHED:
            emit sensorDataReceived();
            qDebug()<<"SENDING_SENSORDATA_FINISHED";
            setInfo("Sensordata Received!");
            // set state ready to increase, ask the pal if he is ready - if true - increase
            // need the write pointer
            //
            break;
        case TS_MSG:
            m_refToTimeStampler->time_sync_msg_arrived(value);
            break;

            //)
        default:
            qWarning()<<"Unknown MSG: "<<value;
            break; //technically not needed
        }
    }
    else if (state == HUGE_CHUNK_STATE )
    {
        if ( value.length() == SW_REC_MODE_LENGTH )
        {
            if ( data[0] == SWITCH_RECEIVE_MODE &&
                 data[1] == 0x55 &&
                 data[2] == 0xFF &&
                 data[3] == 0x55 )
            {
                state = CMD_STATE;
                m_refToFileHandler->write_type_to_file(m_ident_str, m_huge_chunk, incoming_type);
                qDebug()<<"SWITCH TO CMD STATE";
            }
        }
        else
        {
            m_huge_chunk.append(value);
        }
    }
}

void DeviceHandler::ble_uart_tx(const QByteArray &value)
{
    if (value.size())
        m_service->writeCharacteristic(m_writeCharacteristic, value, QLowEnergyService::WriteWithResponse); /*  m_writeMode */
}

void DeviceHandler::confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value)
{
    if (d.isValid() && d == m_notificationDescriptor && value == QByteArray::fromHex("0000")) {
        //disabled notifications -> assume disconnect intent
        m_control->disconnectFromDevice();
        delete m_service;
        m_service = 0;
    }
}

void DeviceHandler::disconnectService()
{
    m_found_BLE_UART_Service = false;

    //disable notifications
    if (m_notificationDescriptor.isValid() && m_service
            && m_notificationDescriptor.value() == QByteArray::fromHex("0100")) {
        m_service->writeDescriptor(m_notificationDescriptor, QByteArray::fromHex("0000"));
    } else {
        if (m_control)
            m_control->disconnectFromDevice();

        delete m_service;
        m_service = 0;
    }
}

void DeviceHandler::onCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    Q_UNUSED(c)
    qDebug() << "SIGNAL: Characteristic Changed! " << value;
}

void DeviceHandler::onCharacteristicRead(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    Q_UNUSED(c)
    qDebug() << "SIGNAL: Characteristic Read Value! " << value << " UUID: "<< c.uuid();

}

void DeviceHandler::onCharacteristicWritten(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    Q_UNUSED(c)
    qInfo() << "Characteristic Written! - Payload: " << value;
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());
    if ( data[0] == TS_MSG)
    {
        if (m_refToTimeStampler != 0)
            m_refToTimeStampler->time_sync_msg_sent(value);
    }
}


void DeviceHandler::update_currentService()
{
    connect(m_service, &QLowEnergyService::stateChanged, this, &DeviceHandler::serviceStateChanged);
    connect(m_service, &QLowEnergyService::characteristicChanged, this, &DeviceHandler::ble_uart_rx);
    connect(m_service, &QLowEnergyService::descriptorWritten, this, &DeviceHandler::confirmedDescriptorWrite);
    connect(m_service, QOverload<QLowEnergyService::ServiceError>::of(&QLowEnergyService::error),
            [=](QLowEnergyService::ServiceError newError){ qCritical()<<"ERR - QlowEnergyServiceError!"; Q_UNUSED(newError);});
    connect(m_service, SIGNAL(characteristicRead(QLowEnergyCharacteristic,QByteArray)), this, SLOT(onCharacteristicRead(QLowEnergyCharacteristic,QByteArray)));
    connect(m_service, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)),this, SLOT(onCharacteristicWritten(QLowEnergyCharacteristic,QByteArray)));

    if(m_service->state() == QLowEnergyService::DiscoveryRequired) {
        qWarning("Additional Discovery Required! Continue in StateChanged...");
        m_service->discoverDetails();
    }
    else
        searchCharacteristic();
}


bool DeviceHandler::alive() const
{
    if (m_service)
        return m_service->state() == QLowEnergyService::ServiceDiscovered;

    return false;
}

void DeviceHandler::searchCharacteristic()
{
    if(m_service){
        foreach (QLowEnergyCharacteristic c, m_service->characteristics())
        {
            if( c.isValid() )
            {
                qDebug()<<"Characteristic UUID:"<<c.uuid();
                printProperties(c.properties());
                if ( c.uuid() == QBluetoothUuid( BLE_UART_RX_CHAR) && ( c.properties() & QLowEnergyCharacteristic::WriteNoResponse || c.properties() & QLowEnergyCharacteristic::Write) )
                {
                    qDebug()<<"Write Characteristic Registered";
                    m_writeCharacteristic = c;
                    if(c.properties() & QLowEnergyCharacteristic::WriteNoResponse)
                    {
                        m_writeMode = QLowEnergyService::WriteWithoutResponse;
                    }
                    else
                    {
                        m_writeMode = QLowEnergyService::WriteWithResponse;
                    }
                }
                else if ( c.uuid() == QBluetoothUuid( BLE_UART_TX_CHAR ) )
                {
                    qDebug()<<"Read (Notify) Characteristic Registered";
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
            }
        }

        //        if(!m_test_timer && m_readCharacteristic.isValid() && m_writeCharacteristic.isValid())
        //        {
        //            qDebug()<<"Starting Test Timer";
        //            this->m_test_timer = new QTimer(this);
        //            connect(m_test_timer, &QTimer::timeout, this, &DeviceHandler::onTimerTriggered);
        //            #define INTERVAL_MS 500
        //            m_test_timer->start(INTERVAL_MS);
        //        }
    }

}

void DeviceHandler::printProperties(QLowEnergyCharacteristic::PropertyTypes props)
{
    if (props.testFlag(QLowEnergyCharacteristic::Unknown)) qDebug()<<"Property unknown";
    if (props.testFlag(QLowEnergyCharacteristic::Broadcasting)) qDebug()<<"Property: Broadcasting";
    if (props.testFlag(QLowEnergyCharacteristic::Read)) qDebug()<<"Property: Read";
    if (props.testFlag(QLowEnergyCharacteristic::WriteNoResponse)) qDebug()<<"Property: Write (no response)";
    if (props.testFlag(QLowEnergyCharacteristic::Write)) qDebug()<<"Property: Write";
    if (props.testFlag(QLowEnergyCharacteristic::Notify)) qDebug()<<"Property: Notify";
    if (props.testFlag(QLowEnergyCharacteristic::Indicate)) qDebug()<<"Property: Indicate";
    if (props.testFlag(QLowEnergyCharacteristic::WriteSigned)) qDebug()<<"Property: Write (signed)";
    if (props.testFlag(QLowEnergyCharacteristic::ExtendedProperty)) qDebug()<<"Property: Extended Property";
}
