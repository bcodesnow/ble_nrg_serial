#ifndef LOGFILEHANDLER_H
#define LOGFILEHANDLER_H

#include <QObject>
#include <QDataStream>
#include <QVariant>
#include <QDebug>

#include <graphpainter.h>
#include <paintdata.h>

class LogFileHandler : public QObject
{
    Q_OBJECT

    Q_PROPERTY(quint64 idx MEMBER m_curr_idx NOTIFY idxChanged)
    Q_PROPERTY(QString lastPath MEMBER m_last_path NOTIFY lastPathChanged)
    Q_PROPERTY(QVariant paintDataList READ getPaintDataList NOTIFY paintDataListChanged)

private:
    quint64 m_curr_idx;
    QString m_last_path;
    quint32 m_fil_src_cnt; // file source count
    quint8  m_last_type; // and last type make out when it increments.
    QString* m_log_fil_buf;
    QString m_homeLocation;
    bool m_is_aut_incr_en;

    QList<QObject*> m_paintDataList;


signals:
    void lastPathChanged(void);
    void idxChanged(quint64 tidx);
    void confirmNeeded();
    void paintDataListChanged();
    void qmltestsignal(int idx);
    void newPaintData(QObject *dataptr, QString dataname);
    // void newPaintData(QObject *dataptr, int type, char side);

public:
    explicit LogFileHandler(QObject *parent = nullptr);

    Q_INVOKABLE void demo(QString ident, int type);

    void sortArray(QByteArray *arr, uint16_t wp);
    QVector<QVariant> bytesToInt16(QByteArray arr);
    QVector<QVariant> bytesToUint16(QByteArray arr);
    QVector<QVariant> bytesToFloat32(QByteArray arr);

    int typeToIndex(uint8_t type);

    void saveToCsv(QString location, QVector<QVariant> data);

    QByteArray loadFromFile(QString ident, uint8_t type);

    void write_type_to_file(QString ident, QByteArray data, uint8_t type, uint16_t wp);

    void start_new_log_fil();

    void fin_log_fil(QString ident);

    QString getHomeLocation();


    QVariant getPaintDataList()
    {
        return QVariant::fromValue(m_paintDataList);
    }

    Q_INVOKABLE void sendCatchSuccessFromQML(bool wasItCatched);
    Q_INVOKABLE void confirm (QString ident, bool bcatch)
    {
        add_to_log_fil_slot(ident, QString("SUCCESS"), QString(( bcatch ? "CATCH" : "DROP" )) );
    }

    Q_INVOKABLE void rst_idx()
    {
        m_curr_idx = 0;
        emit idxChanged(m_curr_idx);
    }
    quint64 get_idx()
    {
        return m_curr_idx;
    }
    void set_aut_incr(bool onoff)
    {
        m_is_aut_incr_en = onoff;
    }
    Q_INVOKABLE void incr_idx()
    {
        m_curr_idx++;
        emit idxChanged(m_curr_idx);
    }
    void set_last_type (uint8_t type)
    {
        m_last_type = type;
    }
    void set_fil_src_cnt (quint16 cunt)
    {
        m_fil_src_cnt = cunt;
    }

public slots:
    void add_to_log_fil_slot(QString ident, QString key, QString val);

};

#endif // LOGFILEHANDLER_H


