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

#include "connectionhandler.h"
#include <QtBluetooth/qtbluetooth-config.h>

ConnectionHandler::ConnectionHandler(QObject *parent) : QObject(parent)
{
    qDebug()<<"Connection Handler -> hostModeStateChanged of QBluetoothLocalDevice";
    connect(&m_localDevice, &QBluetoothLocalDevice::hostModeStateChanged,
            this, &ConnectionHandler::hostModeChanged);

     qDebug()<<"Fetching Adapter List";
     m_adapterList = QBluetoothLocalDevice::allDevices();
     qDebug()<<"Adapter List Size: "<< m_adapterList.size();
}

bool ConnectionHandler::alive() const
{
    return m_localDevice.isValid() && m_localDevice.hostMode() != QBluetoothLocalDevice::HostPoweredOff;
}

bool ConnectionHandler::requiresAddressType() const
{
#if QT_CONFIG(bluez)
    return true;
#else
    return false;
#endif
}

QString ConnectionHandler::name() const
{
    return m_localDevice.name();
}

QString ConnectionHandler::address() const
{
    return m_localDevice.address().toString();
}

void ConnectionHandler::hostModeChanged(QBluetoothLocalDevice::HostMode /*mode*/)
{
    emit deviceChanged();
}

//QVariant ConnectionHandler::adapters()
//{
//    return QVariant::fromValue(m_adapters);
//}

//QList<QBluetoothAddress> ConnectionHandler::getAdaptersAddr()
//{
//    QList<QBluetoothAddress> tmplist;
//    tmplist.clear();
//    for (int i=0;i<m_adapters.size();i++) {
//        tmplist.append(((AdapterInfo*)m_adapters.at(i))->getAddress());
//    }
//    return tmplist;
//}

//void ConnectionHandler::addAdapter(QString name, QBluetoothAddress addr)
//{
//    qDebug()<<"BT-Adapter added:"<<name<<addr;
//    m_adapters.append(new AdapterInfo(name, addr));
//}

//void ConnectionHandler::scanAdapters()
//{
//    m_adapters.clear();
//    QList<QBluetoothHostInfo> adapterList = QBluetoothLocalDevice::allDevices();
//    for (int i=0;i<adapterList.size();i++) {
//        addAdapter(adapterList.at(i).name(),adapterList.at(i).address());
//    }
//}

//void ConnectionHandler::initBtAdapters(QBluetoothAddress &leftaddr, QBluetoothAddress &rightaddr)
//{
//    scanAdapters();
//    QList<QBluetoothAddress> adapterAddrList = this->getAdaptersAddr();

//    int adapterCount = adapterAddrList.size();
//    if (adapterCount == 1) {
//        // set address to both handles
//        leftaddr = rightaddr = adapterAddrList.at(0);
//    }
//    else if (adapterCount > 1) {
//        // set first two addresses to handles
//        leftaddr = adapterAddrList.at(0);
//        rightaddr = adapterAddrList.at(1);
//    }
//    else {
//        // none adapters found, which means QBluetoothLocalDevice is not supported on OS
//    }
//}


