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
#include "deviceinfo.h"
#include <QtEndian>
#include <QRandomGenerator>
#include <QDebug>
#include <QtGlobal>
#include <QStandardPaths>
#include <QFile>

/*
 *  This is the real deal, it creates a BLE Controller
 *  setDevice connects the device
 *  void DeviceHandler::serviceDiscovered(const QBluetoothUuid &gatt) - this slot is called during service discovery, here is the place to filter for uuid.
 *  setDevice -> serviceDiscovered -> serviceScanDone
 */

void DeviceHandler::sendCMDStringFromTerminal(const QString &str)
{
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
    else
    {
        qCritical()<<"Unknown Command!";
    }

    if (tba.size())
        m_service->writeCharacteristic(m_writeCharacteristic, tba, QLowEnergyService::WriteWithResponse); /*  m_writeMode */
}

DeviceHandler::DeviceHandler(QObject *parent) :
    BluetoothBaseClass(parent),
    m_control(0),
    m_service(0),
    m_currentDevice(0),
    m_refToOtherDevice(0),
    m_found_BLE_UART_Service(false)
{

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


void DeviceHandler::setDevice(DeviceInfo *device)
{
    clearMessages();
    m_currentDevice = device;
    if (device != NULL)
        m_deviceAddress = device->getAddress();

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
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() { setError("LowEnergy controller disconnected");});

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

    //! [Filter HeartRate service 2]
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
    case CDSM_STATE_ERROR:
        return QString("Error");
    default:
        return QString("Unknown");
    }
}

void write_type_to_file(QByteArray data, uint8_t type)
{
    QString homeLocation = QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory);

    switch (type)
    {
    case TYPE_AUD:
        homeLocation.append( QString("AUDIO") );
        break;
    case TYPE_GYR:
        homeLocation.append( QString("GYR") );
        break;
    case TYPE_ACC:
        homeLocation.append( QString("ACC") );
        break;
    case TYPE_PRS:
        homeLocation.append( QString("PRS") );
        break;
    case TYPE_MAG:
        homeLocation.append( QString("GYR") );
        break;
    default:
        homeLocation.append( QString("SOMEFILE") );
        break;
    }

    QFile file(homeLocation);
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();
    qDebug()<<"FILE_WRITTEN";
}

void DeviceHandler::ble_uart_rx(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
#define COMMAND_STATE    0x00
#define HUGE_CHUNK_STATE 0x01

    static uint8_t state = COMMAND_STATE;
    static uint16_t incoming_byte_count;
    static uint8_t incoming_type;


    QByteArray huge_chunk;

    if (c.uuid() != QBluetoothUuid(BLE_UART_TX_CHAR)) // TX CHAR OF THE SERVER
        return;
    qDebug()<<"<Data Received:";
    // ignore any other characteristic change -> shouldn't really happen though
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());
    quint8 flags = data[1];
    qInfo()<<"Data[1]"<<flags;
    qInfo()<<"Data:"<<value.toHex();


    if (state == COMMAND_STATE)
    {
        switch ( data[0] )
        {
        case TRIGGERED:
            //inform the other device
            if (m_refToOtherDevice != NULL)
            {
                QByteArray tba;
                tba.resize(2);
                tba[0] = TRIGGERED;
                tba[1] = 0xFF; //just a second char, not needed
                m_refToOtherDevice->ble_uart_tx(tba);
            }
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
            qDebug()<<"ALIVE: -STATE- "<<m_deviceState<<" -SUB STATE- "<<m_deviceSubState<<" -LAST ERROR- "<<m_deviceLastError;
            emit fileIndexOnDeviceChanged();
            emit deviceStateChanged();
            break;

        case HUGE_CHUNK_START:
            state = HUGE_CHUNK_STATE;
            huge_chunk.clear();
            incoming_byte_count = (uint16_t) ( data[1] << 8);
            incoming_byte_count |=  data[2];
            qDebug()<<"!!!!!!!!1incoming byte count:"<<incoming_byte_count;
            incoming_type = data[3];
            break;

        case HUGE_CHUNK_FINISH:
            state = COMMAND_STATE;
            write_type_to_file(huge_chunk, incoming_type);
            if (huge_chunk.size() == incoming_byte_count)
                qDebug()<<"!!!!!!!VERY WELL!";
            else
                qDebug()<<"!!!!!OHHOHEHOEASFAFE!";
            break;

        default:
            qDebug()<<"MSG:"<<value;
            break; //technically not needed
        }
    }
    else if (state == HUGE_CHUNK_STATE )
    {
        huge_chunk.append(value);
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
    qDebug() << "SIGNAL: Characteristic Written!" << value;
}

void DeviceHandler::onTimerTriggered()
{

    QByteArray data;
    data.resize(5);
    data[0] = 0x55;
    data[1] = 0x11;
    data[2] = 0x55;
    data[3] = 0x11;
    data[4] = 0x55;

#define CHUNK_SIZE 16
    qDebug() << "onTimerTriggered::writeTest.. ";
    if (m_service->state() == QLowEnergyService::ServiceDiscovered)
        qDebug()<<"Service is in Discovered State";
    else
        qDebug()<<"Service state is shitty, still trying to send";

    if(m_service && m_writeCharacteristic.isValid())
    {
        if(data.length() > CHUNK_SIZE)
        {
            int sentBytes = 0;
            while (sentBytes < data.length())
            {
                m_service->writeCharacteristic( m_writeCharacteristic, data.mid(sentBytes, CHUNK_SIZE), m_writeMode);
                sentBytes += CHUNK_SIZE;
                if(m_writeMode == QLowEnergyService::WriteWithResponse){
                    //waitForWrite();
                    qDebug()<<"waitForWrite() commented out";
                    if(m_service->error() != QLowEnergyService::NoError)
                        return;
                }
            }

        }
        else
        {
            m_service->writeCharacteristic(m_writeCharacteristic, data, QLowEnergyService::WriteWithResponse); /*  m_writeMode */
            qDebug()<<"writecall";
        }
    }

    //    qDebug()<<"Trying to Read";
    //    if(m_service && m_readCharacteristic.isValid())
    //        m_service->readCharacteristic(m_readCharacteristic);
}

void DeviceHandler::update_currentService()
{
    connect(m_service, &QLowEnergyService::stateChanged, this, &DeviceHandler::serviceStateChanged);
    connect(m_service, &QLowEnergyService::characteristicChanged, this, &DeviceHandler::ble_uart_rx);
    connect(m_service, &QLowEnergyService::descriptorWritten, this, &DeviceHandler::confirmedDescriptorWrite);
    connect(m_service, QOverload<QLowEnergyService::ServiceError>::of(&QLowEnergyService::error),
            [=](QLowEnergyService::ServiceError newError){ qCritical()<<"ERR - QlowEnergyServiceError!"; });
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
