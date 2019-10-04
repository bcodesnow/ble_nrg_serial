#ifndef TERMINALTOQML_H
#define TERMINALTOQML_H

#include <QObject>
#include <QList>
#include <QString>
#include <QColor>

#define MAX_MSG_IN_BUFF 50

class TerminalToQmlB : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isActive MEMBER m_isActive NOTIFY isActiveChanged)

private:
    static TerminalToQmlB* handleToThis;
    bool m_isActive;
    QStringList m_ioBuff;

signals:
    void messageArrived(QString str, QString clr, qint8 fmt);
    void isActiveChanged(bool);

public:
    TerminalToQmlB()
    {
        handleToThis = this;
        //qInstallMessageHandler(fakeHandler);
        m_isActive = false;
    }
    void setActive(bool enable)
    {
        m_isActive = enable;
        emit messageArrived("Terminal Started", "green", 0);
    }

    Q_INVOKABLE void messageFromQml(const QString &str, const QString &clr, const qint8 &fmt)
    {

    }

    static void fakeHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        if (handleToThis)
            handleToThis->redirectCallback(type, context, msg);
    }

    void redirectCallback (QtMsgType type, const QMessageLogContext &context, const QString &msg);


    ~TerminalToQmlB() {}
};


#endif // TERMINALTOQML_H
