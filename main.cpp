#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTimer>
#include <QQuickStyle>
#include <QQmlContext>
#include <QQmlContext>
#include <QtMessageHandler>
#include <QtGlobal>
#include <QScreen>

#include <QMediaPlayer>
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

#define BEEP_ON_START 1

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    //QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
#if ( BEEP_ON_START == 1 )
    QMediaPlayer* player = new QMediaPlayer;
    player->setMedia(QUrl("qrc:/common/start.mp3"));
    player->setVolume(100);
    player->play();
#endif


    qmlRegisterType<GraphPainter>("GraphPainterCpp",1,0,"GraphPainterCpp");
    qRegisterMetaType<DeviceInfo::DeviceType>("DeviceType");
    qRegisterMetaType<DeviceController::AddressType>("AddressType");
    qmlRegisterUncreatableType<DeviceInfo>("Shared", 1, 0, "DeviceType", "Enum is not a type");
    qmlRegisterUncreatableType<DeviceController>("Shared", 1, 0, "AddressType", "Enum is not a type");

    QmlListAdapter* ladapter = new QmlListAdapter();
    TerminalToQmlB term; // unused.. remove?
    term.setActive (true); // true

    LogFileHandler* log_file_handler = new LogFileHandler();

    TimeSyncHandler* time_sync_handler = new TimeSyncHandler(ladapter->getList());
    time_sync_handler->start_time_stamp();

    CatchController* catch_controller = new CatchController(ladapter->getList(), time_sync_handler,log_file_handler);

    ConnectionHandler connection_handler; // keeps track of local ble interface

    NetworkManager* ntwMngr = new NetworkManager(log_file_handler);

    DeviceFinder device_finder(ladapter, &connection_handler, time_sync_handler,catch_controller,log_file_handler);
    qDebug()<<"MainThread ID"<<QThread::currentThreadId();

#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID))
    LinuxTerminalInterface lti;
    engine.rootContext()->setContextProperty("linuxInterface", &lti);
    engine.rootContext()->setContextProperty("QML_OS_LINUX", QVariant(true));
#else
    engine.rootContext()->setContextProperty("QML_OS_LINUX", QVariant(false));
#endif

    engine.rootContext()->setContextProperty("terminalToQml", &term);
    engine.rootContext()->setContextProperty("connectionHandler", &connection_handler);
    engine.rootContext()->setContextProperty("deviceFinder", &device_finder);
    engine.rootContext()->setContextProperty("catchController", catch_controller);
    engine.rootContext()->setContextProperty("ladApter", ladapter);

    engine.rootContext()->setContextProperty("fileHandler", log_file_handler);

    engine.rootContext()->setContextProperty("networkManager", ntwMngr);

    QQuickStyle::setStyle("Material");

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    return app.exec();
}
