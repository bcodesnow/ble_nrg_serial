#include "devicecontroller.h"
#include "mci_catch_detection.h"

// TODO: this is critical to add ts() message again -> this starts time sync.. we should also set the connections prior to syncing
//void DeviceController::sendCMDStringFromTerminal(const QString &str)
//{
    //    if (!connectionAlive())
    //    {
    //        qCritical()<<"Shit is fkd up";
    //        return;
    //    }

    //    QByteArray tba;
    //    if ( str == QString("get_state()"))
    //    {
    //        tba.resize(1);
    //        tba[0] = GET_STATE;
    //        qInfo()<<"Fetching State";
    //    }
    //    else if ( str == QString("start()"))
    //    {
    //        tba.resize(1);
    //        tba[0] = START;
    //        qInfo()<<"Sending Start";
    //    }
    //    else if ( str == QString("stop()"))
    //    {
    //        tba.resize(1);
    //        tba[0] = STOP;
    //        qInfo()<<"Sending Stop";
    //    }

    ////    else if (str == "confirm(1)" )
    ////    {
    ////        tba.resize(2);
    ////        tba[0] = WRITE_CATCH_SUCCESS;
    ////        tba[1] = 1;
    ////        qInfo()<<"Sending Catch Configrm";
    ////    }

    ////    else if (str == "confirm(0)" )
    ////    {
    ////        tba.resize(0);
    ////        tba[0] = WRITE_CATCH_SUCCESS;
    ////        tba[1] = 0;
    ////        qInfo()<<"Sending Catch Confrim";
    ////    }
    //    else if (str == "ts()" )
    //    {
    //        m_refToTimeStampler->start_time_sync(m_ident_idx);
    //        qInfo()<<"TimeSync in Test..";
    //    }
    //    else
    //    {
    //        qCritical()<<"Unknown Command!";
    //    }

    //    if (tba.size())
    //        m_service->writeCharacteristic(m_writeCharacteristic, tba, QLowEnergyService::WriteWithResponse); /*  m_writeMode */
//}

QString stateToString(uint16_t tmp)
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


void printProperties(QLowEnergyCharacteristic::PropertyTypes props)
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

void  DeviceController::printThroughput()
{
    float kbyte_ps;
    float kbit_ps;
    float secs;
    quint64 elapsed;

    elapsed = m_debugTimer.elapsed();
    kbyte_ps = 0;
    kbit_ps = 0;
    secs = (float) elapsed / 1000.0f;
    kbyte_ps = hc_transfer_struct.incoming_byte_count /  secs / 1000  ;
    kbit_ps = kbyte_ps * 8.0;
    qInfo()<<"HC -> Transfer of"<<hc_transfer_struct.incoming_byte_count<<"bytes took"<< elapsed<<"ms";
    qInfo()<<"HC -> Throughput of net data is" <<kbyte_ps<< "kbyte / s : "<<"or "<<kbit_ps<<"kbit/s";
    secs = (float) elapsed / 1000.0f;
    kbyte_ps = ( hc_transfer_struct.incoming_byte_count + hc_transfer_struct.incoming_package_count ) /  secs / 1000  ;
    kbit_ps = kbyte_ps * 8.0;
    qInfo()<<"HC -> Throughput of raw data is" <<kbyte_ps<< "kbyte / s : "<<"or "<<kbit_ps<<"kbit/s";
}
