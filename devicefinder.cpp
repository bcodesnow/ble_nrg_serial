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

/*
 *  Uses QBluetoothDeviceDiscoveryAgent with 5000ms Timeout Filter for BLE Devices only
 *  Device Discovered calls addDevice Slot other outputs end in scanFinished or scanError
 *  Added Devices end up in the m_devices QList<QObject*>DeviceInfo
 *
 *
 */

#include "devicefinder.h"
#include "ble_uart.h"
#include "devicehandler.h"
#include "deviceinfo.h"
#include "QThread"
#include <QDebug>


DeviceFinder::DeviceFinder(QList<DeviceInterface*>* devicelist, ConnectionHandler* connHandler, TimeSyncHandler* ts_handler,
                           CatchController* catch_controller, LogFileHandler* logfile_handler, QObject *parent):
    BluetoothBaseClass(parent),
    m_device_list(devicelist),
    m_conn_handler_ptr(connHandler),
    m_timesync_handler_ptr(ts_handler),
    m_logfile_handler_ptr(logfile_handler),
    m_catch_controller_ptr(catch_controller)

{
    m_selectedDevicesCount = 0;
    //! [devicediscovery-1]
    m_deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    m_deviceDiscoveryAgent->setLowEnergyDiscoveryTimeout(5000);

    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &DeviceFinder::addDevice);
    connect(m_deviceDiscoveryAgent, static_cast<void (QBluetoothDeviceDiscoveryAgent::*)(QBluetoothDeviceDiscoveryAgent::Error)>(&QBluetoothDeviceDiscoveryAgent::error),
            this, &DeviceFinder::scanError);

    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &DeviceFinder::scanFinished);
    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled, this, &DeviceFinder::scanFinished);
    //! [devicediscovery-1]
}

DeviceFinder::~DeviceFinder()
{
    qDeleteAll(m_found_devices);
    m_found_devices.clear();
}

void DeviceFinder::startSearch()
{
    qDebug()<<"DeviceFinder -> Starting Search";
    clearMessages();
    if (m_device_list->size() != 0)
        qDeleteAll(m_device_list->begin(), m_device_list->end());

    m_device_list->clear();

    emit devicesChanged();

    //! [devicediscovery-2]
    m_deviceDiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    //! [devicediscovery-2]
    emit scanningChanged();
    setInfo(tr("Scanning for devices..."));
    qDebug()<<"DeviceFinder -> QBluetoothDeviceDiscoveryAgent searching LE";
}

//! [devicediscovery-3]
void DeviceFinder::addDevice(const QBluetoothDeviceInfo &device)
{
    // If device is LowEnergy-device, add it to the list
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        m_found_devices.append(new DeviceInfo(device));
        setInfo(tr("Low Energy device found. Scanning more..."));
        qDebug()<<"Low Energy device found. Scanning more..."<<device.address().toString();
        //! [devicediscovery-3]
        emit devicesChanged();
        //! [devicediscovery-4]
    }
    //...
}
//! [devicediscovery-4]

void DeviceFinder::scanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    qDebug()<<"Scan ERROR";
    if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError)
        setError(tr("The Bluetooth adaptor is powered off."));
    else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError)
        setError(tr("Writing or reading from the device resulted in an error."));
    else
        setError(tr("An unknown error has occurred."));
}

void DeviceFinder::scanFinished()
{
    if (m_found_devices.size() == 0)
        setError(tr("No Low Energy devices found."));
    else
        setInfo(tr("Scanning done."));

    emit scanningChanged();
    emit devicesChanged();

    qDebug()<<"Found BLE Devices Count:"<<m_found_devices.size();
}


void DeviceFinder::connectToSelectedDevices()
{
    int adapterListSize = m_conn_handler_ptr->m_adapterList.size();
    int deviceListSize = m_device_list->size();
    int k = 0;

    qDebug()<<"We have "<< adapterListSize <<"adapters detected";

    for (int i = 0; i < m_found_devices.size(); i++)
    {
        if  (((DeviceInfo*)m_found_devices.at(i))->getDeviceFlags() & DEVICE_SELECTED )
        {
            m_device_list->append(new DeviceInterface(m_timesync_handler_ptr, m_catch_controller_ptr, m_logfile_handler_ptr, deviceListSize));
            deviceListSize++;
            m_device_list->last()->init_device( &m_conn_handler_ptr->m_adapterList[k], (DeviceInfo*) m_found_devices[i] );
            k++;
            if ( k >= adapterListSize)
                k = 0;
        }
    }
    qDebug()<<"Connection started to"<<m_selectedDevicesCount<<"from"<<m_found_devices.size()<<"found devices..";
    clearMessages();
}



bool DeviceFinder::scanning() const
{
    return m_deviceDiscoveryAgent->isActive();
}

QVariant DeviceFinder::devices()
{
    return QVariant::fromValue(m_found_devices);
}

void DeviceFinder::addDeviceToSelection(const quint8 &idx)
{
    if ( m_selectedDevicesCount < 2 )
    {
        ((DeviceInfo*) m_found_devices.at(idx) )->setDeviceFlags( ( (DeviceInfo*) m_found_devices.at(idx) )->getDeviceFlags() | DEVICE_SELECTED);
        m_selectedDevicesCount++;
        emit ((DeviceInfo*) m_found_devices.at(idx) )->deviceChanged();
    }
}

void DeviceFinder::removeDeviceFromSelection(const quint8 &idx)
{
    if ( idx < m_found_devices.size() )
    {
        ((DeviceInfo*) m_found_devices.at(idx) )->setDeviceFlags( 0 );
        m_selectedDevicesCount--;
        emit ((DeviceInfo*) m_found_devices.at(idx) )->deviceChanged();
    }
}

// TODO
void DeviceFinder::sendConfirmationToBothDevices(const quint8 &success)
{
    // TODO THIS IS NOT THREADSAFE
    QByteArray tba;
    tba.resize(2);
    tba[0] = CMD_WRITE_CATCH_SUCCESS;
    tba[1] = success;

    // there is no public method..
    //    if (m_initializedDevicesList[0] == true)
    //        m_deviceHandler[0].sendCMDStringFromTerminal(tba);
    //    if (m_initializedDevicesList[1] == true)
    //        m_deviceHandler[1].ble_uart_tx(tba);
}

// TODO
void DeviceFinder::sendRestartToBothDevices()
{
    //    QByteArray tba;
    //    tba.resize(1);
    //    tba[0] = START;
    //    if (m_initializedDevicesList[0] == true)
    //        m_deviceHandler[0].ble_uart_tx(tba);
    //    if (m_initializedDevicesList[1] == true)
    //        m_deviceHandler[1].ble_uart_tx(tba);
}

// TODO
void DeviceFinder::sendEnableSDtoBothDevices(bool enable)
{
    //    QByteArray tba;
    //    tba.resize(2);
    //    tba[0] = TURN_ON_SD_LOGGING;
    //    tba[1] = enable;
    //    if (m_initializedDevicesList[0] == true)
    //        m_deviceHandler[0].ble_uart_tx(tba);
    //    if (m_initializedDevicesList[1] == true)
    //        m_deviceHandler[1].ble_uart_tx(tba);
}

