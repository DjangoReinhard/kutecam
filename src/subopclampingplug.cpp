/* 
 * **************************************************************************
 * 
 *  file:       subopclampingplug.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    29.4.2022 by Django Reinhard
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
#include "subopclampingplug.h"
#include "ui_opSub.h"
#include "ui_mainwindow.h"
#include "core.h"
#include "cctargetdefinition.h"
#include "gocontour.h"
#include "goline.h"
#include "occtviewer.h"
#include "operationlistmodel.h"
#include "pathbuilder.h"
#include "targetdeflistmodel.h"
#include "util3d.h"
#include "work.h"
#include "workstep.h"
#include "wsarc.h"
#include <QDebug>


SubOPClampingPlug::SubOPClampingPlug(OperationListModel* olm, TargetDefListModel* tdModel, QWidget* parent)
 : OperationSubPage(olm, tdModel, parent) {
  ui->lCycle->setVisible(false);
  ui->cbCycle->setVisible(false);
  ui->lRetract->setVisible(false);
  ui->spRetract->setVisible(false);
  ui->spDwell->setVisible(false);
  connect(Core().uiMainWin()->actionClamping_Plug, &QAction::triggered, this, &SubOPClampingPlug::createOP);
  }


void SubOPClampingPlug::createOP() {
  int mx = olm->rowCount();

  if (!Core().workData()->cpOnTop) {
     qDebug() << "clamping plug is not on top, so NO clamping plug operation!";
     return;
     }
  qDebug() << "clamping plug is on top, so create clamping plug operation ...";
  OperationSubPage::createOP(mx, QString(tr("ClampingPlug #%1")).arg(mx), ClampingPlugOP);
  connectSignals();
  }


int SubOPClampingPlug::findMax(double d0, double d1, double d2, double d3) {
  double v[] = { d0, d1, d2, d3, 0};
  int    i=0, j=0;

  for (; i < 4; ++i) {
      for (j=i+1; j < 4; ++j) {
          if (v[j] > v[i]) break;
          }
      qDebug() << "what: " << v[i] << " - " << v[j];
      if (v[i] > v[j]) break;
      }
  return fmax(0, i);
  }


int SubOPClampingPlug::findMax(int d0, int d1, int d2, int d3) {
  int v[] = { d0, d1, d2, d3, 0};
  int i=0, j=0;

  for (; i < 4; ++i) {
      for (j=i+1; j < 4; ++j) {
          if (v[j] > v[i]) break;
          }
      qDebug() << "what: " << v[i] << " - " << v[j];
      if (v[i] > v[j]) break;
      }
  return fmax(0, i);
  }


// create target definition and add it to tdModel
void SubOPClampingPlug::processSelection() {
  Bnd_Box bbCP = Core().workData()->clampingPlug->BoundingBox();
  CCTargetDefinition* ctd = new CCTargetDefinition(bbCP.CornerMin(), bbCP.CornerMax());
  GOContour* c = new GOContour(Core().helper3D()->centerOf(bbCP));
  gp_Pnt p0, p1;

//  cp has corners   -16  /  -50  /   55    and    74  /  50  /  65
//  cp has workpiece -61  /  -55  /  -95    and    79  /  55  /  55
  qDebug() << "cp has corners" << bbCP.CornerMin().X() << " / " << bbCP.CornerMin().Y() << " / " << bbCP.CornerMin().Z()
           << "   and   "   << bbCP.CornerMax().X() << " / " << bbCP.CornerMax().Y() << " / " << bbCP.CornerMax().Z();
  p0 = gp_Pnt(bbCP.CornerMax().X(), bbCP.CornerMin().Y(), bbCP.CornerMin().Z());
  p1 = gp_Pnt(bbCP.CornerMin().X(), bbCP.CornerMin().Y(), bbCP.CornerMin().Z());
  c->add(new GOLine(p0, p1));
  p0 = p1;
  p1 = gp_Pnt(bbCP.CornerMin().X(), bbCP.CornerMax().Y(), bbCP.CornerMin().Z());
  c->add(new GOLine(p0, p1));
  p0 = p1;
  p1 = gp_Pnt(bbCP.CornerMax().X(), bbCP.CornerMax().Y(), bbCP.CornerMin().Z());
  c->add(new GOLine(p0, p1));
  p0 = p1;
  p1 = gp_Pnt(bbCP.CornerMax().X(), bbCP.CornerMin().Y(), bbCP.CornerMin().Z());
  c->add(new GOLine(p0, p1));
  ctd->setContour(c);
  ctd->setZMin(bbCP.CornerMin().Z());
  ctd->setZMax(bbCP.CornerMax().Z());
  curOP->setFinalDepth(ctd->zMin());
  ui->spDepth->setValue(curOP->finalDepth());
  tdModel->append(ctd);
  }


// evaluate target definition and prepare toolpath generation
void SubOPClampingPlug::processTargets() {
  Bnd_Box bbWP = Core().workData()->workPiece->BoundingBox();
  CCTargetDefinition* ctd = static_cast<CCTargetDefinition*>(tdModel->item(0));
  gp_Pnt p0 = bbWP.CornerMin();
  gp_Pnt p1 = bbWP.CornerMax();

  p0.SetZ(ctd->zMin());
  p1.SetZ(ctd->zMax());

  curOP->cutPart = Core().helper3D()->createBox(p0, p1);

  curOP->cutPart->SetColor(Quantity_NOC_CYAN);
  curOP->cutPart->SetTransparency(0.8);
  curOP->cShapes.push_back(curOP->cutPart);
  Core().view3D()->showShapes(curOP->cShapes);
  Core().view3D()->refresh();

#ifdef REDNOSE
  //  cp has corners   -16  /  -50  /   55    and    74  /  50  /  65
  //  cp has workpiece -61  /  -55  /  -95    and    79  /  55  /  55
  //
  //  cp has corners   -16  /  -38  /   55    and    68  /  33  /  65
  //  cp has workpiece -61  /  -55  /  -95    and    79  /  55  /  55
  qDebug() << "cp has workpiece" << bbWP.CornerMin().X() << " / " << bbWP.CornerMin().Y() << " / " << bbWP.CornerMin().Z()
           << "   and   "   << bbWP.CornerMax().X() << " / " << bbWP.CornerMax().Y() << " / " << bbWP.CornerMax().Z();

  double d0 = ctd->cornerMin().Y() - bbWP.CornerMin().Y();
  double d1 = bbWP.CornerMax().X() - ctd->cornerMax().X();
  double d2 = bbWP.CornerMax().Y() - ctd->cornerMax().Y();
  double d3 = ctd->cornerMin().X() - bbWP.CornerMin().X();
  int    s0 = 1 + d0 / curOP->cutWidth();
  int    s1 = 1 + d1 / curOP->cutWidth();
  int    s2 = 1 + d2 / curOP->cutWidth();
  int    s3 = 1 + d3 / curOP->cutWidth();

//             start with region 3 steps: 1  *  1  *  1  *  9
//
//             start with region 3 steps: 4  *  3  *  5  *  9
  int region = findMax(s0, s1, s2, s3);

  qDebug() << "start with region" << region << "steps:" << s0 << " * " << s1 << " * " << s2 << " * " << s3;
#endif
  }


// display generated toolpath (process list of worksteps)
void SubOPClampingPlug::showToolPath() {
  if (!curOP->workSteps().size()) return;
  Handle(AIS_Shape) as;

  for (auto ws : curOP->workSteps()) {
      ws->dump();
      switch (ws->type()) {
        case WTTraverse:
             curOP->toolPaths.push_back(Core().helper3D()->genFastMove(ws->startPos(), ws->endPos()));
             break;
        case WTStraightMove:
             curOP->toolPaths.push_back(Core().helper3D()->genWorkLine(ws->startPos(), ws->endPos()));
             break;
        case WTArc: {
             WSArc* wa = static_cast<WSArc*>(ws);

             curOP->toolPaths.push_back(Core().helper3D()->genWorkArc(ws->startPos(), ws->endPos(), wa->centerPos(), wa->isCCW()));
             } break;
        default: break;
        }
      }
  Core().view3D()->showShapes(curOP->toolPaths);
  Core().view3D()->refresh();
  }


// generated worksteps from target definition
void SubOPClampingPlug::toolPath() {
  if (!curOP->cutDepth()) return;
  processTargets();
  curOP->workSteps() = pathBuilder->genToolPath(curOP, curOP->cutPart);
  Core().view3D()->showShapes(curOP->toolPaths, false);
  if (curOP->showCutParts) Core().view3D()->showShapes(curOP->cShapes, false);
  Core().view3D()->refresh();
  showToolPath();
  }
