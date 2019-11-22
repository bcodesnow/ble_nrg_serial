#include "graphpainter.h"

GraphPainter::GraphPainter(QQuickItem *parent) :
    QQuickPaintedItem(parent)
{
    //connect(this, &GraphPainter::graphDataChanged, this, &GraphPainter::onGraphDataChanged);
}

void GraphPainter::paint(QPainter *painter)
{
    if (m_paintData == nullptr)
    {
        return;
    }
    QRectF borders = QQuickItem::boundingRect();
    QVector<QVariant> ampVector = (((PaintData*) m_paintData)->getPaintBuffer());
    QVector<double> timeVector;
    timeVector = ((PaintData*) m_paintData)->getTimeVector();
    if (ampVector.isEmpty() || timeVector.isEmpty()) {
        return;
    }
#if (VERBOSITY_LEVEL >= 2)
    qDebug()<<"GraphPainter::paint()"<<m_name<<"time: "<<timeVector.size()<<"amp: "<<ampVector.size();
#endif

    // values needed for drawing
    bool paintAxisMid = false;
    bool paintAxisBottom = false;
    int x0, y0;
    double minVal, maxVal, diffVal;
    double quadrantSize;

    // set painter options
    QPen pen;
    painter->setRenderHints(QPainter::Antialiasing, true);

    // set origin
    x0 = m_borderPxX;

    // minima/maxima
    minVal = (*std::min_element(ampVector.constBegin(), ampVector.constEnd())).toDouble();
    maxVal = (*std::max_element(ampVector.constBegin(), ampVector.constEnd())).toDouble();

    // scale time vector
    for (int i=0;i<timeVector.size();i++) {
        timeVector[i] = timeVector[i]/timeVector.at(timeVector.size()-1)
                * (borders.width()-m_borderPxX*3) + m_borderPxX;
    }

    // paint it
    switch (((PaintData*) m_paintData)->getType()) {
    case TYPE_AUD:
        // 1 axis, + -
        paintAxisMid = true;
        quadrantSize = borders.height()/2;
        // scale value vector
        for (int i=0;i<ampVector.size();i++)
        {
            if (ampVector[i] >= 0)
            {
                ampVector[i] = ampVector[i].toDouble()/maxVal*(quadrantSize-m_borderPxY);
            }
            else {
                ampVector[i] = -ampVector[i].toDouble()/minVal*(quadrantSize-m_borderPxY);
            }
        }
        y0 = static_cast<int>(borders.height()/2);
        pen = QPen(m_monoAxisCol);
        pen.setWidthF(m_lineWidth);
        painter->setPen(pen);
        for (int i=1;i<ampVector.size();i++) {
            painter->drawLine(static_cast<int>(timeVector[i-1]),y0+ampVector.at(i-1).toInt(),
                    static_cast<int>(timeVector[i]), y0+ampVector.at(i).toInt());
        }
        break;
    case TYPE_PRS:
        // 1 axis, +
        paintAxisBottom = true;
        quadrantSize = borders.height();
        // scale value vector
        diffVal = maxVal - minVal;
        for (int i=0;i<ampVector.size();i++)
        {
            if (ampVector[i] >= 0)
            {
                ampVector[i] = -( (ampVector[i].toDouble()-minVal)/diffVal )*( quadrantSize - m_borderPxY );
            }
            else {
                ampVector[i] = ( (ampVector[i].toDouble()-minVal)/diffVal )*( quadrantSize - m_borderPxY );
            }
        }
        y0 = static_cast<int>(borders.height() - m_borderPxY);
        pen = QPen(m_monoAxisCol);
        pen.setWidthF(m_lineWidth);
        painter->setPen(pen);
        for (int i=1;i<ampVector.size();i++) {
            painter->drawLine(static_cast<int>(timeVector[i-1]),y0+ampVector.at(i-1).toInt(),
                    static_cast<int>(timeVector[i]), y0+ampVector.at(i).toInt());
        }
        break;
    case TYPE_GYR:
        // 3 axis, + -
        Q_FALLTHROUGH();
    case TYPE_ACC:
        // 3 axis, + -
        Q_FALLTHROUGH();
    case TYPE_MAG:
        // 3 axis, + -
        paintAxisMid = true;
        quadrantSize = borders.height()/2;
        // scale value vector
        for (int i=0;i<ampVector.size();i++)
        {
            if (ampVector[i] >= 0)
            {
                ampVector[i] = ampVector[i].toDouble()/maxVal*(quadrantSize-m_borderPxY);
            }
            else {
                ampVector[i] = -ampVector[i].toDouble()/minVal*(quadrantSize-m_borderPxY);
            }
        }
        y0 = static_cast<int>(borders.height()/2);
        int x1, x2, y1, y2;
        int j;
        // x axis
        pen = QPen(m_xAxisCol);
        pen.setWidthF(m_lineWidth);
        painter->setPen(pen);
        for (int i=1;i<timeVector.size();i++) {
            // time axis
            x1 = static_cast<int>(timeVector[i-1]);
            x2 = static_cast<int>(timeVector[i]);
            j=(i-1)*3;
            y1 = y0+ampVector.at(j).toInt();
            y2 = y0+ampVector.at(j+3).toInt();
            painter->drawLine(x1, y1, x2, y2);
        }
        // y axis
        pen = QPen(m_yAxisCol);
        pen.setWidthF(m_lineWidth);
        painter->setPen(pen);
        for (int i=1;i<timeVector.size();i++) {
            // time axis
            x1 = static_cast<int>(timeVector[i-1]);
            x2 = static_cast<int>(timeVector[i]);
            j=((i-1)*3)+1;
            y1 = y0+ampVector.at(j).toInt();
            y2 = y0+ampVector.at(j+3).toInt();
            painter->drawLine(x1, y1, x2, y2);
        }
        // z axis
        pen = QPen(m_zAxisCol);
        pen.setWidthF(m_lineWidth);
        painter->setPen(pen);
        for (int i=1;i<timeVector.size();i++) {
            // time axis
            x1 = static_cast<int>(timeVector[i-1]);
            x2 = static_cast<int>(timeVector[i]);
            j=((i-1)*3)+2;
            y1 = y0+ampVector.at(j).toInt();
            y2 = y0+ampVector.at(j+3).toInt();
            painter->drawLine(x1, y1, x2, y2);
        }
        break;
    case TYPE_LOG:
        return;
    default:
        return;
    }

    // paint coordinate system
    if (m_paintAxis)
    {
        pen = QPen(m_axisColor);
        pen.setWidthF(m_axisWidth);
        painter->setPen(pen);
        painter->setRenderHints(QPainter::Antialiasing, true);
        double tmp;
        QPainterPath path;

        // paint X axis
        path.moveTo(x0,y0);
        tmp = borders.width()-m_borderPxX/2;
        path.lineTo(tmp,y0);
        path.lineTo(tmp-m_arrowLength, y0+m_arrowWidth/2);
        path.moveTo(tmp,y0);
        path.lineTo(tmp-m_arrowLength, y0-m_arrowWidth/2);

        // paint Y axis
        path.moveTo(x0,y0);
        if (paintAxisMid) {
            tmp = -(borders.height()/2-m_borderPxY);
            path.lineTo(x0, y0+tmp);
            path.lineTo(x0-m_arrowWidth/2, y0+tmp+m_arrowLength);
            path.moveTo(x0, y0+tmp);
            path.lineTo(x0+m_arrowWidth/2, y0+tmp+m_arrowLength);
            path.moveTo(x0,y0);
            path.lineTo(x0, y0-tmp);
        }
        else if (paintAxisBottom) {
            tmp = -(borders.height()-m_borderPxY);
            path.lineTo(x0, y0+tmp);
            path.lineTo(x0-m_arrowWidth/2, y0+tmp+m_arrowLength);
            path.moveTo(x0, y0+tmp);
            path.lineTo(x0+m_arrowWidth/2, y0+tmp+m_arrowLength);
        }
        // Draw!
        painter->drawPath(path);
    }
}

void GraphPainter::setName(QString name)
{
    m_name = name;
    emit nameChanged();
}

QString GraphPainter::getName()
{
    return m_name;
}

//void GraphPainter::onGraphDataChanged()
//{
//    this->update();
//    qDebug()<<"updated"<<m_name;
//}

