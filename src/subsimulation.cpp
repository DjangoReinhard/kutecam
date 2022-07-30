/* 
 * **************************************************************************
 * 
 *  file:       subsimulation.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    11.6.2022 by Django Reinhard
 *  copyright:  (c) 2022 Django Reinhard -  all rights reserved
 * 
 *  This program is free software: you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by 
 *  the Free Software Foundation, either version 2 of the License, or 
 *  (at your option) any later version. 
 *   
 *  This program is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *  GNU General Public License for more details. 
 *   
 *  You should have received a copy of the GNU General Public License 
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 * **************************************************************************
 */
#include "subsimulation.h"
#include "ui_opSim.h"
#include "core.h"
#include "kuteCAM.h"
#include "occtviewer.h"
#include "toolentry.h"
#include "toollistmodel.h"
#include <AIS_Shape.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <QDebug>


SubSimulation::SubSimulation(OperationListModel* olm, TargetDefListModel* tdModel, PathBuilder* pb, QWidget* parent)
 : OperationSubPage(olm, tdModel, pb, parent, false)
 , ui(new Ui::OpSim) {
  ui->setupUi(this);
  connect(ui->pbStop,    &QPushButton::clicked, this, &SubSimulation::stopSimulation);
  connect(ui->pbResume,  &QPushButton::clicked, this, &SubSimulation::resumeSimulation);
  connect(ui->pbRestart, &QPushButton::clicked, this, &SubSimulation::restartSimulation);
  connect(this, &SubSimulation::updatePosition, this, &SubSimulation::moveCone);
  }


void SubSimulation::createTool(int toolNum) {
  int          tn = Core().toolListModel()->findToolNum(toolNum);
  ToolEntry*   activeTool = Core().toolListModel()->tool(tn);
  double       cutRadius = activeTool->fluteDiameter() / 2;
  double       cutHeight = 0;
  TopoDS_Shape cutPart;
  TopoDS_Shape sTool;

  if (!asTool.IsNull()) Core().view3D()->removeShape(asTool);
  if (activeTool->cuttingAngle()) {
     double     cutAngle  = kute::deg2rad(activeTool->cuttingAngle() / 2);

     cutHeight =  cutRadius / tan(cutAngle);
     BRepPrimAPI_MakeCone     mkCone(0, cutRadius, cutHeight);

     cutPart = mkCone.Shape();
     }
  else {
     BRepPrimAPI_MakeCylinder mkCyl(cutRadius, activeTool->fluteLength());

     cutHeight = activeTool->fluteLength();
     cutPart   = mkCyl.Shape();
     }
  BRepPrimAPI_MakeCylinder mkShaft0(cutRadius, activeTool->cuttingDepth() - cutHeight);
  BRepPrimAPI_MakeCylinder mkShaft1(activeTool->shankDiameter() / 2, activeTool->freeLength() - activeTool->cuttingDepth());
  gp_Trsf move = gp_Trsf();

  move.SetTranslation(gp_Pnt(0, 0, 0), gp_Pnt(0, 0, cutHeight));
  TopoDS_Shape movedShape = BRepBuilderAPI_Transform(mkShaft0.Shape(), move, false);

  sTool = BRepAlgoAPI_Fuse(cutPart, movedShape);
  move.SetTranslation(gp_Pnt(0, 0, 0), gp_Pnt(0, 0, activeTool->cuttingDepth()));
  movedShape = BRepBuilderAPI_Transform(mkShaft1.Shape(), move, false);
  sTool = BRepAlgoAPI_Fuse(sTool, movedShape);

  asTool = new AIS_Shape(sTool);
  asTool->SetColor(Quantity_NOC_GRAY);
  asTool->SetTransparency(0.4);

  Core().view3D()->showShape(asTool, false);
  }


void SubSimulation::loadOP(Operation* op) {
  curOP = op;
  }


void SubSimulation::moveCone(const gp_Pnt& pos) {
  gp_Trsf   move;

//  qDebug() << "moveCone: " << x << "/" << y << "/" << z;
  move.SetValues (1, 0, 0, pos.X()
                , 0, 1, 0, pos.Y()
                , 0, 0, 1, pos.Z());
  Core().view3D()->context()->SetLocation(asTool, move);
  Core().view3D()->refresh();
  }


void SubSimulation::processSelection() {
  }


void SubSimulation::restartSimulation() {
  if (!curOP) return;
  createTool(curOP->toolNum());
  timerShapeIndex = 0;
  timerCurveOffset = 0;
  timer.start(1, this);
  }


void SubSimulation::resumeSimulation() {
  timer.start(1, this);
  }


void SubSimulation::stopSimulation() {
  timer.stop();
  }


void SubSimulation::timerEvent(QTimerEvent *e) {
  if (e->timerId() == timer.timerId()) {
     double delta = (double)ui->dStep->value() / 10.0;
     gp_Pnt pos;

     for (;;) {
         Handle(AIS_Shape) curSeg = curOP->toolPaths.at(timerShapeIndex);
         BRepAdaptor_Curve bac(TopoDS::Edge(curSeg->Shape()));
         double            first = bac.FirstParameter();
         double            last  = bac.LastParameter();


         if (kute::isEqual(timerCurveOffset, 0)) {
            timerCurveOffset = first;
            }
         if (timerCurveOffset < last) {
            try {
                GCPnts_AbscissaPoint nextPoint(bac, delta, timerCurveOffset);

                if (nextPoint.IsDone()) {
                   timerCurveOffset = nextPoint.Parameter();
                   if (timerCurveOffset > last) {
                      timerCurveOffset = last;
                      pos = bac.Value(timerCurveOffset);
                      timerCurveOffset = last * 2;
                      }
                   else pos = bac.Value(timerCurveOffset);
                   }
                }
            catch (Standard_Failure const& f) {
                qDebug() << f.GetMessageString();
                }
            }
         else {
            if (++timerShapeIndex >= curOP->toolPaths.size()) {
               timer.stop();
               Core().view3D()->removeShape(asTool);
               Core().view3D()->refresh();
               asTool.Nullify();

               return;
               }
            timerCurveOffset = 0;
            continue;
            }
         break;
         }
     emit updatePosition(pos);
     }
  }


void SubSimulation::genFinishingToolPath() {
  }


// could this work? - start simulation from toolpath call?
void SubSimulation::genRoughingToolPath() {
  restartSimulation();
  }
