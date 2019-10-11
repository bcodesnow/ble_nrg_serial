#include "devicehandler.h"

bool DeviceHandler::ble_uart_send_cmd_with_resp(const QByteArray &value, quint16 timeout, quint8 retry)
{
    if (cmd_resp_struct.cmd_timer.isActive())
        return true;
    cmd_resp_struct.last_cmd.clear();
    cmd_resp_struct.last_cmd.append(value);

    cmd_resp_struct.timeout = timeout;
    cmd_resp_struct.retry = retry;

    cmd_resp_struct.cmd_timer.singleShot(timeout, this, SLOT(onCmdTimerExpired()));

    if (value.size())
        m_service->writeCharacteristic(m_writeCharacteristic, value, QLowEnergyService::WriteWithoutResponse); /*  m_writeMode */
    return false;
}

void DeviceHandler::onCmdTimerExpired()
{
    if ( cmd_resp_struct.retry )
    {
        cmd_resp_struct.retry--;
        cmd_resp_struct.cmd_timer.singleShot(cmd_resp_struct.timeout, this, SLOT(onCmdTimerExpired()));
        m_service->writeCharacteristic(m_writeCharacteristic, cmd_resp_struct.last_cmd, QLowEnergyService::WriteWithoutResponse); /*  m_writeMode */
    }
    else
    {
        // ERROR
        qWarning()<<"CMD Timeout!";
    }
}

void DeviceHandler::ble_uart_send_cmd_ok()
{
    QByteArray tba;
    tba.resize(2);
    tba[0] = CMD_OK;
    tba[1] = 0xFF;
    ble_uart_send_cmd_with_resp(tba);
}

void DeviceHandler::ble_uart_tx(const QByteArray &value)
{
    if (value.size())
        m_service->writeCharacteristic(m_writeCharacteristic, value, QLowEnergyService::WriteWithResponse); /*  m_writeMode */
}

inline bool DeviceHandler::isDeviceInRequestedConnState()
{
    return ( ( m_dev_conn_param_info.current_mode == m_dev_requested_conn_mode ) && ( m_dev_conn_param_info.requested_mode == m_dev_requested_conn_mode ));
}

void DeviceHandler::ble_uart_rx(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    // it would have been the best to only move the send receive functions to an own thread.. with this solution we will have a bit faster dispatch of time sync and receive..

    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());
    static const QBluetoothUuid ble_uart_receive = QBluetoothUuid(BLE_UART_TX_CHAR); // TX CHAR OF THE SERVER

    if ( c.uuid() == ble_uart_receive )
    {
        switch ( data[0] )
        {
        case CMD_OK:
        {
            cmd_resp_struct.cmd_timer.stop();
            qDebug()<<"CMD Response: OK!"<<cmd_resp_struct.last_cmd.at(0);
        }
            break;
        case TRIGGERED:
        {
            //inform the other device
            emit requestDispatchToOtherDevices(value, m_ident_idx); // TODO this will be received by the catch controller

//            if (m_refToOtherDevice != NULL)
//                m_refToOtherDevice->ble_uart_tx(value); // 1-1 forward it! - Write with Response as a CMD?!
            //
//            if (m_sdEnabled)
//                m_refToFileHandler->add_to_log_fil(m_ident_str,"File ID on Device", QString::number(data[1]));

            //ble_uart_send_cmd_ok(); // Should we also ACK this to the device?!
            emit triggeredArrived(); // TODO this is handled by the deviceinterface
        }
            break;

        case DATA_SAVED_TO_SD:
        {
            // confirm if it was a catch or drop -> we should give it to a class "above" -> catch controller
            qDebug()<<"Writing data to SD finished, waiting for confimation!";
            // send CMD_WRITE_CATCH_SUCC
            setInfo("Waiting for Confirmation!");
        }
            break;

        case ALIVE:
        {
            emit aliveArrived(value);
        }
            break;

        case REPLY_START_HUGE_CHUNK:
        {
            huge_chunk_start_t* hc_transfer_struct_ptr;
            hc_transfer_struct_ptr = (huge_chunk_start_t*) &data[1];
            hc_transfer_struct = *hc_transfer_struct_ptr;
            emit startHugeChunkArrived();
        }
            break;

        case SENSORDATA_AVAILABLE:
        {
            qDebug()<<"Sensor Data avialable for download!";
            setInfo("Sensordata available!");
            emit sensorDataAvailableArrived();
        }
            break;
            //
        case SENDING_SENSORDATA_FINISHED:
        {
            emit sensorDataReceived(); // qml uses this..to stop blinking the button..change name later!

            qDebug()<<"SENDING_SENSORDATA_FINISHED && allowing device to speak -> setShutUp(0) -> commented out";
            //                setShutUp(0);
            //                m_refToOtherDevice->setShutUp(0);
            setInfo("Sensordata Received!");
        }
            break;
        case TS_MSG:
        {
            emit timeSyncMessageArrived(value);
            //m_refToTimeStampler->time_sync_msg_arrived(value);
        }
            break;

        case CMD_START_HUGE_CHUNK_ACK_PROC:
        {
            emit startHugeChunkAckProcArrived(value);
        }
            break;

        case REPLY_MISSED_PACKAGE:
        {
            emit replyMissingPackageArrived();
        }
            break;

        case DIAG_INFO:
        {
            if (data[1] == DIAG_1_TYPE_LENGTH_TEST)
            {
                qDebug()<<"MESS LEN:::"<<value.size();
            }
            else
            {
                qDebug()<<"Unimplemented diag msg";
            }
        }
            break;
            //
        case CONN_PARAM_INFO:
        {
            // i should have implemented all packages this way..
            conn_param_info_t* cp_ptr;
            cp_ptr = (conn_param_info_t*) &data[1];
            m_dev_conn_param_info = *cp_ptr;
            emit connParamInfoArrived();
        }
            break;
            //
        default:
            qWarning()<<"Unknown MSG: "<<value;
            break; //technically not needed
        }
    }

    else //(c.uuid() != QBluetoothUuid(BLE_UART_TX_CHAR)) -> One of the TX_POOL Characteristics
    {
        // this is the ble rx pooled part.. it could be also moved to an own handler
        uint16_t tidx;
        huge_chunk_indexed_byterray_t hc_tmp_iba_struct;

        tidx = data[0];

        if (hc_helper_struct.first_multi_chunk && !tidx )
        {
            tidx = 0;
            hc_helper_struct.first_multi_chunk = false;
            m_debugTimer.start();
        }
        else if ( tidx > hc_helper_struct.hc_highest_index )
        {
            hc_helper_struct.hc_highest_index = tidx;
        }
        else
        {
            do {
                tidx += ( 0xFF + 1 );
            } while (tidx < ( hc_helper_struct.hc_highest_index - 100 ));
            // this is not as bullet proof as hell, but it would work as long the messages are aligned within 100
            hc_helper_struct.hc_highest_index = tidx;
        }
        //  qDebug()<<"HC -> Received IDX" << data[0] << "Calculated IDX" << tidx;

        hc_tmp_iba_struct.barr.append(value.size()-1, data[1]);
        hc_tmp_iba_struct.received = 1;

        if (tidx < hc_transfer_struct.incoming_package_count)
            m_hc_vec.replace(tidx,hc_tmp_iba_struct);
        else
        {
            // We might show the devil
            qWarning()<<"HC -> Received idx too high, would leak out of the vector and crash the system!";
        }
        if (tidx > hc_helper_struct.last_idx++)
        {
            qDebug()<<"HC -> Skipped !!! "<<hc_helper_struct.last_idx+1;
        }

        if ( tidx == hc_transfer_struct.incoming_package_count - 1 )
        {
            qDebug()<<"HC -> Last PKG!";
            printThroughput();
        }
        else if (tidx > hc_transfer_struct.incoming_package_count )
        {
            qWarning()<<"HC -> Too many?!";
        }
    }
}


void DeviceHandler::onAliveArrived(QByteArray value)
{
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());
    m_deviceState = state_to_string(data[1]);
    m_fileIndexOnDevice = data[3];
    m_deviceSubState = data[2];
    m_deviceLastError = data[4];

    if (m_deviceLastError)
        m_refToFileHandler->add_to_log_fil(m_ident_str,"Last Error", QString(data[4]));

    qDebug()<<m_ident_str<<" --- ALIVE: -STATE- "<<m_deviceState<<" -SUB STATE- "<<m_deviceSubState<<" -LAST ERROR- "<<m_deviceLastError;

    // TODO: if SD
    m_sdEnabled = data[5];
//    emit sdEnabledChanged();
//    emit fileIndexOnDeviceChanged();
    emit deviceStateChanged();
    setInfo("Alive!");
}

void DeviceHandler::requestMissingPackage()
{
    request_missing_pkg_t req;
    req.pkg_id = m_hc_missed.at(0);
    m_hc_missed.removeAt(0);

    QByteArray tba;
    tba.append((const char*) &req, sizeof(request_missing_pkg_t));
    qDebug()<<"HC -> Requesting missed : "<<req.pkg_id;
    this->ble_uart_send_cmd_with_resp(tba);
}


void DeviceHandler::onReplyMissingPackageArrived(QByteArray value)
{
    qDebug()<<"Missed Package arived";
    m_timeoutTimer.stop();

    huge_chunk_indexed_byterray_t tmp;
    tmp.barr.append( value.right(value.size() - 1) );
    tmp.received = 1;
    m_hc_vec.replace( hc_helper_struct.missed_in_request , tmp );

    m_hc_missed.removeAt( hc_helper_struct.missed_in_request); //added
    hc_helper_struct.missed_to_request--; // added

    if (m_hc_missed.size())
        requestMissingPackage();

    if (m_hc_missed.size())
    {
        requestMissingPackage();
    }
    else
    {
        this->ackHugeChunk();
        parse_n_write_received_pool( hc_transfer_struct.write_pointer, hc_transfer_struct.incoming_type );
        // reset variables
    }
}


void DeviceHandler::onStartHugeChunkArrived()
{
    quint8 val = 0;

    if ( hc_transfer_struct.incoming_byte_count )
    {
        qDebug()<<"HC -> Multi Chunk Transfer Starts! "<< hc_transfer_struct.incoming_byte_count << " Bytes to Receive!";
    }
    else
    {
        // show the devil
    }

    //if ( hc_transfer_struct.incoming_byte_count % 19 )
    //   val = 1;
    //incoming_package_count = (incoming_byte_count/19) + val;

    m_hc_vec = QVector<huge_chunk_indexed_byterray_t> ( hc_transfer_struct.incoming_package_count );
    qDebug()<<"HC -> Pkg Count"<< hc_transfer_struct.incoming_package_count;
    hc_helper_struct.hc_highest_index = 0;
    hc_helper_struct.first_multi_chunk = true;
    hc_helper_struct.missed_to_request = 0;

    // TODO this is not threadsafe..
    emit add_to_log_fil_sig(m_ident_str, QString("Type"), QString::number(hc_transfer_struct.incoming_type));
    emit add_to_log_fil_sig(m_ident_str, QString("ByteCountToReceive"), QString::number(hc_transfer_struct.incoming_byte_count));
    emit add_to_log_fil_sig(m_ident_str, QString("WritePointer"), QString::number(hc_transfer_struct.write_pointer));

    ble_uart_send_cmd_ok();

    setInfo("Incoming!");

}



void DeviceHandler::onStartHugeChunkAckProcArrived(QByteArray value)
{
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());

    qDebug()<<"HUGE_CHUNK_ACK_PROC : started!";
    qDebug()<<"vecsize!"<<                    m_hc_vec.size();
    qDebug()<<"HUGE_CHUNK_ACK_PROC : started!";

    if ( m_hc_vec.size() != hc_transfer_struct.incoming_package_count )
    {
        qDebug()<<"HUGE_CHUNK_ACK_PROC : arrived_package_count != incoming_package_count!";

        for ( int i = 0; i < m_hc_vec.size(); i++)
        {
            if ( !m_hc_vec.at(i).received )
            {
                qDebug()<<"Adding"<<i;
                hc_helper_struct.missed_to_request++;
                m_hc_missed.append(i); //to rst later
            }
        }
        qDebug()<<"HUGE_CHUNK_ACK_PROC : need to request"<<m_hc_missed.size()<<"packages ;( ;";
    }

    qDebug()<<"m_missed_to_request"<<hc_helper_struct.missed_to_request;

    if ( hc_helper_struct.missed_to_request == 0 )
    {
        setInfo("All Received!");
        qDebug()<<"All Received";
        emit add_to_log_fil_sig(m_ident_str, QString("AllArrived"), QString("TRUE"));

        ackHugeChunk();
        parse_n_write_received_pool( hc_transfer_struct.write_pointer, hc_transfer_struct.incoming_type);
    }
    else
    {
        qDebug()<<"Requesting missed";
        requestMissingPackage();
    }
}


void DeviceHandler::onConnParamInfoArrived()
{

    qDebug()<<"PARA_INFO_MSG:"<<m_ident_idx<<m_ident_str<<"from dev"<<
              m_dev_conn_param_info.latency<<m_dev_conn_param_info.interval<<
              m_dev_conn_param_info.current_mode<<m_dev_conn_param_info.requested_mode;

    if (isDeviceInRequestedConnState())
    {
        m_connParamTimer.stop();

        //make the necessary shutup!!
        qDebug()<<"Dev also made the necessary changes and switched!";

        //continue with requesting sensor data.. if the second device also reached its requested state..
        // todo replace checking the connection mode flag with checking app state...
        // MODIFY AND TEST IF ITS ENOUGH IF ONE DEV HAS IDEAL CONDITIONS
        if (downloading_sensor_data_active && m_refToOtherDevice->isDeviceInRequestedConnState() )
        {
            qDebug()<<"Other device is also ready -> go on!";


            requestSensorData();
        }

    }
    else
    {
        setConnParamsOnCentral(m_dev_requested_conn_mode);
    }
}

void DeviceHandler::onConnParamTimerExpired()
{
    // TODO here it is
}

void DeviceHandler::onSensorDataAvailableArrived()
{
    // todo this is something qml could jump on again..
}

// TODO
//m_refToOtherDevice->setRequestedConnParamsOnDevice(SLOW);
//m_refToOtherDevice->setConnParamsOnCentral(SLOW);
