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
#include "ble_uart.h"

int main(int argc, char *argv[])
{
    TerminalToQmlB term;

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    //QApplication app(argc, argv);
    // can be changed to QGuiApplication for QML only

    QTimer timer;

    LogFileHandler log_file_handler;
    log_file_handler.set_aut_incr(false);
    //log_file_handler.set_fil_src_cnt(2);
    //log_file_handler.set_last_type(TYPE_PRS);
    ConnectionHandler connection_handler; //keeps track of local ble interface
    DeviceHandler device_handler[2];
    device_handler[0].setRefToFileHandler(&log_file_handler);
    device_handler[0].setIdentifier("LEFT");


    device_handler[1].setRefToFileHandler(&log_file_handler);
    DeviceFinder device_finder(&device_handler[0]);
    device_handler[1].setIdentifier("RIGHT");

    QQmlApplicationEngine engine;
    qmlRegisterUncreatableType<DeviceHandler>("Shared", 1, 0, "AddressType", "Enum is not a type");
    engine.rootContext()->setContextProperty("terminalToQml", &term);
    engine.rootContext()->setContextProperty("connectionHandler", &connection_handler);
    engine.rootContext()->setContextProperty("deviceFinder", &device_finder);
    engine.rootContext()->setContextProperty("deviceHandler_0", &device_handler[0]);
    engine.rootContext()->setContextProperty("deviceHandler_1", &device_handler[1]);


    //QQmlContext *ctxt = engine.rootContext();
    //ctxt->setContextProperty("termModel", QVariant::fromValue(term.m_ioBuff));
    //qDebug() << qApp->primaryScreen()->size();

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    qDebug()<<"Starting Search";
    device_finder.startSearch();


    return app.exec();
}
