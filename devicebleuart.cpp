#include "devicecontroller.h"

bool DeviceController::bleUartSendCmdWithResp(const QByteArray &value, quint16 timeout, quint8 retry)
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

    qDebug()<<"bleUartSendCmdWithResp(const QByteArray &value, quint16 timeout, quint8 retry)";
    return false;
}

void DeviceController::onCmdTimerExpired()
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
        // is this a moment where the devil should appear?
        qWarning()<<"CMD Timeout!";
        if (cmd_resp_struct.last_cmd.at(0) == CMD_REQUEST_SENSORDATA)
            emit allDataDownloaded(false, m_ident_idx);
    }
}



void DeviceController::bleUartSendCmdOk()
{
    qDebug()<<"Sending CMD OK!";
    QByteArray tba;
    tba.resize(2);
    tba[0] = CMD_OK;
    tba[1] = 0xFF;
    bleUartTx(tba);
}

void DeviceController::bleUartTx(const QByteArray &value)
{
    qDebug()<<"bleUartTx(const QByteArray &value)";
    if (value.size())
        m_service->writeCharacteristic(m_writeCharacteristic, value, QLowEnergyService::WriteWithoutResponse); /*  m_writeMode todo -> I JUST CHANGED IT TO FIXED WITHOUT */
}

inline bool DeviceController::isDeviceInRequestedConnState()
{
    return ( ( m_dev_conn_param_info.current_mode == m_dev_requested_conn_mode ) && ( m_dev_conn_param_info.requested_mode == m_dev_requested_conn_mode ));
}

void DeviceController::bleUartRx(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    // it would have been the best to only move the send receive functions to an own thread.. with this solution we will have a bit faster dispatch of time sync and receive..

    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());
    static const QBluetoothUuid ble_uart_receive = QBluetoothUuid(BLE_UART_TX_CHAR); // TX CHAR OF THE SERVER

    QString hex= QString("%1").arg(data[0] , 0, 16);

    if ( c.uuid() == ble_uart_receive )
    {

        qDebug()<<"BLE UART RX FIRST BYTE:"<<hex;

        switch ( data[0] )
        {
        case CMD_OK:
            cmd_resp_struct.cmd_timer.stop();
            qDebug()<<"CMD OK Received..!";

            break;
        case TRIGGERED:

            //inform the other device
            emit requestDispatchToOtherDevices(value, m_ident_idx); // TODO this will be received by the catch controller
            //ble_uart_send_cmd_ok(); // Should we also ACK this to the device?!
            emit triggeredArrived(value); // TODO this is handled by the deviceinterface
            this->bleUartSendCmdOk();

            break;

        case DATA_SAVED_TO_SD:

            // todo!!!
            // confirm if it was a catch or drop -> we should give it to a class "above" -> catch controller
            qDebug()<<"Writing data to SD finished, waiting for confimation!";
            // send CMD_WRITE_CATCH_SUCC
            //setInfo("Waiting for Confirmation!");

            break;

        case ALIVE:

            emit aliveArrived(value);

            break;

        case REPLY_START_HUGE_CHUNK:

            huge_chunk_start_t* hc_transfer_struct_ptr;
            hc_transfer_struct_ptr = (huge_chunk_start_t*) &data[1];
            hc_transfer_struct = *hc_transfer_struct_ptr;
            emit startHugeChunkArrived();

            break;

        case REPLY_NO_CHUNK_AVAILABLE:

            emit noChunkAvailableArrived();

            break;

        case CMD_SENSORDATA_AVAILABLE:

            qDebug()<<"Sensor Data avialable for download!";
            emit sensorDataAvailableArrived(m_ident_idx);
            //bleUartSendCmdOk(); todo, the catch controller also sends ok?!

            break;
            //

        case TS_MSG:

            emit timeSyncMessageArrived(value);

            break;

        case CMD_START_HUGE_CHUNK_ACK_PROC:

            emit startHugeChunkAckProcArrived(value);

            break;

        case REPLY_MISSED_PACKAGE:

            emit replyMissingPackageArrived();

            break;

        case DIAG_INFO:

            if (data[1] == DIAG_1_TYPE_LENGTH_TEST)
            {
                qDebug()<<"MESS LEN:::"<<value.size();
            }
            else
            {
                qDebug()<<"Unimplemented diag msg";
            }

            break;
            //
        case CONN_PARAM_INFO:

            // i should have implemented all packages this way..
            conn_param_info_t* cp_ptr;
            cp_ptr = (conn_param_info_t*) &data[1];
            m_dev_conn_param_info = *cp_ptr;
            bleUartSendCmdOk();
            emit connParamInfoArrived();

            break;
            //
        default:
            qWarning()<<"Unknown MSG: "<<value;
            break; //technically not needed
        }

        // todo if cmd id -> send ack.. this could save us from some headache
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

        hc_tmp_iba_struct.barr.append(value.size()-1, data[1]); // add the 19 bytes
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



void DeviceController::sendRequestMissingPackage()
{
    request_missing_pkg_t req;
    req.pkg_id = m_hc_missed.at(0);
    m_hc_missed.removeAt(0);

    QByteArray tba;
    tba.append((const char*) &req, sizeof(request_missing_pkg_t));
    qDebug()<<"HC -> Requesting missed : "<<req.pkg_id;
    this->bleUartSendCmdWithResp(tba);
}


void DeviceController::onReplyMissingPackageArrived(QByteArray value)
{
    qDebug()<<"Missed Package arrived";
    cmd_resp_struct.cmd_timer.stop();

    huge_chunk_indexed_byterray_t tmp;
    tmp.barr.append( value.right(value.size() - 1) );
    tmp.received = 1;
    m_hc_vec.replace( hc_helper_struct.missed_in_request , tmp );

    m_hc_missed.removeAt( hc_helper_struct.missed_in_request); //added
    hc_helper_struct.missed_to_request--; // added

    if (m_hc_missed.size())
    {
        sendRequestMissingPackage();
    }
    else
    {
        this->sendAckHugeChunk();
        writeReceivedChunkToFile( hc_transfer_struct.write_pointer, hc_transfer_struct.incoming_type );
        hugeChunkDownloadFinished();
        // reset variables
    }
}


void DeviceController::onStartHugeChunkArrived()
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

    emit invokeAddToLogFile(m_ident_str, QString("Type"), QString::number(hc_transfer_struct.incoming_type));
    emit invokeAddToLogFile(m_ident_str, QString("ByteCountToReceive"), QString::number(hc_transfer_struct.incoming_byte_count));
    emit invokeAddToLogFile(m_ident_str, QString("WritePointer"), QString::number(hc_transfer_struct.write_pointer));

    bleUartSendCmdOk();

}

quint8 hc_chopchop_mode;

void DeviceController::onStartHugeChunkAckProcArrived(QByteArray value)
{
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());

    qDebug()<<"HUGE_CHUNK_ACK_PROC : started!";
    qDebug()<<"vecsize!"<<m_hc_vec.size();
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
        hugeChunkDownloadFinished();
    }
    else
    {
        qDebug()<<"Requesting missed";
        sendRequestMissingPackage();
    }
}

void DeviceController::onNoChunkAvailableArrived()
{
    cmd_resp_struct.cmd_timer.stop(); //todo dont forget to add this on all replies!
    bleUartSendCmdOk();
    emit allDataDownloaded(true, m_ident_idx);
}

void DeviceController::startDownloadAllDataProcedure()
{
    hc_chopchop_mode = true;
    sendRequestSensorData();
}

void DeviceController::sendAckHugeChunk()
{
    // add timeout timer..
    QByteArray tba;
    tba.resize(2);
    tba[0] = CMD_HC_OK;
    qInfo()<<"ackHugeChunk()";
    this->bleUartSendCmdWithResp(tba);
}

void DeviceController::hugeChunkDownloadFinished()
{
    qDebug()<<"All Received";

    emit invokeAddToLogFile("TODOSTRING", QString("AllArrived"), QString("TRUE"));

    sendAckHugeChunk();
    writeReceivedChunkToFile( hc_transfer_struct.write_pointer, hc_transfer_struct.incoming_type);

    if (hc_chopchop_mode)
        m_nextRequestTimer.singleShot(50, this, &DeviceController::onNextRequestTimerExpired );
}

void DeviceController::onNextRequestTimerExpired()
{
    sendRequestSensorData();
}

void DeviceController::onConnParamInfoArrived()
{

    qDebug()<<"PARA_INFO_MSG:"<<m_ident_idx<<m_ident_str<<"from dev:"<<
              "interval:"<<m_dev_conn_param_info.interval<<"latency:"<<m_dev_conn_param_info.latency<<
              "current mode:"<<m_dev_conn_param_info.current_mode<<"requested mode:"<<m_dev_conn_param_info.requested_mode<<"callcount"<<m_dev_conn_param_info.reserved;

    if (isDeviceInRequestedConnState() && m_connParamTimer.isActive())
    {
        m_connParamTimer.stop();
        emit requestedConnModeReached(true, m_dev_conn_param_info.current_mode);
    }
}


void DeviceController::onConnParamTimerExpired()
{
    if ( retries_remaining )
    {
        retries_remaining--;
        setConnParamsOnCentral(m_dev_requested_conn_mode);
        m_connParamTimer.singleShot(500, this, SLOT(onConnParamTimerExpired()));
    }
    else
    {
        emit requestedConnModeReached(false, m_dev_conn_param_info.current_mode);
        qDebug()<<"connparamupdate failed after x retries..";
        //show the devil or reply back to catch controller that the action failed
    }
}


