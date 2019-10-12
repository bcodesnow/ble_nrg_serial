#ifndef DEVICEHANDLER_TYPES_H
#define DEVICEHANDLER_TYPES_H

#include <QByteArray>
#include <QTimer>

struct huge_chunk_indexed_byterray_t
{
    quint16 received;
    QByteArray barr;
};

struct huge_chunk_helper_t
{
    uint16_t hc_highest_index;
    bool first_multi_chunk;
    uint16_t last_idx;
    uint16_t missed_to_request;
    uint16_t missed_in_request;
};

struct cmd_resp_struct_t
{
    QTimer cmd_timer;
    QByteArray last_cmd;
    quint8 retry;
    quint16 timeout;
};

struct device_helper_struct_t
{
    int idx;
    QString ident_str; // Q_PROPERTY
    QString address; // Q_PROPERTY
    QString mainState; // Q_PROPERTY
};

#endif // DEVICEHANDLER_TYPES_H
