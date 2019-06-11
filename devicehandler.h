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

#ifndef DEVICEHANDLER_H
#define DEVICEHANDLER_H

#include "bluetoothbaseclass.h"
#include "logfilehandler.h"
#include "timestampler.h"

#include <QDateTime>
#include <QVector>
#include <QTimer>
#include <QLowEnergyController>
#include <QLowEnergyService>

#define CDSM_STATE_INIT													( 0u )
#define CDSM_STATE_RUNNING                                              (	1u << 0u )
#define CDSM_STATE_READY_TO_BE_TRIGGERED                                ( 1u << 1u )	/* There is already data for 1 sec in the buffer. */
#define CDSM_STATE_TRIGGERED                                            (	1u << 2u )
#define CDSM_STATE_POST_TRIGGER_DATA_COLLECTED                          ( 1u << 3u )  /* The needed amount of data after the trigger has been collected, lets save it to the SD card. */
#define CDSM_STATE_STOPPING                                             ( 1u << 4u )
#define CDSM_STATE_STOPPED                                              ( 1u << 5u )
#define CDSM_STATE_RESTARTING                                           ( 1u << 6u )
#define CDSM_STATE_ERROR 												( 1u << 7u )
#define CDSM_STATE_COLLECT_DATA                                         (	CDSM_STATE_RUNNING | CDSM_STATE_READY_TO_BE_TRIGGERED | CDSM_STATE_TRIGGERED )

#define CDSM_SUBSTATE_STOPPING                                  0u
#define	CDSM_SUBSTATE_SAVING_AUDIO                              1u
#define CDSM_SUBSTATE_SAVING_MAGNETO                            2u
#define CDSM_SUBSTATE_SAVING_PRESSURE                           3u
#define CDSM_SUBSTATE_SAVING_ACC                                4u
#define CDSM_SUBSTATE_SAVING_GYRO                               5u
#define CDSM_SUBSTATE_SENDING_DATA_COLLECTED                    6u


class DeviceInfo;
class TimeStampler;

class DeviceHandler : public BluetoothBaseClass
{
    Q_OBJECT

    Q_PROPERTY(bool alive READ alive NOTIFY aliveChanged)
    Q_PROPERTY(AddressType addressType READ addressType WRITE setAddressType)
    Q_PROPERTY(QString deviceAddress MEMBER m_deviceAddress NOTIFY deviceAddressChanged)
    Q_PROPERTY(QString deviceState MEMBER m_deviceState NOTIFY deviceStateChanged)
    Q_PROPERTY(bool sdEnabled MEMBER m_sdEnabled NOTIFY sdEnabledChanged)
    Q_PROPERTY(qint16 fileIndexOnDevice MEMBER m_fileIndexOnDevice NOTIFY fileIndexOnDeviceChanged)

private:
    const QString BLE_UART_RX_CHAR = "{d973f2e2-b19e-11e2-9e96-0800200c9a66}";
    const QString BLE_UART_TX_CHAR = "{d973f2e1-b19e-11e2-9e96-0800200c9a66}";
    const QString BLE_UART_SERVICE = "{d973f2e0-b19e-11e2-9e96-0800200c9a66}";

    QString m_ident_str;
    quint8 m_ident_idx;

    QByteArray m_huge_chunk;
    bool m_sdEnabled;
    QString m_deviceAddress;
    QString m_deviceState;
    qint16 m_fileIndexOnDevice;
    quint8 m_deviceSubState;
    quint8 m_deviceLastError;

    bool m_found_BLE_UART_Service;

    QLowEnergyController *m_control;
    QLowEnergyService *m_service;
    QLowEnergyController::RemoteAddressType m_addressType = QLowEnergyController::PublicAddress;

    DeviceInfo *m_currentDevice;
    QLowEnergyDescriptor m_notificationDescriptor;

    DeviceHandler* m_refToOtherDevice;
    LogFileHandler* m_refToFileHandler;
    TimeStampler* m_refToTimeStampler;

    QLowEnergyCharacteristic m_writeCharacteristic;
    QLowEnergyService::WriteMode m_writeMode;
    QLowEnergyCharacteristic m_readCharacteristic;

    //QLowEnergyController
    void serviceDiscovered(const QBluetoothUuid &);
    void serviceScanDone();

    //QLowEnergyService
    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void ble_uart_rx(const QLowEnergyCharacteristic &c, const QByteArray &value);
    QString state_to_string(uint8_t tmp);

    void confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value);
    void update_currentService();

    void searchCharacteristic();
    void printProperties(QLowEnergyCharacteristic::PropertyTypes);

public:
    DeviceHandler(QObject *parent = 0);

    Q_INVOKABLE void sendCMDStringFromTerminal(const QString &str);
    Q_INVOKABLE void requestBLESensorData(void);

    enum class AddressType {
        PublicAddress,
        RandomAddress
    };
    Q_ENUM(AddressType)

    void setDevice(DeviceInfo *device);
    void setAddressType(AddressType type);
    AddressType addressType() const;

    void setRefToOtherDevice (DeviceHandler* t_dev_handler);
    void setRefToFileHandler (LogFileHandler* t_fil_helper);
    void setRefToTimeStampler (TimeStampler* t_time_stampler);

    void setIdentifier(QString str, quint8 idx);

    bool alive() const;
    void ble_uart_tx(const QByteArray &value);

signals:
    void aliveChanged();
    void deviceAddressChanged();
    void deviceStateChanged();
    void fileIndexOnDeviceChanged();
    void aliveArrived();
    void sensorDataAvailable();
    void sensorDataReceived();
    void sdEnabledChanged();

public slots:
    void disconnectService();

private slots:
    void onCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void onCharacteristicRead(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void onCharacteristicWritten(const QLowEnergyCharacteristic &c, const QByteArray &value);

};

#endif // DEVICEHANDLER_H
