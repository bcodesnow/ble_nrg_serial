#include "devicehandler.h"



void DeviceHandler::ble_uart_tx(const QByteArray &value)
{
    if (value.size())
        m_service->writeCharacteristic(m_writeCharacteristic, value, QLowEnergyService::WriteWithResponse); /*  m_writeMode */
}


void DeviceHandler::ble_uart_rx(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    // emit signal internally if low priority stuff happens, it will get a loop in the qt state machine and gets handled a bit later. use resources only long if its really necessary.

#define CMD_STATE    0x00
#define HUGE_CHUNK_STATE 0x01
#define REQUESTING_MISSED_STATE 0x02
#define CMD_PARAM_CONF_STATE    0x04

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
            {
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
            }
                break;

            case DATA_COLLECTED:
            {
                //confirm if it was a catch or drop -> we should give it to a class above iE devFinder
                setInfo("Waiting for Confirmation!");
            }
                break;

            case ALIVE:
            {
                // modify to set data and emit only one signal..

                // show current state and file index.. here we should start a timer, and if no more msg arrives to alive for 3secs we know we lsot the sensor.
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
            }
                break;

            case HUGE_CHUNK_START:
            {
                qDebug()<<"HC -> Starts, arrived msg size: <<"<<value.size();
                state = HUGE_CHUNK_STATE;
                incoming_byte_count =  data[1] << 8;
                incoming_byte_count |=  data[2];
                incoming_type = data[3];

                // uint16_t tmp_write_pointer; // moved definition
                tmp_write_pointer =  data[4] << 8;
                tmp_write_pointer |=  data[5];


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
                m_refToFileHandler->add_to_log_fil(m_ident_str, QString("Type"), QString::number(incoming_type));
                m_refToFileHandler->add_to_log_fil(m_ident_str, QString("ByteCountToReceive"), QString::number(incoming_byte_count));
                m_refToFileHandler->add_to_log_fil(m_ident_str, QString("WritePointer"), QString::number(tmp_write_pointer));

                setInfo("Incoming!");
            }
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

            case DIAG_INFO:
                if ( data[1] == DIAG_1_TYPE_HC_STAT )
                {
                }
                else if (data[1] == DIAG_1_TYPE_CONN_PARAM)
                {
                    qDebug()<<"CONN PARAM CHANGE CHECKED BY THE DEVICE!";
                    uint16_t interv =  data[2] << 8;
                    interv |=  data[3];
                    uint16_t superv =  data[4] << 8;
                    superv |=  data[5];
                    qDebug()<<"Interv:"<<interv<<"Superv"<<superv;
                }
                else if (data[1] == DIAG_1_TYPE_LENGTH_TEST)
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
                m_dev_curr_param_info = *cp_ptr;

                qDebug()<<"PARA_INFO_MSG:"<<m_ident_idx<<m_ident_str<<"from dev"<<cp_ptr->latency<<cp_ptr->interval<<cp_ptr->current_mode<<cp_ptr->requested_mode;
//                if (m_dev_curr_param_info.requested_mode == m_dev_requested_conn_mode)
//                {
//                    qDebug()<<"Dev Received our Mode Request..";
//                    //setConnParamsOnCentral(m_dev_requested_conn_mode);
//                }
                if (m_dev_curr_param_info.current_mode == m_dev_requested_conn_mode && m_dev_curr_param_info.requested_mode == m_dev_requested_conn_mode)
                {
                    //make the necessary shutup!!
                    qDebug()<<"Dev also made the necessary changes and switched!";
                    setShutUp(1);

                    //continue with requesting sensor data..
                    if (m_dev_requested_conn_mode == FAST )
                    {
                        QByteArray tba;
                        tba.resize(2);
                        tba[0] = REQUEST_SENSORDATA;
                        tba[1] = 0xFF;

                        if (tba.size() && m_writeCharacteristic.isValid())
                            m_service->writeCharacteristic(m_writeCharacteristic, tba, QLowEnergyService::WriteWithResponse); /*  m_writeMode */
                    }

                }


                break;
                //
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
