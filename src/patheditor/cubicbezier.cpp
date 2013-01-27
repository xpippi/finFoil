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

#include "cubicbezier.h"

using namespace patheditor;

CubicBezier::CubicBezier(QSharedPointer<RestrictablePoint> startPoint, QSharedPointer<RestrictablePoint> endPoint,
                         QGraphicsItem *parent, QGraphicsScene *scene)
    : PathItem(startPoint, endPoint, parent, scene)
{
    QPointF startToEnd = *endPoint - *startPoint;
    QPointF increment = startToEnd / 3;
    _cPoint1 = QSharedPointer<RestrictablePoint>(new RestrictablePoint(0,0));
    _cPoint2 = QSharedPointer<RestrictablePoint>(new RestrictablePoint(0,0));
    *_cPoint1 += *startPoint + increment;
    *_cPoint2 += *_cPoint1 + increment;


    _controlPoints.append(_cPoint1);
    _controlPoints.append(_cPoint2);
}

QSharedPointer<RestrictablePoint> CubicBezier::controlPoint1()
{
    return _cPoint1;
}

QSharedPointer<RestrictablePoint> CubicBezier::controlPoint2()
{
    return _cPoint2;
}

QList<QSharedPointer<RestrictablePoint> > CubicBezier::controlPoints()
{
    return _controlPoints;
}

QRectF CubicBezier::boundingRect() const
{
    return _boundingRect;
}

void CubicBezier::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPainterPath painterPath;

    painterPath.moveTo(*startPoint());
    painterPath.cubicTo(*_cPoint1, *_cPoint2, *endPoint());

    painter->drawPath(painterPath);

    // faster than bounding rect
    _boundingRect = painterPath.controlPointRect();
}

void CubicBezier::paint(PathSettings *settings, QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setPen(settings->linePen());
    paint(painter, option, widget);

    painter->setPen(settings->controlLinePen());
    QPainterPath painterPath;
    painterPath.moveTo(*startPoint());
    painterPath.lineTo(*_cPoint1);
    painterPath.moveTo(*endPoint());
    painterPath.lineTo(*_cPoint2);

    painter->drawPath(painterPath);
}