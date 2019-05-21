#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "devicefinder.h"
#include "devicehandler.h"
#include <QQmlContext>
#include <QtMessageHandler>
#include <QtGlobal>
#include <QScreen>
#include "terminaltoqmlb.h"

int main(int argc, char *argv[])
{
    TerminalToQmlB term;

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    //QApplication app(argc, argv);
    // can be changed to QGuiApplication for QML only

    DeviceHandler device_handler;
    DeviceFinder device_finder(&device_handler);

    QQmlApplicationEngine engine;
    //qmlRegisterUncreatableType<DeviceHandler>("Shared", 1, 0, "AddressType", "Enum is not a type");

    engine.rootContext()->setContextProperty("terminalToQml", &term);
    //QQmlContext *ctxt = engine.rootContext();
    //ctxt->setContextProperty("termModel", QVariant::fromValue(term.m_ioBuff));
    //qDebug() << qApp->primaryScreen()->size();

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    qDebug()<<"Starting Search";
    device_finder.startSearch();

    return app.exec();
}
