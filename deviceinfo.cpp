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

#include "deviceinfo.h"
#include <QBluetoothAddress>
#include <QBluetoothUuid>

DeviceInfo::DeviceInfo(const QBluetoothDeviceInfo &info):
    BluetoothBaseClass(), m_device(info)
{
    m_deviceFlags = 0;
}

DeviceInfo::DeviceInfo(DeviceInfo *t)
{
    deviceIsInRequiredConnectionState = t-> deviceIsInRequiredConnectionState;
    deviceIsTimeSynced  = t->deviceIsTimeSynced;
    m_sensorDataWaitingForDownload = t-> m_sensorDataWaitingForDownload;
    m_device = t->getDevice();
    m_deviceFlags = t->getDeviceFlags();
    m_deviceIdx = t->getDeviceIndex();
    m_deviceIdentifier = t->getDeviceIdentifier();
    m_deviceMainState = t->getDeviceMainState();
    m_deviceType = t->getDeviceType();
}

QBluetoothDeviceInfo DeviceInfo::getDevice() const
{
    return m_device;
}

void DeviceInfo::setDeviceFlags(const qint64 &flags)
{
    m_deviceFlags = flags;
    emit deviceChanged();
}

void DeviceInfo::setDeviceIndex(qint32 idx)
{
    m_deviceIdx = idx;
    emit deviceChanged();
}

void DeviceInfo::setDeviceIdentifier(QString ident)
{
    m_deviceIdentifier = ident;
}

void DeviceInfo::setDeviceMainState(QString state)
{
    m_deviceMainState = state;
}

void DeviceInfo::setDeviceType(DeviceInfo::DeviceType type)
{
    m_deviceType = type;
}

QString DeviceInfo::getName() const
{
    return m_device.name();
}

QString DeviceInfo::getAddress() const
{
    return m_device.address().toString();
}

qint64 DeviceInfo::getDeviceFlags() const
{
    return m_deviceFlags;
}

qint32 DeviceInfo::getDeviceIndex() const
{
    return m_deviceIdx;
}

QString DeviceInfo::getDeviceIdentifier() const
{
    return m_deviceIdentifier;
}

QString DeviceInfo::getDeviceMainState() const
{
    return m_deviceMainState;
}

DeviceInfo::DeviceType DeviceInfo::getDeviceType() const
{
    return m_deviceType;
}

void DeviceInfo::setDevice(const QBluetoothDeviceInfo &device)
{
    m_device = device;
    emit deviceChanged();
}
