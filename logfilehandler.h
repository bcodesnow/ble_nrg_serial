#ifndef LOGFILEHANDLER_H
#define LOGFILEHANDLER_H

#include <QObject>

class LogFileHandler : public QObject
{
    Q_OBJECT

    Q_PROPERTY(quint64 idx MEMBER m_curr_idx NOTIFY idxChanged)
    Q_PROPERTY(QString lastPath MEMBER m_last_path NOTIFY lastPathChanged)

private:
    quint64 m_curr_idx;
    QString m_last_path;
    quint32 m_fil_src_cnt; // file source count
    quint8  m_last_type; // and last type make out when it increments.
    QString m_log_fil_buf;
    QString m_homeLocation;
    bool m_is_aut_incr_en;

public:
    explicit LogFileHandler(QObject *parent = nullptr);
    void write_type_to_file(QByteArray data, uint8_t type);

signals:
    void lastPathChanged(void);
    void idxChanged(quint64 tidx);
    void confirmNeeded();

public:
    void start_new_log_fil();
    void add_to_log_fil(QString ident, QString key, QString val);
    void fin_log_fil();

    Q_INVOKABLE void confirm (QString ident, bool bcatch)
    {
        add_to_log_fil(ident, QString("SUCCESS"), QString(( bcatch ? "CATCH" : "DROP" )) );
    }

    Q_INVOKABLE void rst_idx()
    {
        m_curr_idx = 0;
    }
    quint64 get_idx()
    {
        return m_curr_idx;
    }
    void set_aut_incr(bool onoff)
    {
        m_is_aut_incr_en = onoff;
    }
    void incr_idx()
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
};

#endif // LOGFILEHANDLER_H


