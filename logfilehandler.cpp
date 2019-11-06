#include "logfilehandler.h"
#include <QStandardPaths>
#include <QFile>
#include <QtDebug>
#include "ble_uart.h"
#include <QString>

LogFileHandler::LogFileHandler(QObject *parent) : QObject(parent),
    m_curr_idx(0), m_fil_src_cnt(0), m_is_aut_incr_en(0), m_last_type(0),
    m_last_path("NONE")
{
    // testsignal
    // connect(this, SIGNAL(qmltestsignal(int)), this, SLOT(signaltestslot(int)));
    m_log_fil_buf = new QString();
    m_homeLocation = QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory);
    m_paintDataList.reserve(TYPE_COUNT*2);
    //    QVector<QObject*> reserver = QVector<QObject*>(TYPE_COUNT*2);
    //   // QVector<QVariant> nullvec = {0};
    //    m_paintDataList = reserver.toList();
    //    for (int i=0;i<m_paintDataList.size();i++) {
    //        m_paintDataList.replace(i, new PaintData(this));
    //    }
}

void LogFileHandler::sortArray(QByteArray *arr, uint16_t wp)
{
    QByteArray tmpArray = arr->left(wp);
    arr->remove(0,wp);
    arr->append(tmpArray);
}

QVector<QVariant> LogFileHandler::bytesToInt16(QByteArray arr)
{
    int16_t tmp_int;
    QVector<QVariant> intvec;//(QVector<QVariant>(arr.size()/2));
    intvec.reserve(arr.size()/2);

    for (int i=0; i<arr.size(); i+=2)
    {
        tmp_int = static_cast<int16_t>(static_cast<int16_t>(arr[i+1]) << 8);
        tmp_int |= static_cast<int16_t>( arr[i] ) & 0xFF;
        intvec.append(QVariant(tmp_int));
    }
    return intvec;
}

QVector<QVariant> LogFileHandler::bytesToUint16(QByteArray arr)
{
    uint16_t tmp_uint;
    QVector<QVariant> uintvec(QVector<QVariant>(arr.size()/2));
    uintvec.clear();
    for (int i=0; i<arr.size(); i+=2)
    {
        tmp_uint = static_cast<uint16_t>(static_cast<uint16_t>(arr[i+1]) << 8);
        tmp_uint |= static_cast<uint16_t>( arr[i] ) & 0xFF;
        uintvec.append(QVariant(tmp_uint));
    }
    return uintvec;
}

QVector<QVariant> LogFileHandler::bytesToFloat32(QByteArray arr)
{
    float tmp_float;
    QByteArray tmp_arr;
    QVector<QVariant> floatvec;//(QVector<QVariant>(arr.size()/4));
    floatvec.reserve(arr.size()/4);

    for (int i=0;i<arr.size();i=i+4)
    {
        tmp_arr = arr.mid(i,4);
        QDataStream stream(tmp_arr);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision); // for float (4-bytes / 32-bits)
        stream.setByteOrder(QDataStream::LittleEndian);
        stream >> tmp_float;
        floatvec.append(QVariant(tmp_float));
    }
    return floatvec;
}

int LogFileHandler::typeToIndex(uint8_t type)
{
    switch (type)
    {
    case TYPE_AUD:
        return 0;
    case TYPE_GYR:
        return 1;
    case TYPE_ACC:
        return 2;
    case TYPE_PRS:
        return 3;
    case TYPE_MAG:
        return 4;
    case TYPE_LOG:
        return -1;
    default:
        return -2;
    }

}

//void LogFileHandler::saveToCsv(QString location, QVector<QVariant> data)
//{
//    qDebug()<<"save to csv:"<<location;
//    location.append(".csv");
//    QFile file(location);

//    if (file.open( QIODevice::WriteOnly | QIODevice::Append ))
//    {
//        QTextStream output(&file);
//        //output << "header" << '\n'; // optional: header line
//        for (int i=0;i<data.size();i++) {
//            // Fill a line (converting both int and float to qreal)
//            output << data.at(i).toReal() << '\n';
//        }
//        file.close();
//        qDebug()<<"FILE WRITTEN";
//    }
//    else qDebug()<<"Error: could not create .csv file for"<<location;
//}

QByteArray LogFileHandler::loadFromFile(QString ident, uint8_t type)
{
    QString location = m_homeLocation+"Catch_Data/catch_balint/0_"+ident+"_";
    switch (type)
    {
    case TYPE_AUD:
        location.append("AUDIO");
        break;
    case TYPE_GYR:
        location.append("GYR");
        break;
    case TYPE_ACC:
        location.append("ACC");
        break;
    case TYPE_PRS:
        location.append("PRS");
        break;
    case TYPE_MAG:
        location.append("MAG");
        break;
    case TYPE_LOG:
        break;
    default:
        break;
    }
    QFile loader(location);
    QByteArray data;
    if (loader.open( QIODevice::ReadOnly ))
    {
        qDebug()<<"Reading file:"<<loader.fileName();
        loader.waitForReadyRead(-1);
        data =  loader.readAll();
        loader.close();
    }
    else qDebug()<<"Failed reading file:"<<loader.fileName();
    return data;
}

void LogFileHandler::demo(QString ident, int type)
{
    qDebug()<<"DEMO";
    QVector<QVariant> dataVec(QVector<QVariant>(0));
    QByteArray data = loadFromFile(ident, type);
    QString tmpLocation = m_homeLocation;

    switch (type)
    {
    case TYPE_AUD:
        dataVec = bytesToInt16(data);
        break;
    case TYPE_GYR:
        dataVec = bytesToInt16(data);
        break;
    case TYPE_ACC:
        dataVec = bytesToInt16(data);
        break;
    case TYPE_PRS:
        dataVec = bytesToFloat32(data);
        break;
    case TYPE_MAG:
        dataVec = bytesToInt16(data);
        break;
    case TYPE_LOG:
        break;
    default:
        break;
    }

    // remove previous data
    for (int i=0;i<m_paintDataList.size();i++) {
        if (((PaintData*)m_paintDataList.at(i))->getType() == type &&
                ((PaintData*)m_paintDataList.at(i))->getSide() == ident.at(0).toLatin1())
        {
            m_paintDataList.removeAt(i);
        }
    }

    // add data
    m_paintDataList.append(new PaintData(type, ident.at(0).toLatin1(), dataVec));
    emit paintDataListChanged();
    for (int i=0;i<m_paintDataList.size();i++) {
        emit newPaintData(m_paintDataList.at(i),((PaintData*)m_paintDataList.at(i))->getName());
    }
    qDebug()<<"DEMO END";
}

void LogFileHandler::write_type_to_file_slot(QString ident, QByteArray* data, quint8 type, quint16 wp)
{
    qDebug()<<"write_type_to_file_slot"<<data->size();

    QVector<QVariant> dataVec(QVector<QVariant>(0));
    static quint64 counter;

    // get target location
    QString tmpLocation = m_homeLocation+"/"+m_curr_dir+"/";
    qDebug()<<"File Path: "<<tmpLocation;
    QString idx_str = tr("%1_%2_").arg(m_curr_idx).arg(ident);
    tmpLocation.append( idx_str );

    // sort by writepointer
    if (type != TYPE_LOG && !data->isEmpty())
        sortArray(data, wp);

    // Byte conversion
    switch (type)
    {
    case TYPE_AUD:
        dataVec = bytesToInt16(*data);
        tmpLocation.append( QString("AUDIO") );
        break;
    case TYPE_GYR:
        dataVec = bytesToInt16(*data);
        tmpLocation.append( QString("GYR") );
        break;
    case TYPE_ACC:
        dataVec = bytesToInt16(*data);
        tmpLocation.append( QString("ACC") );
        break;
    case TYPE_PRS:
        dataVec = bytesToFloat32(*data);
        tmpLocation.append( QString("PRS") );
        break;
    case TYPE_MAG:
        dataVec = bytesToInt16(*data);
        tmpLocation.append( QString("MAG") );
        break;
    case TYPE_LOG:
        tmpLocation.append( QString("LOG") );
        break;
    default:
        tmpLocation.append( QString("SOMEFILE") );
    }

    if (type != TYPE_LOG && !dataVec.isEmpty())
    {
        // remove previous paint data
        for (int i=0;i<m_paintDataList.size();i++) {
            if (((PaintData*)m_paintDataList.at(i))->getType() == type &&
                    ((PaintData*)m_paintDataList.at(i))->getSide() == ident.at(0).toLatin1())
            {
                m_paintDataList.removeAt(i);
            }
        }
        // add data to painters
        m_paintDataList.append(new PaintData(type, ident.at(0).toLatin1(), dataVec));
        emit paintDataListChanged();
        for (int i=0;i<m_paintDataList.size();i++) {
            emit newPaintData(m_paintDataList.at(i),((PaintData*)m_paintDataList.at(i))->getName());
        }
        // save as csv
        // saveToCsv(tmpLocation, dataVec);
    }
    //    else {
    // save as text
    QFile file(tmpLocation);
    qDebug()<<file.open(QIODevice::WriteOnly);
    qDebug()<<file.write(*data);
    file.close();
    //    }

    // increment file index
    if ( m_is_aut_incr_en )
    {
        if (type == m_last_type)
        {
            counter++;
            if ( counter % m_fil_src_cnt == 0)
            {
                m_curr_idx++;
                emit idxChanged(m_curr_idx);
            }
        }
    }
}

void LogFileHandler::add_to_log_fil_slot(QString ident, QString key, QString val)
{
    QString tmpString = ident + SEP_CHAR + key + SEP_CHAR + val + LINE_END;
    m_log_fil_buf->append(tmpString);
}

void LogFileHandler::sendCatchSuccessFromQML(bool wasItCatched)
{
    add_to_log_fil_slot("Info","Username",m_curr_user);
    add_to_log_fil_slot("Info","CatchMode",m_curr_catch_mode);
    qDebug()<<"USE here the lastCatchSuccess from the Catch controller, make a qproperty daraus";

    //add_to_log_fil_slot("Info","SUCCESS", wasItCatched ? "CATCH" : "DROP");


    // fin_log_fil(QString("Info"));
}

void LogFileHandler::rst_idx()
{
    m_curr_idx = 0;
    emit idxChanged(m_curr_idx);
}

quint64 LogFileHandler::get_idx()
{
    return m_curr_idx;
}

void LogFileHandler::set_aut_incr(bool onoff)
{
    m_is_aut_incr_en = onoff;
}

void LogFileHandler::incr_idx()
{
    m_curr_idx++;
    emit idxChanged(m_curr_idx);
}

void LogFileHandler::set_last_type(uint8_t type)
{
    m_last_type = type;
}

void LogFileHandler::set_fil_src_cnt(quint16 cunt)
{
    m_fil_src_cnt = cunt;
}

void LogFileHandler::set_curr_dir(QString username)
{
    if (username.isEmpty())
        m_curr_user = "dev";
    else
        m_curr_user = username;

    m_curr_dir = "catch_data_WD_"+m_curr_user+"_"+QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    QDir dir;
    dir.setPath(m_homeLocation);
    dir.mkdir(m_curr_dir);
    qDebug()<<"Created folder"<<m_curr_dir<<"in"<<m_homeLocation;
}

void LogFileHandler::set_curr_catch_mode(QString mode)
{
    m_curr_catch_mode = mode;
}

//void LogFileHandler::confirm (QString ident, bool bcatch)
//{
//    add_to_log_fil_slot(ident, QString("SUCCESS"), QString(( bcatch ? "CATCH" : "DROP" )) );
//}

void LogFileHandler::fin_log_fil(QString ident)
{
    QByteArray ba; // get rid of this..
    ba =  m_log_fil_buf->toUtf8();
    write_type_to_file_slot(ident, &ba, TYPE_LOG, 0);
    m_log_fil_buf->clear();
}

QString LogFileHandler::getHomeLocation()
{
    return m_homeLocation;
}

QVariant LogFileHandler::getPaintDataList()
{
    return QVariant::fromValue(m_paintDataList);
}
