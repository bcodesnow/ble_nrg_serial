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
#include "qmllistadapter.h"



int main(int argc, char *argv[])
{
    //QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    qmlRegisterType<GraphPainter>("GraphPainterCpp",1,0,"GraphPainterCpp");

    QmlListAdapter* ladapter = new QmlListAdapter();
    TerminalToQmlB term;
    term.setActive (false); // true

    LogFileHandler* log_file_handler = new LogFileHandler();
    log_file_handler->set_aut_incr(false);

//    QList<DeviceInterface*> device_interfaces;

    TimeSyncHandler* time_sync_handler = new TimeSyncHandler(ladapter->getList());
    time_sync_handler->start_time_stamp();

    CatchController* catch_controller = new CatchController(ladapter->getList(), time_sync_handler,log_file_handler);

    //log_file_handler.set_fil_src_cnt(2);
    //log_file_handler.set_last_type(TYPE_PRS);

    ConnectionHandler connection_handler; // keeps track of local ble interface

//    QBluetoothAddress leftAdapter, rightAdapter; // Local addresses for connection
//    // search for bt adapters and select an address for both devices
//    connection_handler.initBtAdapters(leftAdapter, rightAdapter);

    DeviceFinder device_finder(ladapter->getList(), &connection_handler, time_sync_handler,catch_controller,log_file_handler);
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

//    qmlRegisterUncreatableType<DeviceInfo>("com.dev", 1, 0, "DeviceInfo",  QStringLiteral("We do handle object creation only on c++ side in this project") );
//    qmlRegisterUncreatableType<DeviceInterface>("com.dev", 1, 0, "DeviceInterface",  QStringLiteral("We do handle object creation only on c++ side in this project") );

    //engine.rootContext()->setContextProperty("terminalToQml", &term);
    engine.rootContext()->setContextProperty("connectionHandler", &connection_handler);
    engine.rootContext()->setContextProperty("deviceFinder", &device_finder);
    engine.rootContext()->setContextProperty("catchController", catch_controller);
    engine.rootContext()->setContextProperty("ladApter", ladapter);
    //engine.rootContext()->setContextProperty("deviceInterfaces", QVariant::fromValue(device_interfaces));

    engine.rootContext()->setContextProperty("fileHandler", log_file_handler);

    //engine.rootContext()->setContextProperty("networkManager", &ntwMngr);

    QQuickStyle::setStyle("Material");

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    return app.exec();
}
