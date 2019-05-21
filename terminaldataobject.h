#ifndef TERMINALDATAOBJECT_H
#define TERMINALDATAOBJECT_H

#include <QObject>
#include <QColor>

class TerminalDataObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString str MEMBER m_string NOTIFY strChanged)
    Q_PROPERTY(QColor clr MEMBER m_color NOTIFY clrChanged)
signals:
    void strChanged();
    void clrChanged();

private:
    QString m_string;
    QColor m_color;
public:
    TerminalDataObject(QObject *parent = nullptr, QString t_str = nullptr, QColor t_clr = nullptr): m_string(t_str), m_color(t_clr)
    {

    }

};
#endif // TERMINALDATAOBJECT_H
