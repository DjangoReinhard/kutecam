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


SubOPClampingPlug::SubOPClampingPlug(OperationListModel* olm, TargetDefListModel* tdModel, PathBuilder* pb, QWidget* parent)
 : OperationSubPage(olm, tdModel, pb, parent) {
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
  if (curOP->showCutParts) {
     curOP->cutPart->SetColor(Quantity_NOC_CYAN);
     curOP->cutPart->SetTransparency(0.8);
     curOP->cShapes.push_back(curOP->cutPart);
     Core().view3D()->showShapes(curOP->cShapes, false);
     Core().view3D()->refresh();
     }
  }


void SubOPClampingPlug::genFinishingToolPath() {

  }


void SubOPClampingPlug::genRoughingToolPath() {
  if (!curOP->cutDepth()) return;
  processTargets();
  curOP->workSteps() = pathBuilder()->genToolPath(curOP, curOP->cutPart, false);
  Core().view3D()->showShapes(curOP->toolPaths, false);
  if (curOP->showCutParts) Core().view3D()->showShapes(curOP->cShapes, false);
  Core().view3D()->refresh();
  showToolPath(curOP);
  }
