#include "linuxterminalinterface.h"
#include "linuxterminalinterface.h"
#include <QDebug>

LinuxTerminalInterface::LinuxTerminalInterface(QObject *parent) : QObject(parent)
{
    m_process = new QProcess();
    m_password = "";
    connect(m_process, &QProcess::readyReadStandardOutput, this, &LinuxTerminalInterface::onStdOutPut );
}

QString LinuxTerminalInterface::password() const
{
    return m_password;
}

void LinuxTerminalInterface::executeCmdWithSudo(QString cmd, QString arg1)
{
//    if (m_password == "")
//        .. prompt

    m_process->start("bash");
    m_process->waitForStarted(300);
    //echo PWD | sudo -S + CMD
    QString toSend;
    if (arg1 == "")
        toSend = "echo " + m_password + " | sudo -S " + cmd+"\n";
    else
        toSend = "echo " + m_password + " | sudo -S " + cmd+"\n"; // todo...


    m_process->write(toSend.toUtf8());
    m_process->waitForFinished(300);
    QString returned = m_process->readAll();
    qDebug()<<"""Linux command:"<<toSend<<"Returned:"<<returned;
    m_process->close();

//    echo 'deb blah ... blah' | sudo tee -a /etc/apt/sources.list
//            sudo sh -c "echo 'something' >> /etc/privilegedFile"
}

void LinuxTerminalInterface::writeValueToProtectedFile(QString pathToFile, int value)
{
    QString toSend = "sudo -k && echo -e '"  +m_password + "\n"+ QString::number(value) + "' | sudo -S tee " + pathToFile + " > /dev/null 2>&1"; // replace > with -a to append..
    m_process->start(toSend);
    //m_process->waitForStarted(300);
//    m_process->write(toSend.toUtf8());
    m_process->waitForFinished(300);
    QString returned = m_process->readAll();
    qDebug()<<"Linux command:"<<toSend<<"Returned:"<<returned;
    m_process->close();

}

void LinuxTerminalInterface::executeTestCmd()
{
    QString toSend = "echo 6 | sudo tee /sys/kernel/debug/bluetooth/hci0/conn_min_interval";

    m_process->start("bash");
     m_process->waitForStarted(300);
    m_process->write(toSend.toUtf8());
    m_process->waitForFinished(300);
    QString returned = m_process->readAll();
    qDebug()<<"Linux command:"<<toSend<<"Returned:"<<returned;
    m_process->close();
}

void LinuxTerminalInterface::setPassword(QString password)
{
    if (m_password == password)
        return;
    m_password = password;
    emit passwordChanged(m_password);
    qDebug()<<"password:"<<m_password;
}

void LinuxTerminalInterface::onStdOutPut()
{
    qDebug()<<"!!onStdOutp";
    qDebug()<<m_process->readAllStandardOutput();
}

void LinuxTerminalInterface::writeAllSudoCommands()
{
    qDebug()<<"PW hinterlegt, jetzt conn_min und conn max setzen..\n  echo 16 > /sys/kernel/debug/bluetooth/hci0/conn_min_interval \necho 17 > /sys/kernel/debug/bluetooth/hci0/conn_max_interval ";

    //  lti.setPassword("SetYourSudoerPW");
      //lti.executeTestCmd();
      this->writeValueToProtectedFile("/sys/kernel/debug/bluetooth/hci0/conn_min_interval", 6);
      //    lti.executeCmdWithSudo("echo 6 > /sys/kernel/debug/bluetooth/hci0/conn_min_interval");
      //    lti.executeCmdWithSudo("echo 9 > /sys/kernel/debug/bluetooth/hci0/conn_max_interval");
      //lti.executeCmdWithSudo("cat /sys/kernel/debug/bluetooth/hci0/conn_min_interval");
      //lti.executeCmdWithSudo("cat /sys/kernel/debug/bluetooth/hci0/conn_max_interval");
}
