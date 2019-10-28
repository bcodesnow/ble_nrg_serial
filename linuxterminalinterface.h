#ifndef LINUXTERMINALINTERFACE_H
#define LINUXTERMINALINTERFACE_H

#include <QObject>
#include <QProcess>

class LinuxTerminalInterface : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString password READ password WRITE password NOTIFY passwordChanged)

public:
    explicit LinuxTerminalInterface(QObject *parent = nullptr);
    QString password() const;

private:
    QString m_password;
    QProcess* m_process;

signals:

    void passwordChanged(QString password);

public slots:
    void executeCmdWithSudo(QString cmd, QString arg1="");

    void password(QString password);
};

#endif // LINUXTERMINALINTERFACE_H
