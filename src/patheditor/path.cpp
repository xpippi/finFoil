/****************************************************************************
  
 Copyright (c) 2013, Hans Robeers
 All rights reserved.
 
 BSD 2-Clause License
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 
   * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
   
   * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
   
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
****************************************************************************/

#include "path.h"
#include "pathtemplates.hpp"

#include <QVarLengthArray>
#include <QJsonArray>
#include "patheditor/line.h"
#include "patheditor/cubicbezier.h"

using namespace patheditor;

Path::Path(QObject *parent) :
    QObject(parent)
{
}

void Path::append(std::shared_ptr<PathItem> pathItem)
{
    // Set pathItem next and prev
    if (!_pathItemList.isEmpty())
    {
        std::shared_ptr<PathItem> last = _pathItemList.last();
        last->setNextPathItem(pathItem);
        pathItem->setPrevPathItem(last);
    }

    if (!_pathItemList.isEmpty())
    {
        pathItem->setStartPoint(_pathItemList.last()->endPoint());
    }

    emit onAppend(pathItem.get());
    _pathItemList.append(pathItem);
}

QList<std::shared_ptr<PathItem> > Path::pathItems()
{
    return _pathItemList;
}

QList<const PathItem *> Path::constPathItems() const
{
    return jenson::toConstList(_pathItemList);
}

QRectF Path::controlPointRect() const
{
    if (_pathItemList.count() <= 0)
        return QRectF(0,0,0,0);

    QRectF retVal = _pathItemList.first()->controlPointRect();
    foreach(std::shared_ptr<PathItem> item, _pathItemList)
    {
        retVal |= item->controlPointRect();
    }

    return retVal;
}

QPointF Path::pointAtPercent(qreal t) const
{
    int pathItemCount = _pathItemList.count();
    qreal itemRange = 1/qreal(pathItemCount);

    int item = 0;
    while (t > itemRange)
    {
        t -= itemRange;
        item++;
    }

    t = t/itemRange;

    return _pathItemList[item]->pointAtPercent(t);
}

qreal Path::minX(qreal *t_top) const
{
    return extreme<X, Min>(this, t_top);
}

qreal Path::maxX(qreal *t_top) const
{
    return extreme<X, Max>(this, t_top);
}

qreal Path::minY(qreal *t_top) const
{
    return extreme<Y, Min>(this, t_top);
}

qreal Path::maxY(qreal *t_top) const
{
    return extreme<Y, Max>(this, t_top);
}

qreal Path::area(int resolution) const
{
    qreal percStep = 1 / qreal(resolution);
    QVarLengthArray<QPointF, PATH_AREARES> points(resolution);
    qreal perc = 0;
    for (int i = 0; i < resolution; i++)
    {
        points[i] = pointAtPercent(perc);
        perc += percStep;
    }

    qreal area = 0;
    int j = 0;
    for (int i = 0; i < resolution; i++)
    {
        j = (i + 1) % resolution;
        area += points[i].x() * points[j].y();
        area -= points[j].x() * points[i].y();
    }

    return qAbs(area) / 2;
}

void Path::onPathChanged()
{
    emit pathChanged(this);
}

void Path::onPathReleased()
{
    emit pathReleased(this);
}


static void appendPntToJsonArray(const PathPoint *pnt, QJsonArray *array)
{
    array->append(pnt->x());
    array->append(pnt->y());
}

QJsonValue PathSerializer::serializeImpl(const Path *object) const
{
    QJsonArray retVal;

    // Move to first point
    QJsonArray moveToStart;
    moveToStart.append(QStringLiteral("M"));
    ::appendPntToJsonArray(object->constPathItems().first()->constStartPoint(), &moveToStart);
    retVal.append(moveToStart);

    foreach (const PathItem *pathItem, object->constPathItems())
    {
        QJsonArray nestedArr;

        switch (pathItem->constControlPoints().count())
        {
        case 0:
            nestedArr.append(QStringLiteral("L"));
            break;

        case 2:
            nestedArr.append(QStringLiteral("C"));
            break;

        default:
            QString msg = QStringLiteral("PathSerializer has no implementation to serialize a path with ")
                    + pathItem->constControlPoints().count()
                    + QStringLiteral(" controlpoints.");
            throw jenson::SerializationException(msg);
        }

        foreach (const ControlPoint *pnt, pathItem->constControlPoints())
            ::appendPntToJsonArray(pnt, &nestedArr);

        ::appendPntToJsonArray(pathItem->constEndPoint(), &nestedArr);

        retVal.append(nestedArr);
    }

    return retVal;
}

template <typename Tpnt>
static std::shared_ptr<Tpnt> takePoint(QJsonArray *pntArray)
{
    std::shared_ptr<Tpnt> retVal;

    QJsonValue xval = pntArray->takeAt(0), yval = pntArray->takeAt(0);

    if(xval.isDouble() && yval.isDouble())
    {
        retVal.reset(new Tpnt(xval.toDouble(), yval.toDouble()));
    }

    return retVal;
}

static std::shared_ptr<PathItem> toPathItem(QJsonArray pathItemJson)
{
    static thread_local std::shared_ptr<PathPoint> s_previousEndPoint;

    std::shared_ptr<PathItem> retVal;

    QString typeId = pathItemJson.takeAt(0).toString();

    if (typeId == QStringLiteral("M"))
    {
        s_previousEndPoint = ::takePoint<PathPoint>(&pathItemJson);
    }
    else if (typeId == QStringLiteral("L"))
    {
        std::shared_ptr<PathPoint> endPnt = ::takePoint<PathPoint>(&pathItemJson);
        if (s_previousEndPoint && endPnt)
            retVal.reset(new Line(s_previousEndPoint, endPnt));
        s_previousEndPoint = endPnt;
    }
    else if (typeId == QStringLiteral("C"))
    {
        std::shared_ptr<ControlPoint> cPnt1 = ::takePoint<ControlPoint>(&pathItemJson);
        std::shared_ptr<ControlPoint> cPnt2 = ::takePoint<ControlPoint>(&pathItemJson);
        std::shared_ptr<PathPoint> endPnt = ::takePoint<PathPoint>(&pathItemJson);
        if (s_previousEndPoint && cPnt1 && cPnt2 && endPnt)
            retVal.reset(new CubicBezier(s_previousEndPoint, cPnt1, cPnt2, endPnt));
        s_previousEndPoint = endPnt;
    }

    return retVal;
}

std::unique_ptr<Path> PathSerializer::deserializeImpl(const QJsonValue *jsonValue, QString *errorMsg) const
{
    if (!jsonValue->isArray())
    {
        if (errorMsg) errorMsg->append("\n PathSerializer::deserializeImpl -> path is not a json array");
        return nullptr;
    }

    std::unique_ptr<Path> path(new Path());

    QJsonArray array = jsonValue->toArray();

    foreach (QJsonValue value, array)
    {
        if (value.isArray())
        {
            std::shared_ptr<PathItem> pathItem = ::toPathItem(value.toArray());

            if (pathItem)
                path->append(pathItem);
        }
    }

    return path;
}