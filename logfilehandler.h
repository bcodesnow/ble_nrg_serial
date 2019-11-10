#ifndef LOGFILEHANDLER_H
#define LOGFILEHANDLER_H

#include <QObject>
#include <QDataStream>
#include <QVariant>
#include <QDateTime>
#include <QDir>
#include <QDebug>


#include <graphpainter.h>
#include <paintdata.h>

#define SEP_CHAR " : "
#define LINE_END ";\n"

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
    QString m_curr_user;
    QString m_curr_dir;
    QString m_curr_catch_mode;
    bool m_is_aut_incr_en;
    QList<QObject*> m_paintDataList;

signals:
    void lastPathChanged(void);
    void idxChanged(quint64 tidx);
    void confirmNeeded();

    void paintDataListChanged();
    void updateAllPainters(QList<QObject*> datalist);
    void invokeGoogleUpload(QString filename, QByteArray data);

public:
    explicit LogFileHandler(QObject *parent = nullptr);

    void sortArray(QByteArray *arr, uint16_t wp);
    QVector<QVariant> bytesToInt16(QByteArray arr);
    QVector<QVariant> bytesToUint16(QByteArray arr);
    QVector<QVariant> bytesToFloat32(QByteArray arr);


    QString getHomeLocation();
    QVariant getPaintDataList();

    Q_INVOKABLE void rst_idx();
    quint64 get_idx();
    void set_aut_incr(bool onoff);
    Q_INVOKABLE void incr_idx();
    void set_last_type (uint8_t type);
    void set_fil_src_cnt (quint16 cunt);
    Q_INVOKABLE void set_curr_dir (QString username);
    Q_INVOKABLE void set_curr_catch_mode (QString mode);


public slots:
    void fin_log_fil();
    void add_to_log_fil_slot(QString ident, QString key, QString val);
    void write_type_to_file_slot(QString ident, QByteArray* data, quint8 type, quint16 wp);

};

#endif // LOGFILEHANDLER_H


