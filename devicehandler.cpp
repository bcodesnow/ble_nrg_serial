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
#include <QLowEnergyConnectionParameters>
#include <QThread>

// change connection period with shutup!
// correct parsed length
// check if passed package nrs are correct when requesting a missed.
// request sensor data change to 7.5ms
// use less tx pool objects on each device
// add a catch drop button dialog
// add at startup the question to use sd card
// force connection parameters from both sides before huge chunk


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

void DeviceHandler::setBtAdapter(QBluetoothAddress addr)
{
    m_adapterAddress = addr;
    qDebug()<<m_ident_str<<"Received the Adapter:"<<addr.toString();
}

void DeviceHandler::setIdentifier(QString str, quint8 idx, QBluetoothAddress addr)
{
    m_ident_str = str;
    m_ident_idx = idx;
    m_adapterAddress = addr;
}

void DeviceHandler::setDevice(DeviceInfo *device)
{
    clearMessages();
    m_currentDevice = device;
    if (device != nullptr)
    {
        m_deviceAddress = device->getAddress();
        emit deviceAddressChanged();
    }

    // Disconnect and delete old connection
    if (m_control) {
        m_control->disconnectFromDevice();
        delete m_control;
        m_control = nullptr;
    }

    // Create new controller and connect it if device available
    if (m_currentDevice) {
        // We are using fixed RandomAddressType)
        m_addressType = QLowEnergyController::RandomAddress;

        // Make connections
        if (m_adapterAddress.isNull()) {
            // Create default connection
            m_control = new QLowEnergyController(m_currentDevice->getDevice(), this);
        }
        else {
            // Connect with specified bt adapter
            m_control = new QLowEnergyController(m_currentDevice->getDevice().address(),m_adapterAddress);
            qDebug()<<m_ident_str<<"connecting with adapter:"<<m_adapterAddress;
        }

        m_control->setRemoteAddressType(QLowEnergyController::RandomAddress); //changed to Random Address

        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &DeviceHandler::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &DeviceHandler::serviceScanDone);

        connect(m_control, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error), this, [this](QLowEnergyController::Error error) {Q_UNUSED(error);setError("Cannot connect to remote device.");});
        connect(m_control, &QLowEnergyController::connected, this, &DeviceHandler::onConnected);
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() { setError("BLE controller disconnected!");});
        connect(m_control, &QLowEnergyController::connectionUpdated, this, &DeviceHandler::onConnectionParamUpdated);

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
        qDebug()<<"NOT USED SERVICE STATE : "<<s;
        //nothing for now
        break;
    }

    emit aliveChanged();
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

void DeviceHandler::onConnectionParamUpdated(const QLowEnergyConnectionParameters &newParameters)
{
    qDebug()<<"DeviceHandler::onConnectionParamUpdated!!"<<newParameters.latency()<<
              newParameters.minimumInterval()<<newParameters.maximumInterval()<<
              newParameters.supervisionTimeout();
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
#ifdef USE_DEBUG
    qInfo() << "Characteristic Written! - Payload: " << value;
#endif
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());
    // use the state instead of reading every byte first byte we send...
    if ( data[0] == TS_MSG)
    {
        if (m_refToTimeStampler != 0)
            m_refToTimeStampler->time_sync_msg_sent(value);
    }
}


void DeviceHandler::onConnected()
{

    setInfo("Controller connected. Search services...");
    m_control->discoverServices();

}

void DeviceHandler::update_currentService()
{
    // connect(m_connParamTimer, &QTimer::timeout, this, &DeviceHandler::onConnParamTimerExpired);
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
                    //update_conn_period(); // DEBUG
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



///////////////////////////////////////////////////////////////////////////////////////////////////////

void DeviceHandler::requestSensorData()
{
    QByteArray tba;
    tba.resize(2);
    tba[0] = CMD_REQUEST_SENSORDATA;
    tba[1] = 0xFF;
    ble_uart_send_cmd_with_resp(tba);

}


void DeviceHandler::ackHugeChunk()
{
    // add timeout timer..
    QByteArray tba;
    tba.resize(2);
    tba[0] = HUGE_CHUNK_ACK_PROC;
    tba[1] = HC_1_ACK;
    qInfo()<<"ackHugeChunk()";
    this->ble_uart_tx(tba);
}

//void DeviceHandler::update_conn_period()
//{
//    quint8 conn_min_max =  m_ident_idx ? 6 : 10;
//    // add timeout timer..
//    QByteArray tba;
//    tba.resize(5);
//    tba[0] = SET_CONN_PERIOD;
//    tba[1] = conn_min_max;
//    tba[2] = conn_min_max;
//    tba[3] = (500u << 8) & 0xFF;
//    tba[4] = 500u & 0xFF;
//    qDebug()<<"Sending Conn Period!() min max = "<< conn_min_max << "default timeout = 500";
//    this->ble_uart_tx(tba);
//}

void DeviceHandler::setRequestedConnParamsOnDevice(uint8_t mode)
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
    this->ble_uart_send_cmd_with_resp(tba);
}


void DeviceHandler::parse_n_write_received_pool (uint16_t tmp_write_pointer, uint8_t type )
{
    QByteArray tba;
    for (int i = 0; i < m_hc_vec.size(); i++)
    {
        tba.append( m_hc_vec.at(i).barr );

    }
    qDebug()<<"PARSER FINISHED, gathered:"<<tba.size();
    //m_refToFileHandler->write_type_to_file(m_ident_str, m_huge_chunk, type, tmp_write_pointer);
}

//void DeviceHandler::setConnParams(double min_peri, double max_peri, int supervision_timeout, quint8 latency)
//{
//    QLowEnergyConnectionParameters para;
//    para.setLatency(latency);
//    para.setIntervalRange(min_peri, max_peri);
//    para.setSupervisionTimeout(supervision_timeout);
//    m_control->requestConnectionUpdate(para);
//}


//void  DeviceHandler::onConnParamTimerExpired()
//{

//}

void DeviceHandler::setShutUp(bool mode)
{
    qDebug()<<"Turning DEV"<<m_ident_idx<<m_ident_str<<"in shutupMode:"<<mode;
    QByteArray tba;
    tba.resize(2);
    tba[0] = SET_SHUT_UP;
    tba[1] = mode;
    this->ble_uart_tx(tba);
}



void DeviceHandler::setConnParamsOnCentral(uint8_t mode)
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

    //m_connParamTimer.singleShot(100, this, SLOT(onConnParamTimerExpired()));
    m_control->requestConnectionUpdate(para);
}
