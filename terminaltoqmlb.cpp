#include "terminaltoqmlb.h"
#include <QtGlobal>

TerminalToQmlB* TerminalToQmlB::handleToThis = NULL;

void TerminalToQmlB::redirectCallback(QtMsgType type, const QMessageLogContext &context, const QString &msg)
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
