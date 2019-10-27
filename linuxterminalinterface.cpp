#include "linuxterminalinterface.h"
#include <QDebug>

LinuxTerminalInterface::LinuxTerminalInterface(QObject *parent) : QObject(parent)
{
    m_process = new QProcess();
    m_password = "";
}

QString LinuxTerminalInterface::password() const
{
    return m_password;
}

void LinuxTerminalInterface::executeCmdWithSudo(QString cmd)
{
//    if (m_password == "")
//        .. prompt

    m_process->start("bash");
    m_process->waitForStarted(300);
    //echo PWD | sudo -S + CMD
    const QString toSend = "echo " + m_password + " | sudo -S " + cmd+"\n";
    m_process->write(toSend.toUtf8());
    m_process->waitForFinished(300);
    QString returned = m_process->readAll();
    qDebug()<<"Returned: "<<returned;
    m_process->close();
}

void LinuxTerminalInterface::password(QString password)
{
    if (m_password == password)
        return;

    m_password = password;
    emit passwordChanged(m_password);
}
