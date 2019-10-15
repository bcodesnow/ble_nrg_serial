#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTimer>
#include <QQuickStyle>
#include <QQmlContext>
#include <QQmlContext>
#include <QtMessageHandler>
#include <QtGlobal>
#include <QScreen>
#include "connectionhandler.h"
#include "terminaltoqmlb.h"
#include "devicefinder.h"
#include "devicecontroller.h"
#include "logfilehandler.h"
#include "ble_uart.h"
#include "timesynchandler.h"
#include "graphpainter.h"
#include "deviceinterface.h"


int main(int argc, char *argv[])
{
    //QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    qmlRegisterType<GraphPainter>("GraphPainterCpp",1,0,"GraphPainterCpp");

    TerminalToQmlB term;
    term.setActive (false); // true

    LogFileHandler* log_file_handler = new LogFileHandler();
    log_file_handler->set_aut_incr(false);

    QList<DeviceInterface*> device_interfaces;

    TimeSyncHandler* time_sync_handler = new TimeSyncHandler(&device_interfaces);
    time_sync_handler->start_time_stamp();

    CatchController* catch_controller = new CatchController(&device_interfaces, time_sync_handler,log_file_handler);

    //log_file_handler.set_fil_src_cnt(2);
    //log_file_handler.set_last_type(TYPE_PRS);

    ConnectionHandler connection_handler; // keeps track of local ble interface

//    QBluetoothAddress leftAdapter, rightAdapter; // Local addresses for connection
//    // search for bt adapters and select an address for both devices
//    connection_handler.initBtAdapters(leftAdapter, rightAdapter);

    DeviceFinder device_finder(&device_interfaces, &connection_handler, time_sync_handler,catch_controller,log_file_handler);
    qDebug()<<"MainThread"<<QThread::currentThreadId();

    //device_finder = new DeviceFinder
    //    devices.at(0).init_device(leftAdapter, );

    //DeviceHandler device_handler[2];
    //    device_handler[0].setRefToFileHandler(&log_file_handler);
    //    device_handler[0].setIdentifier("LEFT", 0, leftAdapter);
    //    device_handler[0].setRefToTimeStampler(&ts);
    //    DeviceFinder device_finder(&device_handler[0]);
    //    device_handler[1].setRefToFileHandler(&log_file_handler);
    //    device_handler[1].setIdentifier("RIGHT", 1, rightAdapter);
    //    device_handler[1].setRefToTimeStampler(&ts);

//    ts.setRefToDevHandlerArr(device_handler);

    //NetworkManager ntwMngr(&log_file_handler);

    QQmlApplicationEngine engine;
    qRegisterMetaType<DeviceInfo::DeviceType>("DeviceType");
    qRegisterMetaType<DeviceController::AddressType>("AddressType");
    engine.rootContext()->setContextProperty("terminalToQml", &term);
    engine.rootContext()->setContextProperty("connectionHandler", &connection_handler);
    engine.rootContext()->setContextProperty("deviceFinder", &device_finder);
    //engine.rootContext()->setContextProperty("deviceHandler_0", &device_handler[0]);
    //engine.rootContext()->setContextProperty("deviceHandler_1", &device_handler[1]);
    engine.rootContext()->setContextProperty("fileHandler", log_file_handler);
    //engine.rootContext()->setContextProperty("networkManager", &ntwMngr);





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

    //    QElapsedTimer testtim;
    //    testtim.start();
    //    uint32_t someval;
    //    quint32 someval_q;
    //    for (int i=0; i < 123; i++)
    //    {
    //        someval = testtim.nsecsElapsed() / 1000;
    //        someval_q = testtim.nsecsElapsed() / 1000;
    //        qDebug()<<"val"<<someval<<"qval"<<someval_q;
    //    }

    QQuickStyle::setStyle("Material");

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    return app.exec();
}
