/* 
 * **************************************************************************
 * 
 *  file:       subopdrill.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    23.4.2022 by Django Reinhard
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
#include "subopdrill.h"
#include "ui_opSub.h"
#include "ui_mainwindow.h"
#include "operationlistmodel.h"
#include "core.h"
#include "cuttingparameters.h"
#include "kuteCAM.h"
#include "projectfile.h"
#include "drilltargetdefinition.h"
#include "targetdeflistmodel.h"
#include "toollistmodel.h"
#include "toolentry.h"
#include "occtviewer.h"
#include "util3d.h"
#include "work.h"
#include "wscycle.h"
#include "wstraverse.h"
#include <BRep_Tool.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <QAction>
#include <QStringListModel>
#include <QDebug>


SubOPDrill::SubOPDrill(OperationListModel* olm, TargetDefListModel* tdModel, PathBuilder* pb, QWidget *parent)
 : OperationSubPage(olm, tdModel, pb, parent) {
  ui->lDir->setVisible(false);
  ui->cbDir->setVisible(false);
  ui->lStep->setText(tr("[min/max]"));
  QLabel* lDeltaDec = new QLabel(tr("reduce drilldepth by"));

  ui->gridLayoutX->addWidget(lDeltaDec, 8, 0, 1, 2);
  ui->label_3->setText(tr("Drilldepth"));
  //TODO: add label to replace "cInside"
  ui->cInside->setVisible(false); // row:8 col:0
  ui->cbType->setVisible(false);
  //TODO: use spOff as delta decrement
//  ui->spOff->setVisible(false);
  connect(Core().uiMainWin()->actionDrillNew, &QAction::triggered, this, &SubOPDrill::createOP);
  }


void SubOPDrill::createOP() {
  int mx = olm->rowCount();

  OperationSubPage::createOP(mx, QString(tr("Drill #%1")).arg(mx), DrillOperation);
  connectSignals();
  toolChanged(0);
  }


// drill operation may be based on different selections:
// - circle selection (should be topmost circle of hole
// - cylindrical face selection (the inner face of the hole)
// - plane face selection (every hole of the face will be drill target)
void SubOPDrill::processSelection() {
  std::vector<TopoDS_Shape> selection = Core().view3D()->selection();

  curOP->setUpperZ(curOP->wpBounds.CornerMax().Z());
  curOP->setLowerZ(curOP->wpBounds.CornerMin().Z());
  curOP->setTopZ(curOP->mBounds.CornerMax().Z());

  for (auto s : selection) {
      Handle(AIS_Shape) asTmp = new AIS_Shape(s);
      Bnd_Box           bbSel = asTmp->BoundingBox(); bbSel.SetGap(0);

      if (s.ShapeType() == TopAbs_EDGE) {
         TopoDS_Edge  edge = TopoDS::Edge(s);

         if (BRep_Tool::IsGeometric(edge)) {
            double first, last;
            Handle(Geom_Curve) c = BRep_Tool::Curve(edge, first, last);

            if (c->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
               Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(c);
               const gp_Pnt&       pos    = circle->Position().Location();
               const gp_Dir&       dir    = circle->Position().Direction();
               double              radius = circle->Radius();

               if (kute::isVertical(dir)) {
                  // OK, circle is valid location for drill OP
                  //TODO: do we have to care about Z-location from selection?
                  curOP->setNominalZ(pos.Z());
                  tdModel->append(new DrillTargetDefinition(pos, dir, radius));
                  }
               }
            }
         }
      else if (s.ShapeType() == TopAbs_FACE) {
         Handle(Geom_Surface) selectedFace  = BRep_Tool::Surface(TopoDS::Face(s));

         if (selectedFace->IsKind(STANDARD_TYPE(Geom_Plane))) {
            std::vector<TopoDS_Edge> edges = Core().helper3D()->allEdgesWithin(s);

            for (auto e : edges) {
                if (BRep_Tool::IsGeometric(e)) {
                   double first, last;
                   Handle(Geom_Curve) c = BRep_Tool::Curve(e, first, last);

                   if (c->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
                      Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(c);
                      const gp_Pnt&       pos    = circle->Position().Location();
                      const gp_Dir&       dir    = circle->Position().Direction();
                      double              radius = circle->Radius();

                      if (kute::isVertical(dir)) {
                         // OK, circle is valid location for drill OP
                         curOP->setNominalZ(pos.Z());
                         tdModel->append(new DrillTargetDefinition(pos, dir, radius));
                         }
                      }
                   } // ignore all edges that are not circles
                }
            }
         else {
            // suppose cylindrical face from hole
            std::vector<TopoDS_Edge> edges = Core().helper3D()->allEdgesWithin(s);

//            curOP->setLowerZ(bbSel.CornerMin().Z());
//            curOP->setUpperZ(bbSel.CornerMax().Z());
            for (auto e : edges) {
                if (BRep_Tool::IsGeometric(e)) {
                   double first, last;
                   Handle(Geom_Curve) c = BRep_Tool::Curve(e, first, last);

                   if (c->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
                      Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(c);
                      const gp_Pnt&       pos    = circle->Position().Location();
                      const gp_Dir&       dir    = circle->Position().Direction();
                      double              radius = circle->Radius();

                      if (kute::isVertical(dir)) {
                         // OK, first circle will do
                         curOP->setNominalZ(pos.Z());
                         tdModel->append(new DrillTargetDefinition(pos, dir, radius));
                         break; // don't care for rest of edges
                         }
                      }
                   } // ignore all edges that are not circles
                }
            }
         }
      else {
         throw QString("unsupported selection type!");
         }
      }
  ui->spDepth->setValue(curOP->topZ());
  ui->cAbsolute->setChecked(true);
  if (!validateDrillTargets()) {
     //TODO: what should we do with different hole radius?
     qDebug() << "holes from selection don't have same diameter. Proceed, Ignore, what?!?";
     }
  }


void SubOPDrill::showToolPath(Operation* op) {
  if (!op->workSteps().size()) return;
//  Workstep* ws = op->workSteps().at(0);
//  WSCycle*  wc = static_cast<WSCycle*>(ws);
  QVector<double> zStops;
  double drillDelta = op->upperZ() + op->safeZ0() - op->drillDepth();

  if (op->drillCycle() != 4 || abs(drillDelta) < op->qMin()) {
     zStops.append(op->drillDepth());
     }
  else {
     double qDelta   = op->qMax() - op->qMin();
     double maxSteps = drillDelta / op->qMin();
     double qStep    = qDelta / maxSteps;
     double curStep  = op->qMax();
     double curZ     = op->upperZ() + op->safeZ0();

     while ((curZ - curStep) > op->drillDepth()) {
           curZ -= curStep;
           zStops.append(curZ);
           curStep -= qStep;
           }
     zStops.append(op->drillDepth());
     }
  gp_Pnt lastPos(0, 0, 300);
  double zS0  = op->upperZ() + op->safeZ0();
  double zS1  = op->upperZ() + op->safeZ1();

  op->toolPaths.clear();
  for (auto ws: op->workSteps()) {
      gp_Pnt from = lastPos;
      gp_Pnt to(ws->startPos().X(), ws->startPos().Y(), zS1);
      Handle(AIS_Shape) s = Core().helper3D()->createLine(from, to);

      s->SetColor(Quantity_NOC_CYAN);
      op->toolPaths.push_back(s);

      from = to;
      to.SetZ(zS0);
      s = Core().helper3D()->createLine(from, to);
      s->SetColor(Quantity_NOC_CYAN);
      op->toolPaths.push_back(s);

      from = to;
      to.SetZ(op->drillDepth());
      s = Core().helper3D()->createLine(from, to);
      s->SetColor(Quantity_NOC_RED);
      s->SetWidth(3);
      op->toolPaths.push_back(s);

      for (double z : zStops) {
          to.SetZ(z);
          Core().view3D()->createAxisCross(to, 1, &op->toolPaths, Quantity_NOC_RED);
          }
      lastPos = from;
      lastPos.SetZ(zS1);
      }
  Core().view3D()->showShapes(op->toolPaths);
  Core().view3D()->refresh();
  }


void SubOPDrill::genFinishingToolPath() {
  genRoughingToolPath();
  }


void SubOPDrill::genRoughingToolPath() {
  double    absDrillDepth = curOP->upperZ() + curOP->finalDepth();
  double    drillStart = curOP->upperZ() + curOP->safeZ0();

  //TODO: need to calculate drill depth ...

  if (curOP->isAbsolute()) absDrillDepth = curOP->finalDepth();
  double modelTop = curOP->mBounds.CornerMax().Z();
  double wpTop    = curOP->mBounds.CornerMax().Z();
  double wpMin    = curOP->mBounds.CornerMin().Z();

//  double safeZ0     = curOP->topZ() + curOP->safeZ0();
//  double safeZ1     = curOP->topZ() + curOP->safeZ1();

//  qDebug() << "toolpath generation - change lower z from" << curOP->lowerZ() << "\tto" << drillDepth;

//  curOP->setLowerZ(drillDepth);
  curOP->setDrillDepth(absDrillDepth);
  tdModel->sort();
  for (TargetDefinition* td : tdModel->itemList()) {
      DrillTargetDefinition* dtd = dynamic_cast<DrillTargetDefinition*>(td);
      gp_Pnt                 from(dtd->pos().X(), dtd->pos().Y(), curOP->upperZ());   // Z is not used
      gp_Pnt                 to(dtd->pos().X(), dtd->pos().Y(), 0 /* drillDepth */);  // on drill positions

      curOP->workSteps().push_back(new WSCycle(curOP->drillCycle(), from, to /* , safeZ0, safeZ1, drillDepth */));
      }
  qDebug() << "toolpath consists of" << curOP->workSteps().size() << "items";
  showToolPath(curOP);
  }


bool SubOPDrill::validateDrillTargets() {
  double oldR = 0;
  bool   seenDifferentRadius = false;

  for (TargetDefinition* td: tdModel->itemList()) {
      DrillTargetDefinition* dtd = static_cast<DrillTargetDefinition*>(td);

      if (kute::isEqual(oldR, 0)) oldR = dtd->radius();
      if (!kute::isEqual(oldR, dtd->radius()))
         seenDifferentRadius = true;
      oldR = dtd->radius();
      }
  return !seenDifferentRadius;
  }
