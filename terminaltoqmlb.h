#ifndef TERMINALTOQML_H
#define TERMINALTOQML_H

#include <QObject>
#include <QList>
#include <QString>
#include <QColor>
#include "terminaldataobject.h"

#define MAX_MSG_IN_BUFF 50

class TerminalToQmlB : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isActive MEMBER m_isActive NOTIFY isActiveChanged)
private:
    static TerminalToQmlB* handleToThis;

signals:
    void messageArrived(const QString &str);
    void isActiveChanged(bool);
private:

public:
    TerminalToQmlB()
    {
        handleToThis = this;
        qInstallMessageHandler(fakeHandler);
        m_dataList.append(new TerminalDataObject(nullptr,QString("THIS IS A STRING"), QColor("red")));
                m_dataList.append(new TerminalDataObject(nullptr,QString("THIS IS A STRING"), QColor("red")));
                        m_dataList.append(new TerminalDataObject(nullptr,QString("THIS IS A STRING"), QColor("red")));
    }

    static void fakeHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        if (handleToThis)
            handleToThis->redirectCallback(type, context, msg);
    }
    void redirectCallback (QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        if(m_isActive)
        {
            m_ioBuff.prepend(msg);

            if (m_ioBuff.size() > MAX_MSG_IN_BUFF)
                m_ioBuff.removeLast();

            emit messageArrived(m_ioBuff.at(0));

            m_dataList.append(new TerminalDataObject(nullptr, msg, QColor("red")));
        }
        else
        {
            QByteArray localMsg = msg.toLocal8Bit();
    //            const char *file = context.file ? context.file : "";
    //            const char *function = context.function ? context.function : "";
            switch (type) {
            case QtDebugMsg:
                fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
                break;
            case QtWarningMsg:
                fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
                break;
            case QtCriticalMsg:
                fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
                break;
            case QtFatalMsg:
                fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
                abort();
            case QtInfoMsg:
                fprintf(stderr, "QtInfoMsg: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
                break;

            }
        }
    }

    void messageAddedToBuff(void)
    {
        emit messageArrived(m_ioBuff.at(0));
    }

    ~TerminalToQmlB() {};

    bool m_isActive;

    QList<QObject*> m_dataList;
    QStringList m_ioBuff;

//    enum SomeType { A, B, C, D };
//    Q_ENUM(SomeType);
};


#endif // TERMINALTOQML_H
