/* 
 * **************************************************************************
 * 
 *  file:       subopcontour.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    18.4.2022 by Django Reinhard
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
#include "subopcontour.h"
#include "ui_opSub.h"
#include "ui_mainwindow.h"
#include "cuttingparameters.h"
#include "gocontour.h"
#include "operationlistmodel.h"
#include "occtviewer.h"
#include "contourtargetdefinition.h"
#include "pathbuilder.h"
#include "selectionhandler.h"
#include "targetdeflistmodel.h"
#include "toolentry.h"
#include "toollistmodel.h"
#include "core.h"
#include "util3d.h"
#include "work.h"
#include "workstep.h"
#include "wpcutter.h"
#include "wsarc.h"
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepAlgoAPI_Splitter.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <TopoDS_Iterator.hxx>
#include <QAction>
#include <QStringListModel>
#include <QDebug>


SubOPContour::SubOPContour(OperationListModel* olm, TargetDefListModel* tdModel, QWidget *parent)
 : OperationSubPage(olm, tdModel, parent) {
  ui->lCycle->setVisible(false);
  ui->cbCycle->setVisible(false);
  ui->lRetract->setVisible(false);
  ui->spRetract->setVisible(false);
  ui->spDwell->setVisible(false);
  connect(Core().uiMainWin()->actionContourNew, &QAction::triggered, this, &SubOPContour::createOP);
  }


void SubOPContour::createOP() {
  int mx = olm->rowCount();
  OperationSubPage::createOP(mx, QString(tr("Contour #%1")).arg(mx), ContourOperation);
  connectSignals();
  }


// when we create a contour operation, waterline depth is always 0
// so create a path here - instead create a path at toolpath generation request
void SubOPContour::processSelection() {
  // no support for selection

  // waterline depth will be handled by parent (OperationsPage),
  // so nothing to do here!
  }


void SubOPContour::showToolPath() {
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


// curOP->waterlineDepth() tells where to take the waterline.
// It says nothing about milling depth or the like
void SubOPContour::toolPath() {    
  qDebug() << "OP contour - gonna create toolpath ...";  
  if (!curOP->cutDepth()) return;
  if (Core().workData()->modCut.IsNull()) return;
  gp_Pnt     center = Core().helper3D()->centerOf(curOP->wpBounds);
  GOContour* contour = new GOContour(center);

  contour->setContour(Core().workData()->modCut->Shape());

  qDebug() << "water line contour:";
  qDebug() << contour->toString();
  if (!curOP->targets.size()) {
     ContourTargetDefinition* ctd = new ContourTargetDefinition(Core().helper3D()->centerOf(curOP->wpBounds));

     ctd->setContour(contour);
     ctd->setZMax(curOP->wpBounds.CornerMax().Z());
     ctd->setZMin(curOP->finalDepth());
     curOP->targets.push_back(ctd);
     }
  gp_Pln cutPlane({center.X(), center.Y(), curOP->finalDepth()}, {0, 0, 1});
  BRepBuilderAPI_MakeFace mf(cutPlane, -500, 500, -500, 500);
  curOP->cutPart = Core().selectionHandler()->createCutPart(mf.Shape(), curOP);

  curOP->workSteps() = pathBuilder->genToolPath(curOP, curOP->cutPart);

  // curOP->waterlineDepth()
//  WPCutter cutAlgo(curOP->workPiece);

//  std::vector<GOContour*> cutParts = cutAlgo.processShape(aw->Shape(), center);
//  for (auto cp : cutParts)
//      cp->extendBy(10);
//  clippedCurves.push_back(cutParts);
//  Core().view3D()->showShape(aw);

//  std::vector<GOPocket*>         pool     = splitCurves(clippedCurves);
//  std::vector<Handle(AIS_Shape)> toolPath = path4Pockets(pool);
//  Core().view3D()->showShapes(toolPath);

  Core().view3D()->showShapes(curOP->toolPaths, false);
  curOP->cutPart->SetColor(Quantity_NOC_CYAN);
  curOP->cutPart->SetTransparency(0.7);
  Core().view3D()->showShape(curOP->cutPart);
  if (curOP->showCutParts) Core().view3D()->showShapes(curOP->cShapes, false);
  Core().view3D()->refresh();
  showToolPath();
  }


void SubOPContour::updateCut(double d) {
  if (!curOP) return;
  gp_Pln                  p({0, 0, d}, {0, 0, 1});
  BRepBuilderAPI_MakeFace mf(p, -500, 500, -500, 500);
  Work*                   work  = Core().workData();
  Handle(AIS_Shape)       model = Core().helper3D()->fixRotation(work->model->Shape()
                                                               , curOP->operationA()
                                                               , curOP->operationB()
                                                               , curOP->operationC());
  if (!work->modCut.IsNull()) Core().view3D()->removeShape(work->modCut);
  work->modCut = new AIS_Shape(Core().helper3D()->intersect(mf.Shape(), model->Shape()));
  work->modCut->SetColor(Quantity_NOC_PURPLE);
  Core().view3D()->showShape(work->modCut);

  Core().view3D()->refresh();
  }
