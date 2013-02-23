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

#include "profileeditor.h"

#include <QVBoxLayout>
#include "editablepath.h"
#include "cubicbezier.h"
#include "linerestrictor.h"


using namespace fineditors;
using namespace patheditor;

ProfileEditor::ProfileEditor(QWidget *parent) :
    QWidget(parent)
{
    _pathEditor = new patheditor::PathEditorWidget(this);
    _pathEditor->enableFeature(Features::HorizontalAxis);

    QSharedPointer<PathPoint> point1(new PathPoint(0,0));
    QSharedPointer<PathPoint> point2(new PathPoint(60,-24));
    QSharedPointer<PathPoint> point3(new PathPoint(200,0));
    QSharedPointer<PathPoint> point4(new PathPoint(0,-24));

    _topRestrictor = QSharedPointer<Restrictor>(new LineRestrictor(*point4, *point2));

    point1->setRestrictor(_pathEditor->originRestrictor());
    point2->setRestrictor(_topRestrictor);
    point3->setRestrictor(_pathEditor->horizontalAxisRestrictor());

    QSharedPointer<CubicBezier> part1(new CubicBezier(point1, point2));
    QSharedPointer<CubicBezier> part2(new CubicBezier(point2, point3));
    part1->controlPoint2()->setRestrictor(_topRestrictor);
    part2->controlPoint1()->setRestrictor(_topRestrictor);

    EditablePath* path = new EditablePath();
    path->append(part1);
    path->append(part2);

    _pathEditor->addPath(path);

    _mainLayout = new QVBoxLayout(this);
    _mainLayout->addWidget(_pathEditor);
    this->setLayout(_mainLayout);
}