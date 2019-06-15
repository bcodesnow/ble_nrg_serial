#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "devicefinder.h"
#include "devicehandler.h"
#include <QQmlContext>
#include <QtMessageHandler>
#include <QtGlobal>
#include <QScreen>
#include "connectionhandler.h"
#include "terminaltoqmlb.h"
#include <QTimer>
#include "logfilehandler.h"
#include <QQuickStyle>
#include "ble_uart.h"
#include "timestampler.h"

int main(int argc, char *argv[])
{

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    TerminalToQmlB term;
    term.setActive (true);

    TimeStampler ts;
    ts.start_time_stamp();

    LogFileHandler log_file_handler;
    log_file_handler.set_aut_incr(false);
    //log_file_handler.set_fil_src_cnt(2);
    //log_file_handler.set_last_type(TYPE_PRS);

    ConnectionHandler connection_handler; //keeps track of local ble interface

    DeviceHandler device_handler[2];
    device_handler[0].setRefToFileHandler(&log_file_handler);
    device_handler[0].setIdentifier("LEFT", 0);
    device_handler[0].setRefToTimeStampler(&ts);

    DeviceFinder device_finder(&device_handler[0]);
    device_handler[1].setRefToFileHandler(&log_file_handler);
    device_handler[1].setIdentifier("RIGHT", 1);
    device_handler[1].setRefToTimeStampler(&ts);

    ts.setRefToDevHandlerArr(device_handler);

    QQmlApplicationEngine engine;
    qmlRegisterUncreatableType<DeviceHandler>("Shared", 1, 0, "AddressType", "Enum is not a type");
    engine.rootContext()->setContextProperty("terminalToQml", &term);
    engine.rootContext()->setContextProperty("connectionHandler", &connection_handler);
    engine.rootContext()->setContextProperty("deviceFinder", &device_finder);
    engine.rootContext()->setContextProperty("deviceHandler_0", &device_handler[0]);
    engine.rootContext()->setContextProperty("deviceHandler_1", &device_handler[1]);
    engine.rootContext()->setContextProperty("fileHandler", &log_file_handler);

//    QByteArray tba;
//    uint32_t tstamp = 1252151251;
//    tba.resize(7);
//    tba[0] = TS_MSG;
//    tba[1] = TS_CMD_TIMESTAMP_IN_PAYLOAD;
//    tba[2] = ( tstamp >> 24 ) & 0xFF ;
//    tba[3] = ( tstamp >> 16 ) & 0xFF ;
//    tba[4] = ( tstamp >>  8 ) & 0xFF ;
//    tba[5] =   tstamp & 0xFF ;

//    uint32_t rec_ts = 0;
//    const quint8 *data = reinterpret_cast<const quint8 *>(tba.constData());
//    rec_ts = ( (uint32_t) data[2] ) << 24;
//    rec_ts |=( (uint32_t) data[3] )<< 16;
//    rec_ts |=( (uint32_t) data[4] )<< 8;
//    rec_ts |= ( (uint32_t) data[5] );
//    qDebug()<<"DIISE"<<rec_ts;
//qDebug()<<"get_diff_in_us_to_current_ts"<<ts.get_diff_in_us_to_current_ts(5);
//qDebug()<<"get_diff_in_us_to_current_ts"<<ts.get_diff_in_us_to_current_ts(5);
//qDebug()<<"get_diff_in_us_to_current_ts"<<ts.get_diff_in_us_to_current_ts(5);
//qDebug()<<"get_diff_in_us_to_current_ts"<<ts.get_diff_in_us_to_current_ts(5);
//qDebug()<<"get_diff_in_us_to_current_ts"<<ts.get_diff_in_us_to_current_ts(5);
//qDebug()<<"get_diff_in_us_to_current_ts"<<ts.get_diff_in_us_to_current_ts(5);
//qDebug()<<"get_diff_in_us_to_current_ts"<<ts.get_diff_in_us_to_current_ts(5);
//qDebug()<<"get_diff_in_us_to_current_ts"<<ts.get_diff_in_us_to_current_ts(5);


    QQuickStyle::setStyle("Material");

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    return app.exec();
}
