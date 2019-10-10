#include "devicehandler.h"
#include "deviceinterface.h"
#include "deviceinfo.h"

DeviceInterface::DeviceInterface(QObject *parent) : QObject(parent)
{
    m_dev_handler_ptr = new DeviceHandler(); // we have allocate this dinamically, there is no other way to pass it like this to a thread
}

// TODO we can add relevant information to deviceinfo also - or just keep it general
void DeviceInterface::init_device( QBluetoothAddress adapterAddr, DeviceInfo* device )
{
    //connect(this, SIGNAL(signal_printThreadId()), &dh, SLOT(printThreadId()), Qt::QueuedConnection); // old syntax
    //connect(this, &DeviceInterface::signal_printThreadId, &dh, &DeviceHandler::printThreadId, Qt::QueuedConnection); // new syntax
    connect(this, &DeviceInterface::signal_printThreadId, m_dev_handler_ptr, &DeviceHandler::slot_printThreadId, Qt::QueuedConnection); // new syntax
    connect(this, &DeviceInterface::sig_init, m_dev_handler_ptr, &DeviceHandler::init_slot, Qt::QueuedConnection); // new syntax

//    connect(&m_thread_controller, &QThread::started, this, &DeviceInterface::onDeviceThreadStarted); // new syntax

//    m_dev_handler_ptr->setDevice(device);
//    m_dev_handler_ptr->setBtAdapter(adapterAddr);
    m_dev_handler_ptr->moveToThread(&m_thread_controller);

    m_thread_controller.start();

    emit signal_printThreadId();
    emit sig_init(&adapterAddr, device);
}

void DeviceInterface::onDeviceThreadStarted()
{
    qDebug()<<"Device Thread Started!";
}

