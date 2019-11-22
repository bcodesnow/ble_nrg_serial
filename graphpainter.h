#ifndef GRAPHPAINTER_H
#define GRAPHPAINTER_H

//#include "logfilehandler.h"
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
    Q_PROPERTY(QObject *graphData MEMBER m_paintData NOTIFY graphDataChanged)
    Q_PROPERTY(QString painterName WRITE setName READ getName NOTIFY nameChanged)

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

    QString m_name;

public:
    explicit GraphPainter(QQuickItem *parent = nullptr);
    void paint(QPainter *painter) override;

    void setName(QString name);

    QString getName(void);


signals:
    void graphDataChanged();
    void parentChanged();
    void nameChanged();

public slots:
    void onGraphDataChanged();

};

#endif // GRAPHPAINTER_H
