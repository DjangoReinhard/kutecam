/* 
 * **************************************************************************
 * 
 *  file:       subopnotch.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    15.6.2022 by Django Reinhard
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
#include "subopnotch.h"
#include "ui_opSub.h"
#include "ui_mainwindow.h"
#include "core.h"
#include "kuteCAM.h"
#include "notchtargetdefinition.h"
#include "occtviewer.h"
#include "operationlistmodel.h"
#include "pathbuilder.h"
#include "selectionhandler.h"
#include "targetdeflistmodel.h"
#include "toolentry.h"
#include "toollistmodel.h"
#include "util3d.h"
#include "work.h"
#include <BRep_Tool.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <GccAna_Circ2d2TanOn.hxx>
#include <GC_MakeLine.hxx>
#include <GC_MakePlane.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <Geom_Plane.hxx>
#include <Geom_Surface.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <gp_Lin.hxx>
#include <QAction>
#include <QDebug>


SubOPNotch::SubOPNotch(OperationListModel* olm, TargetDefListModel* tdModel, PathBuilder* pb, QWidget* parent)
 : OperationSubPage(olm, tdModel, pb, parent) {
  ui->lCycle->setVisible(false);
  ui->cbCycle->setVisible(false);
  ui->lRetract->setVisible(false);
  ui->spRetract->setVisible(false);
  ui->spDwell->setVisible(false);
  connect(Core().uiMainWin()->actionNotchNew, &QAction::triggered, this, &SubOPNotch::createOP);
  }


void SubOPNotch::createOP() {
  int mx = olm->rowCount();

  OperationSubPage::createOP(mx, QString(tr("Notch #%1")).arg(mx), NotchOperation);
  connectSignals();
  }


TopoDS_Edge SubOPNotch::determineCenterLine(const NotchTargetDefinition* ntd, Handle(Geom_Line) gl0, Handle(Geom_Line) gl1) {
  TopoDS_Edge rv;
  gp_Dir cd = gl0->Lin().Direction();
  double d0 = gl0->Lin().Distance(ntd->borderPoint(2));
  //TODO: add support for non-parallel notches?
//  double d1 = gl0->Lin().Distance(ntd->borderPoint(3));
  double prm0, prm1, prm2, prm3;
  gp_Pnt p0, p1, p2, pM0, pM1;

  if (ntd->borderPoint(0).Distance(ntd->borderPoint(1))       // border-line(0, 1) is bigger
    > ntd->borderPoint(2).Distance(ntd->borderPoint(3))) {
     gp_Lin      b1  = gl1->Lin();
     gp_Lin      n0  = gl1->Lin().Normal(ntd->borderPoint(0));
     gp_Lin      n1  = gl1->Lin().Normal(ntd->borderPoint(1));
     TopoDS_Edge eN0 = BRepBuilderAPI_MakeEdge(n0, 0, d0);
     TopoDS_Edge eN1 = BRepBuilderAPI_MakeEdge(n1, 0, d0);
     Handle(Geom_Curve) c0 = BRep_Tool::Curve(eN0, prm0, prm1);
     Handle(Geom_Curve) c1 = BRep_Tool::Curve(eN1, prm2, prm3);

     p0 = c0->Value(prm0);
     p2 = c0->Value(prm1);
     if (!kute::isEqual(p0, ntd->borderPoint(0))
      || b1.Distance(p2) > kute::MinDelta) {
        eN0 = BRepBuilderAPI_MakeEdge(n0, 0, -d0);
        eN1 = BRepBuilderAPI_MakeEdge(n1, 0, -d0);

        c0 = BRep_Tool::Curve(eN0, prm0, prm1);
        c1 = BRep_Tool::Curve(eN1, prm2, prm3);
        }
     p0 = c0->Value(prm0);
     p1 = pM0 = c0->Value(prm0 + (prm1 - prm0) / 2);
     p2 = c0->Value(prm1);

     qDebug() << "first normal(0):" << p0.X() << " / " << p0.Y() << " / " << p0.Z();
     qDebug() << "first normal(1):" << p1.X() << " / " << p1.Y() << " / " << p1.Z();
     qDebug() << "first normal(2):" << p2.X() << " / " << p2.Y() << " / " << p2.Z();

     p0 = c1->Value(prm2);
     p1 = pM1 = c1->Value(prm2 + (prm3 - prm2) / 2);
     p2 = c1->Value(prm3);

     qDebug() << "second normal(0):" << p0.X() << " / " << p0.Y() << " / " << p0.Z();
     qDebug() << "second normal(1):" << p1.X() << " / " << p1.Y() << " / " << p1.Z();
     qDebug() << "second normal(2):" << p2.X() << " / " << p2.Y() << " / " << p2.Z();
     }
  else {
     gp_Lin b0  = gl0->Lin();
     gp_Lin n2 = gl0->Lin().Normal(ntd->borderPoint(2));
     gp_Lin n3 = gl0->Lin().Normal(ntd->borderPoint(3));
     TopoDS_Edge eN2 = BRepBuilderAPI_MakeEdge(n2, 0, d0);
     TopoDS_Edge eN3 = BRepBuilderAPI_MakeEdge(n3, 0, d0);
     Handle(Geom_Curve) c0 = BRep_Tool::Curve(eN2, prm0, prm1);
     Handle(Geom_Curve) c1 = BRep_Tool::Curve(eN3, prm2, prm3);

     p0 = c0->Value(prm0);
     p2 = c0->Value(prm1);
     if (!kute::isEqual(p0, ntd->borderPoint(2))
      || b0.Distance(p2) > kute::MinDelta) {
        eN2 = BRepBuilderAPI_MakeEdge(n2, 0, -d0);
        eN3 = BRepBuilderAPI_MakeEdge(n3, 0, -d0);

        c0 = BRep_Tool::Curve(eN2, prm0, prm1);
        c1 = BRep_Tool::Curve(eN3, prm2, prm3);
        }
     p0 = c0->Value(prm0);
     p1 = pM0 = c0->Value(prm0 + (prm1 - prm0) / 2);
     p2 = c0->Value(prm1);

     qDebug() << "first normal(0):" << p0.X() << " / " << p0.Y() << " / " << p0.Z();
     qDebug() << "first normal(1):" << p1.X() << " / " << p1.Y() << " / " << p1.Z();
     qDebug() << "first normal(2):" << p2.X() << " / " << p2.Y() << " / " << p2.Z();

     p0 = c1->Value(prm2);
     p1 = pM1 = c1->Value(prm2 + (prm3 - prm2) / 2);
     p2 = c1->Value(prm3);

     qDebug() << "second normal(0):" << p0.X() << " / " << p0.Y() << " / " << p0.Z();
     qDebug() << "second normal(1):" << p1.X() << " / " << p1.Y() << " / " << p1.Z();
     qDebug() << "second normal(2):" << p2.X() << " / " << p2.Y() << " / " << p2.Z();
     }
  GC_MakeLine bCL(pM0, pM1);
  TopoDS_Edge eCL = BRepBuilderAPI_MakeEdge(bCL.Value()->Lin(), -500, 500);
  //======================================================================
  TopoDS_Shape sCL = Core().helper3D()->intersect(eCL, curOP->cutPart->Shape());
  TopAbs_ShapeEnum stCL = sCL.ShapeType();

  if (!stCL) {
     std::vector<TopoDS_Edge> edges = Core().helper3D()->allEdgesWithin(sCL);

     if (edges.size()) rv = edges.at(0);
     else {
        std::vector<gp_Pnt> cutPoints = Core().helper3D()->allVertexCoordinatesWithin(sCL);

        for (auto p : cutPoints) {
            qDebug() << "cutpoint:" << p.X() << " / " << p.Y() << " / " << p.Z();
            }
        qDebug() << "cutPoints done (" << cutPoints.size() << ")";
        if (cutPoints.size() > 1)
           rv = BRepBuilderAPI_MakeEdge(cutPoints.at(0), cutPoints.at(1));
        }
     }
  else if (stCL == TopAbs_EDGE) {
     rv = TopoDS::Edge(sCL);
     }
  else {
     qDebug() << "SupOPNotch: unsupported CUT result!!! bye!";
     return rv;
     }
  return rv;
  }


void SubOPNotch::processSelection() {
  std::vector<TopoDS_Shape> selection = Core().view3D()->selection();

  if (selection.size() > 1) {
     gp_Pnt p, p0, p1, p2, p3;
     gp_Pln bottomPlane;
     Bnd_Box bb;
     int borderFound = 0;

     for (int i=0; i < selection.size(); ++i) {
         TopoDS_Shape selectedShape = selection.at(i);

         if (selectedShape.ShapeType() == TopAbs_FACE) {
            Handle(Geom_Surface) selectedFace = BRep_Tool::Surface(TopoDS::Face(selectedShape));

            if (selectedFace->IsKind(STANDARD_TYPE(Geom_Plane))) {
               GeomAdaptor_Surface selectedSurface(selectedFace);
               gp_Pln              pln = selectedSurface.Plane();
               gp_Pnt              pos = Core().helper3D()->deburr(pln.Location());
               gp_Dir              dir = Core().helper3D()->deburr(pln.Axis().Direction());

               if (kute::isEqual(dir.Z(), 1)) {
                  bottomPlane = pln;
                  }
               }
            }
         else if (selectedShape.ShapeType() == TopAbs_EDGE) {
            double             param0, param1;
            Handle(Geom_Curve) c = BRep_Tool::Curve(TopoDS::Edge(selectedShape), param0, param1);
            gp_Pnt             p01 = c->Value(param0);
            gp_Pnt             p02 = c->Value(param1);

            if (p01.Z() > bottomPlane.Location().Z()
             && p02.Z() > bottomPlane.Location().Z()) {
               if (!borderFound++) {
                  p0 = p01;
                  p1 = p02;
                  }
               else {
                  p2 = p01;
                  p3 = p02;
                  }
               bb.Add(p01);
               bb.Add(p02);
               }
            }
        }
     p = bb.CornerMin();
     p.SetZ(bottomPlane.Location().Z()); // final depth of notch
     bb.Add(p);

     qDebug() << borderFound << "borders found";
     qDebug() << "size of notch:" << bb.CornerMin().X() << " / " << bb.CornerMin().Y() << " / " << bb.CornerMin().Z()
              << "   to   " << bb.CornerMax().X() << " / " << bb.CornerMax().Y() << " / " << bb.CornerMax().Z();

     tdModel->append(new NotchTargetDefinition(bottomPlane, p0, p1, p2, p3));
     }
  processTargets();
  }


void SubOPNotch::processTargets() {
  if (tdModel->rowCount()) {
     NotchTargetDefinition* ntd = dynamic_cast<NotchTargetDefinition*>(tdModel->item(0));
     bool outside = curOP->isOutside();
     gp_Pnt tmp = ntd->borderPoint(0); tmp.SetZ(tmp.Z() + 10);  // virtual point to ensure vertical plane
     GC_MakePlane mp0(ntd->borderPoint(0), ntd->borderPoint(1), tmp);
     tmp = ntd->borderPoint(2);  tmp.SetZ(tmp.Z() + 10);        // virtual point to ensure vertical plane
     GC_MakePlane mp1(ntd->borderPoint(2), ntd->borderPoint(3), tmp);
     BRepBuilderAPI_MakeFace mf0(mp0.Value()->Pln(), -500, 500, -500, 500);
     Handle(AIS_Shape) wc0 = Core().selectionHandler()->createCutPart(curOP->workPiece, mf0.Shape(), curOP, outside);
     Bnd_Box           bbC = wc0->BoundingBox();
     BRepClass3d_SolidClassifier chkP3(wc0->Shape(), ntd->borderPoint(3), kute::MinDelta);
     BRepClass3d_SolidClassifier chkP4(wc0->Shape(), ntd->borderPoint(3), kute::MinDelta);

     if (chkP3.State() == TopAbs_OUT || chkP4.State() == TopAbs_OUT) {
        outside = !outside;
        wc0 = Core().selectionHandler()->createCutPart(curOP->workPiece, mf0.Shape(), curOP, outside);
        bbC = wc0->BoundingBox();
        }
     qDebug() << "cut-part0 is from"
              << bbC.CornerMin().X() << "/" << bbC.CornerMin().Y() << "/" << bbC.CornerMin().Z()
              << "\tto\t"
              << bbC.CornerMax().X() << "/" << bbC.CornerMax().Y() << "/" << bbC.CornerMax().Z();

     BRepBuilderAPI_MakeFace mf1(mp1.Value()->Pln(), -500, 500, -500, 500);
     Handle(AIS_Shape) wc1 = Core().selectionHandler()->createCutPart(wc0, mf1.Shape(), curOP, !outside);

     bbC = wc1->BoundingBox();
     qDebug() << "cut-part1 is from"
              << bbC.CornerMin().X() << "/" << bbC.CornerMin().Y() << "/" << bbC.CornerMin().Z()
              << "\tto\t"
              << bbC.CornerMax().X() << "/" << bbC.CornerMax().Y() << "/" << bbC.CornerMax().Z();
     BRepBuilderAPI_MakeFace mf2(ntd->bottom(), -500, 500, -500, 500);
     curOP->cutPart = Core().selectionHandler()->createCutPart(wc1, mf2.Shape(), curOP, true);
     BRepClass3d_SolidClassifier chkP0(curOP->cutPart->Shape(), ntd->borderPoint(0), kute::MinDelta);
     BRepClass3d_SolidClassifier chkP1(curOP->cutPart->Shape(), ntd->borderPoint(3), kute::MinDelta);

     if (chkP0.State() == TopAbs_OUT || chkP1.State() == TopAbs_OUT)
        curOP->cutPart = Core().selectionHandler()->createCutPart(wc1, mf2.Shape(), curOP, false);
     curOP->setOutside(outside);
     bbC = curOP->cutPart->BoundingBox();

     qDebug() << "cut-part2 is from"
              << bbC.CornerMin().X() << "/" << bbC.CornerMin().Y() << "/" << bbC.CornerMin().Z()
              << "\tto\t"
              << bbC.CornerMax().X() << "/" << bbC.CornerMax().Y() << "/" << bbC.CornerMax().Z();

     curOP->cutPart->SetColor(Quantity_NOC_CYAN);
     curOP->cutPart->SetTransparency(0.7);
     curOP->cShapes.push_back(curOP->cutPart);
//     wc0->SetColor(Quantity_NOC_PURPLE);
//     wc0->SetTransparency(0.8);
//     curOP->cShapes.push_back(wc0);
//     wc1->SetColor(Quantity_NOC_YELLOW);
//     wc1->SetTransparency(0.6);
//     curOP->cShapes.push_back(wc1);
     Core().view3D()->showShapes(curOP->cShapes);
     }
  }


void SubOPNotch::genFinishingToolPath() {

  }


void SubOPNotch::genRoughingToolPath() {
  if (!tdModel->rowCount()) return;

  NotchTargetDefinition* ntd = dynamic_cast<NotchTargetDefinition*>(tdModel->item(0));
  std::vector<Handle(AIS_Shape)> cutPlanes = createCutPlanes(curOP);

  curOP->workSteps() = pPathBuilder->genNotchPath(curOP, curOP->cutPart, cutPlanes);

  showToolPath(curOP);

//  GC_MakeLine ml0(ntd->borderPoint(0), ntd->borderPoint(1));
//  GC_MakeLine ml1(ntd->borderPoint(2), ntd->borderPoint(3));

//  Bnd_Box     bbCP  = curOP->cutPart->BoundingBox();
//  int         iTool = Core().toolListModel()->findToolNum(curOP->toolNum());
//  ToolEntry*  te    = Core().toolListModel()->tool(iTool);

//  double d0 = ml0.Value()->Lin().Distance(ntd->borderPoint(2));
//  double d1 = ml0.Value()->Lin().Distance(ntd->borderPoint(3));
//  double prm0, prm1;
//  gp_Pnt mp0, mp1;

//  if (curOP->cutPart.IsNull()) return;
//  if (kute::isEqual(d0, d1)) {

//     }
//  else {
//     qDebug() << "non-parallel notches not supported yet";
//     }

//  TopoDS_Edge eCenter = determineCenterLine(ntd, ml0.Value(), ml1.Value());
//  Handle(Geom_Curve) geomCenterLine = BRep_Tool::Curve(eCenter, prm0, prm1);
//  Handle(Geom_Line)  centerLine = Handle(Geom_Line)::DownCast(geomCenterLine);
//  gp_Dir normDir   = centerLine->Lin().Normal(ntd->borderPoint(0)).Direction();
//  gp_Dir centerDir = centerLine->Lin().Direction();

//  mp0 = geomCenterLine->Value(prm0);
//  mp1 = geomCenterLine->Value(prm1);

//  qDebug() << "center-line extended to [" << prm0 << ", " << prm1 << "]:"
//           << mp0.X() << " / " << mp0.Y() << " / " << mp0.Z()
//           << "   ->   "
//           << mp1.X() << " / " << mp1.Y() << " / " << mp1.Z();

//  double cutWidth = fmin(d0, d1) - 2.0 * curOP->offset() + te->fluteDiameter();
//  double cutDepth = bbCP.CornerMax().Z() - (bbCP.CornerMin().Z() + curOP->offset());
//  double cutStep  = fmin(cutDepth, curOP->cutDepth());
//  double xOff     = te->fluteDiameter() * 0.8;
////=============================================================================
//  double advance = curOP->cutWidth();
//  double startZ = bbCP.CornerMax().Z();
//  double lastZ  = curOP->finalDepth() + curOP->offset();
//  double curZ   = startZ;

//  qDebug() << "hold on";
  }
