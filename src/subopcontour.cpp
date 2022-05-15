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
#include "gocircle.h"
#include "gocontour.h"
#include "kuteCAM.h"
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
#include <BRepPrimAPI_MakePrism.hxx>
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


// waterline contours work without selection
void SubOPContour::processSelection() {
  // try to find contour to cut from selection
  std::vector<TopoDS_Shape> selection = Core().view3D()->selection();

  if (curOP->cShapes.size()) {
     Core().view3D()->removeShapes(curOP->cShapes);
     curOP->cShapes.clear();
     }
  if (!selection.size()) return; // process waterline contours at toolpath creation ...
  gp_Pnt            center(curOP->wpBounds.CornerMin().X() + (curOP->wpBounds.CornerMax().X() - curOP->wpBounds.CornerMin().X()) / 2
                         , curOP->wpBounds.CornerMin().Y() + (curOP->wpBounds.CornerMax().Y() - curOP->wpBounds.CornerMin().Y()) / 2
                         , curOP->wpBounds.CornerMin().Z() + (curOP->wpBounds.CornerMax().Z() - curOP->wpBounds.CornerMin().Z()) / 2);
  GOContour*        contour;
  Bnd_Box           bbCP;
  Handle(AIS_Shape) aCF;
  Handle(AIS_Shape) cutPart;
  TopoDS_Shape      cuttingFace;

  if (selection.size() > 1) {
     contour = Core().selectionHandler()->createContourFromSelection(curOP, &bbCP);
     qDebug() << "selected contour:" << contour->toString();
     contour->extendBy(50);

     TopoDS_Shape      cutWire = contour->toShape(-500)->Shape();
     Handle(AIS_Shape) aw      = new AIS_Shape(cutWire);
     gp_Vec            prismVec(0, 0, 1000);

     cuttingFace = BRepPrimAPI_MakePrism(cutWire, prismVec);
     cutPart = Core().selectionHandler()->createCutPart(cuttingFace, curOP);
     ui->spDepth->setValue(bbCP.CornerMin().Z());

     aw->SetColor(Quantity_NOC_ORANGE);
     aw->SetWidth(3);
     curOP->cShapes.push_back(aw);
     if (!tdModel->rowCount()) {
        ContourTargetDefinition* ctd = new ContourTargetDefinition(center, -1);

        ctd->setContour(contour);
        tdModel->append(ctd);
        }
     }
  else {
     TopoDS_Shape      s     = selection.at(0);
     Handle(AIS_Shape) asTmp = new AIS_Shape(s);
     Bnd_Box           bbSel = asTmp->BoundingBox(); bbSel.SetGap(0);

     if (s.ShapeType() == TopAbs_FACE) {     // we support cylindrical faces only
        Handle(Geom_Surface) selectedFace  = BRep_Tool::Surface(TopoDS::Face(s));

         if (!selectedFace->IsKind(STANDARD_TYPE(Geom_Plane))) {
            std::vector<TopoDS_Edge> edges = Core().helper3D()->allEdgesWithin(s);

            curOP->setLowerZ(bbSel.CornerMin().Z());
            curOP->setUpperZ(bbSel.CornerMax().Z());
            for (auto e : edges) {
                if (BRep_Tool::IsGeometric(e)) {
                   double first, last;
                   Handle(Geom_Curve) c = BRep_Tool::Curve(e, first, last);

                   if (c->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
                      Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(c);
                      gp_Pnt              pos    = circle->Position().Location();
                      const gp_Dir&       dir    = circle->Position().Direction();
                      double              radius = circle->Radius();

                      if (kute::isVertical(dir)) {         // OK, first horizontal circle will do
                         pos.SetZ(-500);
                         cuttingFace = BRepPrimAPI_MakeCylinder(gp_Ax2(pos, dir), radius, 1000);
                         cutPart = Core().selectionHandler()->createCutPart(cuttingFace, curOP);
                         bbCP    = cutPart->BoundingBox();

                         tdModel->append(new ContourTargetDefinition(pos, radius));
                         break; // don't care for rest of edges
                         }
                      }
                   }  // ignore all edges that are not circles
                }
            }
         }
     }
  aCF = new AIS_Shape(cuttingFace);
  aCF->SetColor(Quantity_NOC_CYAN);
  aCF->SetTransparency(0.8);
  cutPart->SetColor(Quantity_NOC_CYAN);
  cutPart->SetTransparency(0.8);
  curOP->cShapes.push_back(cutPart);
  curOP->cShapes.push_back(aCF);
  }


void SubOPContour::processTargets() {
  if (!tdModel->rowCount()) return;
  ContourTargetDefinition* ctd = dynamic_cast<ContourTargetDefinition*>(curOP->targets.at(0));

  if (!ctd) return;
  TopoDS_Shape      cuttingFace;

  if (kute::isEqual(ctd->radius(), 0)) {
     // may be stored waterline contour?!?
     TopoDS_Shape      cutWire = ctd->contour()->toShape(-500)->Shape();
     Handle(AIS_Shape) aw      = new AIS_Shape(cutWire);
     gp_Vec            prismVec(0, 0, 1000);

     cuttingFace = BRepPrimAPI_MakePrism(cutWire, prismVec);
     curOP->cutPart = Core().selectionHandler()->createCutPart(cuttingFace, curOP);
     }
  else if (ctd->radius() < 0) {
     // possibly contour from selected faces ...
     TopoDS_Shape      cutWire = ctd->contour()->toShape(-500)->Shape();
     Handle(AIS_Shape) aw      = new AIS_Shape(cutWire);
     gp_Vec            prismVec(0, 0, 1000);

     cuttingFace = BRepPrimAPI_MakePrism(cutWire, prismVec);
     curOP->cutPart = Core().selectionHandler()->createCutPart(cuttingFace, curOP);
     }
  else {
     // possibly cylindrical face selection
     gp_Pnt pos = ctd->pos();

     pos.SetZ(-500);
     cuttingFace = BRepPrimAPI_MakeCylinder(gp_Ax2(pos, {0, 0, 1}), ctd->radius(), 1000);
     curOP->cutPart = Core().selectionHandler()->createCutPart(cuttingFace, curOP);
     }
  if (!curOP->cutPart.IsNull()) {
     curOP->cutPart->SetColor(Quantity_NOC_CYAN);
     curOP->cutPart->SetTransparency(0.8);
     curOP->cShapes.push_back(curOP->cutPart);
     Core().view3D()->showShapes(curOP->cShapes, false);
     }
  Core().view3D()->refresh();
  }


// curOP->waterlineDepth() tells where to take the waterline.
// It says nothing about milling depth or the like
void SubOPContour::toolPath() {    
  qDebug() << "OP contour - gonna create toolpath ...";  
  if (!curOP->cutDepth()) return;         // user didn't choose valid tool settings
  processTargets();
  if (!Core().workData()->modCut.IsNull() && curOP->waterlineDepth()) {
     // use waterline to cut contour
     gp_Pnt     center = Core().helper3D()->centerOf(curOP->wpBounds);
     GOContour* contour = new GOContour(center);

     contour->setContour(Core().workData()->modCut->Shape());
     contour->simplify(curOP->waterlineDepth());
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
     curOP->workSteps() = pathBuilder->genToolPath(curOP, curOP->cutPart, true);
     }
  // try to cut selection based contour
  else if (curOP->targets.size()) {
     ContourTargetDefinition* ctd = dynamic_cast<ContourTargetDefinition*>(curOP->targets.at(0));

     if (!ctd || curOP->cutPart.IsNull()) return;

     if (kute::isEqual(ctd->radius(), 0)) {
        // may be stored waterline contour?!?
        curOP->workSteps() = pathBuilder->genToolPath(curOP, curOP->cutPart, true);
        }
     else if (ctd->radius() < 0) {
        // possibly contour from selected faces ...
        curOP->workSteps() = pathBuilder->genToolPath(curOP, curOP->cutPart, true);
        }
     else {
        // possibly cylindrical face selection
        qDebug() << "cut cylindrical face contour?!?";
        std::vector<TopoDS_Edge> edges = Core().helper3D()->allEdgesWithin(curOP->cutPart->Shape());

        // outside of circle we'd expect different circles and the outer has
        // radius bigger than radius from target definition
        for (auto &e : edges) {
            if (BRep_Tool::IsGeometric(e)) {
               double first, last;
               Handle(Geom_Curve) c = BRep_Tool::Curve(e, first, last);

               if (c->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
                  Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(c);
                  double              radius = circle->Radius();

                  if (radius > ctd->radius()) {
                     // cut outside of circle
                     qDebug() << "cut outside of circle, outer limit:" << radius;
                     }
                  else {
                     // cut circular pocket
                     qDebug() << "cut circular pocket ...";
                     }
                  }
               }
            }
        qDebug() << "what?";
        }
     }
  curOP->cutPart->SetColor(Quantity_NOC_CYAN);
  curOP->cutPart->SetTransparency(0.8);
  Core().view3D()->showShape(curOP->cutPart, false);
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


//void SubOPContour::showToolPath() {
//  if (!curOP->workSteps().size()) return;
//  Handle(AIS_Shape) as;
//  gp_Pnt lastPos = curOP->workSteps().at(0)->startPos();

//  for (int i=0; i < curOP->workSteps().size(); ++i) {
//      Workstep* ws = curOP->workSteps().at(i);

//      if (!kute::isEqual(ws->startPos(), lastPos)) {
//         qDebug() << "found wrong serial at #" << i;
//         ws->dump();
////         throw std::domain_error("invalid sequence!");
//         }
//      switch (ws->type()) {
//        case WTTraverse:
//             curOP->toolPaths.push_back(Core().helper3D()->genFastMove(ws->startPos(), ws->endPos()));
//             break;
//        case WTStraightMove:
//             curOP->toolPaths.push_back(Core().helper3D()->genWorkLine(ws->startPos(), ws->endPos()));
//             break;
//        case WTArc: {
//             WSArc* wa = static_cast<WSArc*>(ws);

//             curOP->toolPaths.push_back(Core().helper3D()->genWorkArc(ws->startPos(), ws->endPos(), wa->centerPos(), wa->isCCW()));
//             } break;
//        default: break;
//        }
//      lastPos = ws->endPos();
//      }
//  Core().view3D()->showShapes(curOP->toolPaths);
//  Core().view3D()->refresh();
//  }
