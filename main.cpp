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
#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID))
#include "linuxterminalinterface.h"
#endif

// TODAYS BUG: AFTER DOWNLOAD THE START IS NOT SENT FOR SOME REASON.....


int main(int argc, char *argv[])
{

    //QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    qmlRegisterType<GraphPainter>("GraphPainterCpp",1,0,"GraphPainterCpp");
    qRegisterMetaType<DeviceInfo::DeviceType>("DeviceType");
    qRegisterMetaType<DeviceController::AddressType>("AddressType");
    qmlRegisterUncreatableType<DeviceInfo>("Shared", 1, 0, "DeviceType", "Enum is not a type");
    qmlRegisterUncreatableType<DeviceController>("Shared", 1, 0, "AddressType", "Enum is not a type");

    QmlListAdapter* ladapter = new QmlListAdapter();
    TerminalToQmlB term; // unused.. remove?
    term.setActive (true); // true

    LogFileHandler* log_file_handler = new LogFileHandler();

    //    QList<DeviceInterface*> device_interfaces;

    TimeSyncHandler* time_sync_handler = new TimeSyncHandler(ladapter->getList());
    time_sync_handler->start_time_stamp();

    CatchController* catch_controller = new CatchController(ladapter->getList(), time_sync_handler,log_file_handler);

    ConnectionHandler connection_handler; // keeps track of local ble interface

    NetworkManager* ntwMngr = new NetworkManager(log_file_handler);

    //    QBluetoothAddress leftAdapter, rightAdapter; // Local addresses for connection
    //    // search for bt adapters and select an address for both devices
    //    connection_handler.initBtAdapters(leftAdapter, rightAdapter);

    DeviceFinder device_finder(ladapter, &connection_handler, time_sync_handler,catch_controller,log_file_handler);
    qDebug()<<"MainThread ID"<<QThread::currentThreadId();

#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID))
    LinuxTerminalInterface lti;
    engine.rootContext()->setContextProperty("linuxInterface", &lti);
    engine.rootContext()->setContextProperty("QML_OS_LINUX", QVariant(true));
#else
    engine.rootContext()->setContextProperty("QML_OS_LINUX", QVariant(false));
#endif

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

    //    qmlRegisterUncreatableType<DeviceInfo>("com.dev", 1, 0, "DeviceInfo",  QStringLiteral("We do handle object creation only on c++ side in this project") );
    //    qmlRegisterUncreatableType<DeviceInterface>("com.dev", 1, 0, "DeviceInterface",  QStringLiteral("We do handle object creation only on c++ side in this project") );

    ///
    //    qmlRegisterType<QmlListAdapter>("QmlListAdapterCpp",1,0,"QmlListAdapterCpp");
    ////
    engine.rootContext()->setContextProperty("terminalToQml", &term);
    engine.rootContext()->setContextProperty("connectionHandler", &connection_handler);
    engine.rootContext()->setContextProperty("deviceFinder", &device_finder);
    engine.rootContext()->setContextProperty("catchController", catch_controller);
    engine.rootContext()->setContextProperty("ladApter", ladapter);
    //engine.rootContext()->setContextProperty("deviceInterfaces", QVariant::fromValue(device_interfaces));

    engine.rootContext()->setContextProperty("fileHandler", log_file_handler);

    engine.rootContext()->setContextProperty("networkManager", ntwMngr);

    QQuickStyle::setStyle("Material");

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    return app.exec();
}
