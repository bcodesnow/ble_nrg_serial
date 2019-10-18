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

#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QString>
#include <QObject>
#include <QBluetoothDeviceInfo>
#include "bluetoothbaseclass.h"

#define DEVICE_SELECTED 0x01

class DeviceInfo :public QObject//: public BluetoothBaseClass
{
    Q_OBJECT
    Q_PROPERTY(QString deviceName READ getName NOTIFY deviceInfoChanged)
    Q_PROPERTY(QString deviceAddress READ getAddress NOTIFY deviceInfoChanged)
    Q_PROPERTY(qint64 deviceFlags READ getDeviceFlags NOTIFY deviceFlagsChanged)
    Q_PROPERTY(qint32 deviceIndex READ getDeviceIndex NOTIFY deviceIndexChanged)
    Q_PROPERTY(QString deviceIdentifier READ getDeviceIdentifier NOTIFY deviceIdentifierChanged)
    Q_PROPERTY(QString deviceMainState READ getDeviceMainState NOTIFY deviceMainStateChanged)
    Q_PROPERTY(DeviceType deviceType READ getDeviceType NOTIFY deviceTypeChanged)

public:
    DeviceInfo(const QBluetoothDeviceInfo &device);
    DeviceInfo(DeviceInfo* t);
    enum DeviceType {
        Wearable,
        Stationary
    };
    Q_ENUM(DeviceType)

    void setDevice(const QBluetoothDeviceInfo &device);
    QString getName() const;
    QString getAddress() const;
    qint64 getDeviceFlags() const;
    qint32 getDeviceIndex() const;
    QString getDeviceIdentifier() const;
    QString getDeviceMainState() const;
    DeviceType getDeviceType() const;
    QBluetoothDeviceInfo getDevice() const;

    void setDeviceFlags(const qint64 &flags); // todo why is this qint64?
    void setDeviceIndex(qint32 idx);
    void setDeviceIdentifier(QString ident);
    void setDeviceMainState(QString state);
    void setDeviceType( DeviceType type);

    bool deviceIsInRequiredConnectionState;
    bool deviceIsTimeSynced;
    bool m_sensorDataWaitingForDownload;
    QBluetoothDeviceInfo m_device;

signals:

    void deviceMainStateChanged(QString deviceMainState);

    void deviceIdentifierChanged(QString deviceIdentifier);

    void deviceTypeChanged(DeviceType deviceType);

    void deviceIndexChanged(qint32 deviceIndex);

    void deviceFlagsChanged(qint64 deviceFlags);

    void deviceInfoChanged(QString deviceAddress);

private:
    qint64 m_deviceFlags;
    qint32 m_deviceIdx;
    QString m_deviceIdentifier;
    QString m_deviceMainState;
    DeviceType m_deviceType;
    qint32 m_deviceIndex;
    QString m_deviceAddress;
    QString m_deviceName;
};

#endif // DEVICEINFO_H
