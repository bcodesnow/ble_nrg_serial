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

}


void LogFileHandler::write_type_to_file(QByteArray data, uint8_t type)
{
    static quint64 counter;
    QString homeLocation = QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory);
    qDebug()<<"File Path: "<<homeLocation;
    QString idx_str = tr("%1").arg(m_curr_idx);
    homeLocation.append( idx_str );

    switch (type)
    {
    case TYPE_AUD:
        homeLocation.append( QString("AUDIO") );
        break;
    case TYPE_GYR:
        homeLocation.append( QString("GYR") );
        break;
    case TYPE_ACC:
        homeLocation.append( QString("ACC") );
        break;
    case TYPE_PRS:
        homeLocation.append( QString("PRS") );
        break;
    case TYPE_MAG:
        homeLocation.append( QString("GYR") );
        break;
    case TYPE_LOG:
        homeLocation.append( QString("LOG") );
        break;

    default:
        homeLocation.append( QString("SOMEFILE") );
        break; // technically not needed
    }

    QFile file(homeLocation);
    qDebug()<<file.open(QIODevice::WriteOnly);
    qDebug()<<file.write(data);
    file.close();
    qDebug()<<"FILE WRITTEN";

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

void LogFileHandler::start_new_log_fil()
{

}

#define SEP_CHAR " : "
#define LINE_END ";\n"
void LogFileHandler::add_to_log_fil(QString ident, QString key, QString val)
{
    QString tmpString = ident + SEP_CHAR + key + SEP_CHAR + val + LINE_END;
    m_log_fil_buf.append(tmpString);
}

void LogFileHandler::fin_log_fil()
{

}
