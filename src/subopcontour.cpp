/* 
 * **************************************************************************
 * 
 *  file:       subopcontour.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    11.4.2022 by Django Reinhard
 *  copyright:  2022 - 2022 Django Reinhard -  all rights reserved
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


// ui->cbMaterial->setCurrentText(pf->value("workpiece-material").toString());
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
  Work*                   work  = Core().workData();
  gp_Pln                  p({0, 0, curOP->waterlineDepth()}, {0, 0, 1});
  BRepBuilderAPI_MakeFace mf(p, -500, 500, -500, 500);
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
  work->modCut = new AIS_Shape(Core().helper3D()->intersect(mf.Shape(), model->Shape()));
  work->wpCut  = new AIS_Shape(Core().helper3D()->intersect(mf.Shape(), curWP->Shape()));
  Handle(TopTools_HSequenceOfShape) ms       = new TopTools_HSequenceOfShape;
  TopoDS_Shape                      contour  = Core().helper3D()->allEdgesWithin(Core().workData()->modCut->Shape(), ms);
  std::vector<TopoDS_Edge>          cEdges   = Core().helper3D()->allEdgesWithin(contour);
  Handle(AIS_Shape)                 cs       = new AIS_Shape(contour);
  Handle(AIS_Shape)                 os;
  TopoDS_Wire                       cw       = TopoDS::Wire(contour);
  Bnd_Box                           bbWP     = curWP->BoundingBox(); bbWP.SetGap(0);
  TopoDS_Shape                      cutShape = work->wpCut->Shape();
  qDebug() << "create polyline from cutshape ...";
  cavc::Polyline<double>            plContour= Core().helper3D()->toPolyline(cEdges);
  qDebug() << "\tcutshape polyline done ...";
  TopoDS_Edge                       edge;

  if (!cutShape.ShapeType()) {
     TopoDS_Iterator it(cutShape);
     // first shape contains model, second shape is rest of workpiece
     TopoDS_Shape tmp = it.Value();

     edge = TopoDS::Edge(tmp);
     }
  gp_Pnt p0, p1, p2;
  double radius;

  bbWP.Enlarge(activeTool->fluteDiameter() * 0.6);

  if (BRep_Tool::IsGeometric(edge)) {
     double first, last;
     Handle(Geom_Curve) c = BRep_Tool::Curve(edge, first, last);

     if (c->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
        Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(c);

        p0     = circle->Position().Location();
        radius = circle->Radius();
        }
     else {
        qDebug() << "not a cylinder";
        p0 = gp_Pnt(bbWP.CornerMin().X(), bbWP.CornerMin().Y(), 0);
        p1 = gp_Pnt(bbWP.CornerMax().X(), bbWP.CornerMin().Y(), 0);
        p2 = gp_Pnt(bbWP.CornerMax().X(), bbWP.CornerMax().Y(), 0);
        }
     }
  else {
     qDebug() << "not geometric!";
     }
  cs->SetColor(Quantity_NOC_YELLOW);
  cs->SetWidth(2);
  curOP->cShapes.push_back(cs);
  // display contour from polyline
  TopoDS_Shape             shPoly = Core().helper3D()->pl2Wire(plContour, 30);
  std::vector<TopoDS_Edge> plEdges = Core().helper3D()->allEdgesWithin(shPoly);
  std::vector<Handle(AIS_Shape)> plShapes;

  for (auto e : plEdges) {
      Handle(AIS_Shape) ae = new AIS_Shape(e);

      ae->SetColor(Quantity_NOC_DARKGREEN);
      ae->SetWidth(2);
      plShapes.push_back(ae);
      }
  Core().view3D()->showShapes(plShapes);

  BRepOffsetAPI_MakeOffset moAlgo(cw);
  double                   firstOffset = curOP->offset() + activeTool->fluteDiameter() / 2;
  double                   commonZ     = (bbWP.CornerMax().Z() - bbWP.CornerMin().Z()) / 2;

  //TODO: calculate number of offset-curves to create
  for (int i=0; i < 4; ++i) {
      moAlgo.Perform(firstOffset + curOP->cutWidth() * i);
      if (moAlgo.IsDone()) {
         Handle(TopTools_HSequenceOfShape) edgePool = new TopTools_HSequenceOfShape;
         TopoDS_Shape                      offWire  = Core().helper3D()->allEdgesWithin(moAlgo.Shape(), edgePool);
         std::vector<TopoDS_Edge>          edges    = Core().helper3D()->allEdgesWithin(offWire);
         qDebug() << "create offset polyline ...";
         cavc::Polyline<double>            offPoly  = Core().helper3D()->toPolyline(edges);
         TopoDS_Shape                      woffPoly = Core().helper3D()->pl2Wire(offPoly, 12);
         Handle(AIS_Shape)                 aWop     = new AIS_Shape(woffPoly);

         qDebug() << "\toffset polyline done ...";
         aWop->SetColor(Quantity_NOC_RED);
         aWop->SetWidth(2);
         curOP->toolPaths.append(aWop);
         qDebug() << "dump offset polyline ...";
         Core().helper3D()->dumpPolyline(offPoly);
         qDebug() << "\toffset polyline finished";
         }
      }
  Core().view3D()->showShapes(curOP->toolPaths, false);
  if (curOP->showCutParts) Core().view3D()->showShapes(curOP->cShapes, false);
  Core().view3D()->refresh();
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
