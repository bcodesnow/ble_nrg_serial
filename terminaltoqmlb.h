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
        qInstallMessageHandler(fakeHandler);
    }

    Q_INVOKABLE void messageFromQml(const QString &str, const QString &clr, const qint8 &fmt)
    {

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

            QString tmp_color;
            qint8 tmp_format;

            switch (type) {
            case QtDebugMsg:
                tmp_color = "white";
                tmp_format = 0;
            break;
            case QtWarningMsg:
                tmp_color = "yellow";
                tmp_format = 1;
                break;
            case QtCriticalMsg:
                tmp_color = "red";
                tmp_format = 0;
                break;
            case QtFatalMsg:
                tmp_color = "red";
                tmp_format = 1;
                abort();
            case QtInfoMsg:
                tmp_color = "green";
                tmp_format = 0;
                break;
            }
            emit messageArrived(m_ioBuff.at(0), tmp_color, tmp_format);
        }
        else
        {
            QByteArray localMsg = msg.toLocal8Bit();
            //const char *file = context.file ? context.file : "";
            //const char *function = context.function ? context.function : "";
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

    ~TerminalToQmlB() {}
};


#endif // TERMINALTOQML_H
