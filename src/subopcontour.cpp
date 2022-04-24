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
#include "operationlistmodel.h"
#include "occtviewer.h"
#include "contourtargetdefinition.h"
#include "targetdeflistmodel.h"
#include "toolentry.h"
#include "toollistmodel.h"
#include "core.h"
#include "util3d.h"
#include "work.h"
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


void SubOPContour::processSelection() {

  }


void SubOPContour::showToolPath() {
  }


void SubOPContour::toolPath() {
  qDebug() << "OP contour - gonna create toolpath ...";  
  if (curOP->toolPaths.size()) {
     Core().view3D()->removeShapes(curOP->toolPaths);
     curOP->toolPaths.clear();
     }
  if (curOP->cShapes.size()) {
     Core().view3D()->removeShapes(curOP->cShapes);
     curOP->cShapes.clear();
     }
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
  Handle(AIS_Shape)       curWP = Core().helper3D()->fixRotation(work->workPiece->Shape()
                                                               , curOP->operationA()
                                                               , curOP->operationB()
                                                               , curOP->operationC());

  if (!work->modCut.IsNull()) Core().view3D()->removeShape(work->modCut);
  if (!work->wpCut.IsNull())  Core().view3D()->removeShape(work->wpCut);
  work->wpCut = new AIS_Shape(Core().helper3D()->intersect(mf.Shape(), curWP->Shape()));
  work->wpCut->SetColor(Quantity_NOC_CYAN);

  work->modCut = new AIS_Shape(Core().helper3D()->intersect(mf.Shape(), model->Shape()));
  work->modCut->SetColor(Quantity_NOC_PURPLE);

  Core().view3D()->showShape(work->wpCut);
  Core().view3D()->showShape(work->modCut);
  Core().view3D()->refresh();
  }
