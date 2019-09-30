#ifndef GRAPHPAINTER_H
#define GRAPHPAINTER_H

#include "logfilehandler.h"
#include "paintdata.h"

#include <QObject>
#include <QtQuick/QQuickPaintedItem>
#include <QColor>
#include <QBrush>
#include <QPen>
#include <QPainter>
#include <QVariant>


class GraphPainter : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QObject* graphData MEMBER m_paintData NOTIFY graphDataChanged)
    Q_PROPERTY(QQuickItem *parentRect WRITE setParent NOTIFY parentChanged)
    Q_PROPERTY(QString painterName WRITE setName READ getName NOTIFY nameChanged)

public:
    explicit GraphPainter(QQuickItem *parent = nullptr);
    void paint(QPainter *painter) override;

    void setParent(QQuickItem *prt)
    {
        qDebug()<<"Painter: setting parent to"<<prt;
        m_parent = prt;
        // this->setParent(m_parent);
    }

    Q_INVOKABLE void fillPaintData(QObject *paintdata, QString name)
    {
        if (name == m_name)
        {
            qDebug()<<"Painter: filling new data:"<<m_name;
            m_paintData = paintdata;
            emit graphDataChanged(m_name);
        }
    }

//    Q_INVOKABLE void updatePainting(QString name)
//    {
//        if (m_name == name)
//        {
//            this->update();
//            this->paint(m_painter);
//        }
//    }

    void setName(QString name)
    {
        m_name = name;
        emit nameChanged();
    }

    QString getName(void)
    {
        return m_name;
    }

signals:
    void graphDataChanged(QString graphName);
    void parentChanged();
    void nameChanged();


public slots:


private:


    QObject *m_paintData = nullptr;

    int m_borderPxY = 5;
    int m_borderPxX = 10;
    int m_arrowWidth = 6;
    int m_arrowLength = 10;

    double m_axisWidth = 2;
    double m_lineWidth = 2;

    QColor m_axisColor = "gray";
    QColor m_monoAxisCol = "cyan";
    QColor m_xAxisCol = "red";
    QColor m_yAxisCol = "green";
    QColor m_zAxisCol = "blue";

    bool m_paintAxis = true;
    QQuickItem *m_parent = nullptr;

    QString m_name;

 //   QPainter *m_painter = new QPainter();


};

#endif // GRAPHPAINTER_H
