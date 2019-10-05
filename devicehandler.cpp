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

// change connection period with shutup!
// correct parsed length
// check if passed package nrs are correct when requesting a missed.
// request sensor data change to 7.5ms
// use less tx pool objects on each device
// add a catch drop button dialog
// add at startup the question to use sd card
// force connection parameters from both sides before huge chunk

QElapsedTimer debugTimer;
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
    /*
    huge_chunk_indexed_byterray_t tmp;
    uint8_t data[12] = {0xff, 0x55, 0xff, 0x55, 0xff, 0x55, 0xff, 0x55, 0xff, 0x55, 0xff, 0x55};
    tmp.barr->append(10, data[1]);
    tmp.idx = 123;
    m_hc_vec = QVector<huge_chunk_indexed_byterray_t> ( 8 );
    m_hc_vec.replace(4,tmp);
    m_hc_vec.replace(7,tmp);

    qDebug()<<"Appended"<<"vecsize:" << m_hc_vec.size();
    qDebug()<<m_hc_vec.at(4).barr<<m_hc_vec.at(7).idx;
    qDebug()<<m_hc_vec.at(0).barr<<m_hc_vec.at(0).idx;
    */
    m_hc_missed = QList<quint16>();

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




    if (tba.size() && m_writeCharacteristic.isValid())
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
        qDebug()<<"NOT USED SERVICE STATE : "<<s;
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
#define REQUESTING_MISSED_STATE 0x02

    static uint8_t state = CMD_STATE;
    static uint16_t incoming_byte_count;
    static uint16_t incoming_package_count;
    static uint16_t arrived_package_count;
    static uint16_t hc_highest_index;
    static bool first_multi_chunk;
    static uint8_t incoming_type;
    static uint32_t rec_ts;
    static uint16_t tmp_write_pointer; // moved definition
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());
    huge_chunk_indexed_byterray_t tmp;
    static uint16_t last_idx;
    uint16_t tidx;
    float kbyte_ps;
    float kbit_ps;
    float secs;
    quint64 elapsed;

    if (c.uuid() == QBluetoothUuid(BLE_UART_TX_CHAR)) // TX CHAR OF THE SERVER
    {

        // ignore any other characteristic change -> shouldn't really happen though

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
                qDebug()<<"HC -> Starts, arrived msg size: <<"<<value.size();
                state = HUGE_CHUNK_STATE;
                incoming_byte_count =  data[1] << 8;
                incoming_byte_count |=  data[2];
                incoming_type = data[3];

                // uint16_t tmp_write_pointer; // moved definition
                tmp_write_pointer =  data[4] << 8;
                tmp_write_pointer |=  data[5];
                qDebug()<<"data6 contains:<< data[6]";

                if (data[6] > 1)
                {
                    qDebug()<<"HC -> Multi Chunk Transfer Starts! "<< incoming_byte_count << " Bytes to Receive!";
                    m_multi_chunk_mode = true;
                    quint8 val = 0;
                    if ( incoming_byte_count % 19 )
                        val = 1;
                    incoming_package_count = (incoming_byte_count/19) + val;
                    m_hc_vec = QVector<huge_chunk_indexed_byterray_t> ( incoming_package_count );
                    qDebug()<<"HC -> Calculated Pkg Count"<< incoming_package_count;

                    hc_highest_index = 0;
                    first_multi_chunk = true;
                    m_missed_to_request = 0;
                }
                // MONGO TODO: data[6] -> channel count -> if > 1 -> set huge chunk multi mode flag

                // MONGO TODO: huge chunk write pointer in MCU schicken, arg3 -> channel count

                m_refToFileHandler->add_to_log_fil(m_ident_str, QString("Type"), QString::number(incoming_type));
                m_refToFileHandler->add_to_log_fil(m_ident_str, QString("ByteCountToReceive"), QString::number(incoming_byte_count));
                m_refToFileHandler->add_to_log_fil(m_ident_str, QString("WritePointer"), QString::number(tmp_write_pointer));

                setInfo("Incoming!");

                break;

            case HUGE_CHUNK_FINISH:
                if (m_multi_chunk_mode)
                {
                    //                    for ()
                    // iterate through the vec and check where we missed, implement request reply message to request missing chunks..
                    qDebug()<<"Multichunk transfer finished..";
                }
                else
                {
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
                }
                break;
                //
            case SENSORDATA_AVAILABLE:
                qDebug()<<"Sensor Data can be downloaded!";
                setInfo("Sensordata available!");
                emit sensorDataAvailable();
                setConnParams(7.5, 7.5);
                qDebug()<<"SETTING PARAMS!";
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

            case HUGE_CHUNK_ACK_PROC:
                if ( data[1] == HC1_BEGIN )
                {
                    qDebug()<<"HUGE_CHUNK_ACK_PROC : started!";
                    qDebug()<<"vecsize!"<<                    m_hc_vec.size();
                    qDebug()<<"HUGE_CHUNK_ACK_PROC : started!";

                    if ( m_hc_vec.size() != incoming_package_count )
                    {
                        qDebug()<<"HUGE_CHUNK_ACK_PROC : arrived_package_count != incoming_package_count!";

                        for ( int i = 0; i < m_hc_vec.size(); i++)
                        {
                            if ( !m_hc_vec.at(i).received )
                            {
                                qDebug()<<"Adding"<<i;
                                m_missed_to_request++; //to rst later
                                m_hc_missed.append(i); //to rst later
                            }
                        }
                        qDebug()<<"HUGE_CHUNK_ACK_PROC : need to request"<<m_hc_missed.size()<<"packages ;( ;";
                    }

                    qDebug()<<"m_missed_to_request"<<m_missed_to_request;

                    if ( m_missed_to_request == 0 )
                    {
                        setInfo("All Received!");
                        qDebug()<<"All Received";
                        m_refToFileHandler->add_to_log_fil(m_ident_str, QString("AllArrived"), QString("TRUE"));

                        state = CMD_STATE;
                        ackHugeChunk();
                        parse_n_write_received_pool( tmp_write_pointer, incoming_type);
                    }
                    else
                    {
                        qDebug()<<"Requesting missed";
                        state = REQUESTING_MISSED_STATE;
                        requestMissingPackage();
                    }
                }
                break;
            case 0x0E:
                qDebug()<<"Unimplemented diag msg";
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
                    qDebug()<<"SWITCH TO CMD STATE";
                    // DEBUG MONGO
                    if ( !m_multi_chunk_mode )
                        m_refToFileHandler->write_type_to_file(m_ident_str, m_huge_chunk, incoming_type, tmp_write_pointer);
                }
            }
            else
            {
                m_huge_chunk.append(value);
            }
        }
        else if ( state == REQUESTING_MISSED_STATE )
        {
            if ( data[0] == HUGE_CHUNK_MISSED_PACKAGE )
            {
                qDebug()<<"Missed Package arived";

                tmp.barr.append( value.right(value.size() - 1) );
                tmp.received = 1;
                m_hc_vec.replace(m_missed_in_request , tmp );

                m_hc_missed.removeAt(m_missed_in_request); //added
                m_missed_to_request--; // added

                if (m_hc_missed.size())
                    requestMissingPackage();

                if (m_hc_missed.size())
                {
                    requestMissingPackage();
                }
                else
                {
                    state = CMD_STATE;
                    this->ackHugeChunk();
                    parse_n_write_received_pool( tmp_write_pointer, incoming_type);
                    // reset variables
                }
            }
        }
    }
    else //(c.uuid() != QBluetoothUuid(BLE_UART_TX_CHAR)) // TX CHAR OF THE SERVER
    {
//        uint8_t debugflag = 0;
//        for (int j = 0; j< BLE_UART_TX_POOL.size(); j++)
//            if (QBluetoothUuid( BLE_UART_TX_POOL.at(j) ) == c.uuid() )
//                debugflag = 1;

//        if (!debugflag)
//            qWarning()<<"!!!an unknown characteristic wrote";

        // RX POOL is Active - this happens only if the state is hcs.. we could also filter if we got from the right char type
        //        if ( ( state == HUGE_CHUNK_STATE ) && m_multi_chunk_mode )
        //        {
        // qDebug()<<"HC -> Receiving Packet.. current highest index: " <<hc_highest_index;
        tidx = data[0];

        if (first_multi_chunk && !tidx )
        {
            tidx = 0;
            first_multi_chunk = false;
            debugTimer.start();
        }
        else if ( tidx > hc_highest_index )
        {
            hc_highest_index = tidx;
        }
        else
        {
            do {
                tidx += ( 0xFF + 1 );
            } while (tidx < ( hc_highest_index - 100 ));
            // this is not as bullet proof as hell, but it would work as long the messages are aligned within 100
            hc_highest_index = tidx;
        }
        //  qDebug()<<"HC -> Received IDX" << data[0] << "Calculated IDX" << tidx;

        tmp.barr.append(value.size()-1, data[1]);

        tmp.received = 1;
        if (tidx < incoming_package_count)
            m_hc_vec.replace(tidx,tmp);
        else
            qWarning()<<"HC -> Received idx too high, would leak out of the vector and crash the system!";

        if (tidx > last_idx++)
        {
            qDebug()<<"HC -> Skipped !!! "<<last_idx+1;
        }

        if ( tidx == incoming_package_count - 1 )
        {
            //qDebug()<<"HC -> Last PKG!";
            elapsed = debugTimer.elapsed();
            kbyte_ps = 0;
            kbit_ps = 0;
            secs = (float) elapsed / 1000.0f;
            kbyte_ps = incoming_byte_count /  secs / 1000  ;
            kbit_ps = kbyte_ps * 8.0;
            qInfo()<<"HC -> Transfer of"<<incoming_byte_count<<"bytes took"<< elapsed<<"ms";
            qInfo()<<"HC -> Throughput of net data is" <<kbyte_ps<< "kbyte / s : "<<"or "<<kbit_ps<<"kbit/s";
            secs = (float) elapsed / 1000.0f;
            kbyte_ps = ( incoming_byte_count + incoming_package_count ) /  secs / 1000  ;
            kbit_ps = kbyte_ps * 8.0;
            qInfo()<<"HC -> Throughput of raw data is" <<kbyte_ps<< "kbyte / s : "<<"or "<<kbit_ps<<"kbit/s";

        }
        else if (tidx > incoming_package_count )
        {
            qWarning()<<"HC -> Too many?!";
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

void DeviceHandler::onConnectionParamUpdated(const QLowEnergyConnectionParameters &newParameters)
{
    qDebug()<<"CONN UPDATED!!"<<newParameters.latency()<<
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
    qInfo() << "Characteristic Written! - Payload: " << value;
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());
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

void DeviceHandler::onShutUpSet(bool shutUp)
{
        // add timeout timer..
        QByteArray tba;
        tba.resize(5);
        tba[0] = SET_SHUT_UP;
        tba[1] = shutUp;
        this->ble_uart_tx(tba);
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
#ifdef DEBUG_ALL_OUT
                printProperties(c.properties());
#endif
                if ( c.uuid() == QBluetoothUuid( BLE_UART_RX_CHAR) && ( c.properties() & QLowEnergyCharacteristic::WriteNoResponse || c.properties() & QLowEnergyCharacteristic::Write) )
                {
                    qDebug()<<"Write Characteristic Registered";
                    m_writeCharacteristic = c;
                    emit writeValidChanged();
                    if(c.properties() & QLowEnergyCharacteristic::WriteNoResponse)
                    {
                        m_writeMode = QLowEnergyService::WriteWithoutResponse;
                    }
                    else
                    {
                        m_writeMode = QLowEnergyService::WriteWithResponse;
                    }
                    update_conn_period(); // DEBUG
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

void DeviceHandler::requestMissingPackage()
{

        uint16_t i = m_hc_missed.at(0);
        m_hc_missed.removeAt(0);

        // add timeout timer..
        QByteArray tba;
        tba.resize(4);
        tba[0] = HUGE_CHUNK_ACK_PROC;
        tba[1] = HC_1_REQ;
        tba[2] = ( i >>  8 ) & 0xFF ;
        tba[3] =   i & 0xFF ;
        qDebug()<<"HC -> Requesting missed : "<<i;
        this->ble_uart_tx(tba);
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

void DeviceHandler::update_conn_period()
{
    quint8 conn_min_max =  m_ident_idx ? 6 : 10;
    // add timeout timer..
    QByteArray tba;
    tba.resize(5);
    tba[0] = SET_CONN_PERIOD;
    tba[1] = conn_min_max;
    tba[2] = conn_min_max;
    tba[3] = (500u << 8) & 0xFF;
    tba[4] = 500u & 0xFF;
    qDebug()<<"Sending Conn Period!() min max = "<< conn_min_max << "default timeout = 500";
    this->ble_uart_tx(tba);
}



void DeviceHandler::parse_n_write_received_pool (uint16_t tmp_write_pointer, uint8_t type )
{
    m_huge_chunk.clear();
    for (int i = 0; i < m_hc_vec.size(); i++)
    {
        m_huge_chunk.append( m_hc_vec.at(i).barr );

    }
    qDebug()<<"PARSER FINISHED, gathered:"<<m_huge_chunk.size();
    m_refToFileHandler->write_type_to_file(m_ident_str, m_huge_chunk, type, tmp_write_pointer);
}

void DeviceHandler::setConnParams(double min_peri, double max_peri)
{
    QLowEnergyConnectionParameters para;
    para.setLatency(2);
    para.setIntervalRange(min_peri, max_peri);
    para.setSupervisionTimeout(10000);
    m_control->requestConnectionUpdate(para);
}
