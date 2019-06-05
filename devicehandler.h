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

#include <QDateTime>
#include <QVector>
#include <QTimer>
#include <QLowEnergyController>
#include <QLowEnergyService>


/* Simple Protocol: */

/*1st byte:
    CMD BYTE
**/
#define TRIGGERED     			0x06
#define GET_STATE		  			0x03
#define STOP				  			0x02
#define START				  			0x01
#define IGNORE_LAST_X	  		0x07
#define DATA_COLLECTED  		0x08
#define WRITE_CATCH_SUCCESS 0x09
#define ALIVE								0x10
#define HUGE_CHUNK_START		0x55
#define	HUGE_CHUNK_FINISH 	0x77
#define TURN_ON_SD_LOGGING  0x66

/* IGNORE_LAST_X 		-> byte[1] how many to ignore */

/* ALIVE 							-> byte[1] mainState
                                            -> byte[2] subState
                                            -> byte[3] fileIndex
                                            -> byte[4] lastError
*/

/* HUGE_CHUNK_START		-> byte[1] bytesToSend HBYTE
                                            -> byte[2] bytesToSend LBYTE
                                            -> byte[3] TYPE
                                            -> byte[4] Used Charactertics as Channels (count)
*/
#define TYPE_AUD 1u
#define	TYPE_ACC 2u
#define TYPE_GYR 3u
#define TYPE_MAG 4u
#define TYPE_PRS 5u

/* HUGE_CHUNK_FINISH	-> byte[1] maxRepeatCount */

/* TURN_ON_SD_LOGGING	-> byte[1] ON / OFF */

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

class DeviceHandler : public BluetoothBaseClass
{
    Q_OBJECT

    Q_PROPERTY(bool alive READ alive NOTIFY aliveChanged)
    Q_PROPERTY(AddressType addressType READ addressType WRITE setAddressType)
    Q_PROPERTY(QString deviceAddress MEMBER m_deviceAddress NOTIFY deviceAddressChanged)
    Q_PROPERTY(QString deviceState MEMBER m_deviceState NOTIFY deviceStateChanged)
    Q_PROPERTY(qint16 fileIndexOnDevice MEMBER m_fileIndexOnDevice NOTIFY fileIndexOnDeviceChanged)



public:
    Q_INVOKABLE void sendCMDStringFromTerminal(const QString &str);

    enum class AddressType {
        PublicAddress,
        RandomAddress
    };
    Q_ENUM(AddressType)

    DeviceHandler(QObject *parent = 0);

    void setDevice(DeviceInfo *device);
    void setAddressType(AddressType type);
    AddressType addressType() const;

    void setRefToOtherDevice (DeviceHandler* t_dev_handler);

    bool alive() const;
    void ble_uart_tx(const QByteArray &value);

signals:
    void aliveChanged();
    void deviceAddressChanged();
    void deviceStateChanged();
    void fileIndexOnDeviceChanged();

public slots:
    void disconnectService();

private slots:
    void onCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void onCharacteristicRead(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void onCharacteristicWritten(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void onTimerTriggered(void);


private:
    //QLowEnergyController
    void serviceDiscovered(const QBluetoothUuid &);
    void serviceScanDone();

    //QLowEnergyService
    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void ble_uart_rx(const QLowEnergyCharacteristic &c, const QByteArray &value);
    QString state_to_string(uint8_t tmp);


    // PayloadLength Max 1 CHAR_MAX_LEN-1= 19
//    void sendCMDwaitforReply(quint8 cmd, quint8* payload, int payloadLength)
//    {
//        replyDelayTimer->setSingleShot(true);
//    }

    void confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value);
    void update_currentService();

    void searchCharacteristic();
    void printProperties(QLowEnergyCharacteristic::PropertyTypes);

    QLowEnergyController *m_control;
    QLowEnergyService *m_service;
    QString m_deviceAddress;
    QString m_deviceState;
    qint16 m_fileIndexOnDevice;
    quint8 m_deviceSubState;
    quint8 m_deviceLastError;

    DeviceInfo *m_currentDevice;
    QLowEnergyDescriptor m_notificationDescriptor;

    DeviceHandler* m_refToOtherDevice;

    const QString BLE_UART_RX_CHAR = "{d973f2e2-b19e-11e2-9e96-0800200c9a66}";
    const QString BLE_UART_TX_CHAR = "{d973f2e1-b19e-11e2-9e96-0800200c9a66}";
    const QString BLE_UART_SERVICE = "{d973f2e0-b19e-11e2-9e96-0800200c9a66}";

    bool m_found_BLE_UART_Service;

    QLowEnergyCharacteristic m_writeCharacteristic;
    QLowEnergyService::WriteMode m_writeMode;
    QLowEnergyCharacteristic m_readCharacteristic;

    QTimer* m_test_timer;
    QTimer* replyDelayTimer;

    QLowEnergyController::RemoteAddressType m_addressType = QLowEnergyController::PublicAddress;
};

#endif // DEVICEHANDLER_H
