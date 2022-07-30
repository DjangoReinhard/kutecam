/* 
 * **************************************************************************
 * 
 *  file:       profitmillingbuilder.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    30.7.2022 by Django Reinhard
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
#include "profitmillingbuilder.h"
#include "pathbuilderutil.h"
#include "notchtargetdefinition.h"
#include "operation.h"
#include "core.h"
#include "kuteCAM.h"
#include "occtviewer.h"
#include "toolentry.h"
#include "util3d.h"
#include "wsarc.h"
#include "wsstraightmove.h"
#include "wstraverse.h"
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <ElCLib.hxx>
#include <GccAna_Lin2d2Tan.hxx>
#include <GccEnt.hxx>
#include <GccEnt_QualifiedCirc.hxx>
#include <GccEnt_QualifiedLin.hxx>
#include <GC_MakeLine.hxx>
#include <Geom_Line.hxx>
#include <Geom2d_Line.hxx>
#include <gp_Ax2d.hxx>
#include <TopoDS_Edge.hxx>
#include <QDebug>


ProfitMillingBuilder::ProfitMillingBuilder(PathBuilderUtil* pbu)
 : pbu(pbu) {
  }


TopoDS_Edge ProfitMillingBuilder::determineCenterLine(const NotchTargetDefinition* ntd, Handle(Geom_Line) gl0, Handle(Geom_Line) gl1, TopoDS_Shape cutPart) {
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
  TopoDS_Shape sCL = Core().helper3D()->intersect(eCL, cutPart);
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


std::vector<Workstep*> ProfitMillingBuilder::genToolPath(Operation* op, Handle(AIS_Shape) cutPart, std::vector<Handle(AIS_Shape)> cutPlanes) {
  NotchTargetDefinition* ntd = dynamic_cast<NotchTargetDefinition*>(op->targets.at(0));
  GC_MakeLine ml0(ntd->borderPoint(0), ntd->borderPoint(1));
  GC_MakeLine ml1(ntd->borderPoint(2), ntd->borderPoint(3));
  TopoDS_Edge eM = determineCenterLine(ntd, ml0.Value(), ml1.Value(), op->cutPart->Shape());
  double notchWidth = ml0.Value()->Lin().Distance(ntd->borderPoint(2));
  ToolEntry* curTool = op->toolEntry();
  double prm0, prm1, cPrm0, cPrm1;
  std::vector<Workstep*> rv;
  Handle(Geom_Curve) lC = BRep_Tool::Curve(eM, prm0, prm1);
  Handle(Geom_Line)  centerBase = Handle(Geom_Line)::DownCast(lC);
  Handle(Geom_Curve) geomLast;
  gp_Pnt ptLast(0, 0, 300);
  gp_Pnt startCenterLine = centerBase->Value(prm0);
  gp_Pnt endCenterLine   = centerBase->Value(prm1);
  gp_Dir normDir         = centerBase->Lin().Normal(ntd->borderPoint(2)).Direction();
  gp_Dir centerDir       = centerBase->Lin().Direction();

  double xMax   = startCenterLine.Distance(endCenterLine);
  double xStep  = op->cutWidth();
  double radius = 0.5 * (notchWidth - curTool->fluteDiameter()) - op->offset();
  double leadInOutRadius = (int)(radius * 0.3 + 0.9);

  if (radius < 0) {
     qDebug() << "OUPS!!! - tool diameter to big for this notch!!!";
     return rv;
     }
  xMax += radius;
  for (int n=0; n < cutPlanes.size(); ++n) {
      Bnd_Box bb = cutPlanes.at(n)->BoundingBox();

      startCenterLine.SetZ(bb.CornerMin().Z());
      endCenterLine.SetZ(bb.CornerMin().Z());
      GC_MakeLine mCL(startCenterLine, endCenterLine);
      Handle(Geom_Line) geoCenterLine = mCL.Value();

      for (double i = -radius; i < xMax; i += xStep) {
          gp_Pnt centerMain = geoCenterLine->Value(i);
          gp_Lin normLine(centerMain, normDir);
          TopoDS_Edge eNormEdge     = BRepBuilderAPI_MakeEdge(normLine, -radius, radius);
          Handle(Geom_Curve) geomNE = BRep_Tool::Curve(eNormEdge, prm0, prm1);
          gp_Pnt    ptMain0         = geomNE->Value(prm0);
          gp_Pnt    ptMain1         = geomNE->Value(prm1);
          gp_Pnt    leadOutCenter, leadInCenter = geomNE->Value(prm0 + leadInOutRadius);
          gp_Ax2    axisMain(centerMain, {0, 0, 1}, normDir);
          gp_Circ   circMain(axisMain, radius);
          gp_Circ   circLeadInTangent(gp_Ax2(leadInCenter, {0, 0, 1}, normDir)
                                    , leadInOutRadius);
          gp_Circ2d leadOutCircle, leadInCircle = gp_Circ2d(gp_Ax22d({leadInCenter.X(), leadInCenter.Y()}
                                                                   , {normDir.X(), normDir.Y()})
                                                          , leadInOutRadius);
          gp_Pnt2d  ptLO, ptLI, tangentLO, tangentLI = ElCLib::Value(0.5 * M_PI, leadInCircle);

          if (geomLast.IsNull()) {
             // Path-Segment #1 (initial lead in)
             rv.push_back(new WSTraverse({ptLast.X(), ptLast.Y(), ptLast.Z()}
                                       , {tangentLI.X(), tangentLI.Y(), ptLast.Z()}));
             rv.push_back(new WSTraverse({tangentLI.X(), tangentLI.Y(), ptLast.Z()}
                                       , {tangentLI.X(), tangentLI.Y(), centerMain.Z()}));
             rv.push_back(new WSArc({tangentLI.X(), tangentLI.Y(), centerMain.Z()}
                                  , ptMain0
                                  , leadInCenter
                                  , true));
             }
          else {
             leadOutCenter = geomLast->Value(prm1 - leadInOutRadius);
             leadOutCircle = gp_Circ2d(gp_Ax22d({leadOutCenter.X(), leadOutCenter.Y()}
                                               , {normDir.X(), normDir.Y()})
                                     , leadInOutRadius);
             // calculate tangentpoints for connection line
             GccAna_Lin2d2Tan solver(GccEnt::Enclosing(leadOutCircle)
                                   , GccEnt::Enclosing(leadInCircle)
                                   , kute::MinDelta);
             ptLO = ElCLib::Value(0,    leadOutCircle); // should match previous ptMain1/ptLast
             ptLI = ElCLib::Value(M_PI, leadInCircle);  // should match ptMain0

             if (solver.IsDone() && solver.NbSolutions()) {
                solver.Tangency1(1, prm0,  prm1,  tangentLO);
                solver.Tangency2(1, cPrm0, cPrm1, tangentLI);
                }
             else {
                qDebug() << "OUPS - no tangent line created!";
                }
             // Path-Segment #3 (lead out)
             rv.push_back(new WSArc({ptLO.X(), ptLO.Y(), centerMain.Z()}
                                  , {tangentLO.X(), tangentLO.Y(), centerMain.Z()}
                                  , leadOutCenter
                                  , true));

             // Path-Segment #4 (connect)
             rv.push_back(new WSStraightMove({tangentLO.X(), tangentLO.Y(), centerMain.Z()}
                                           , {tangentLI.X(), tangentLI.Y(), centerMain.Z()}));

             ptLast = gp_Pnt(tangentLI.X(), tangentLI.Y(), centerMain.Z());
             if (i > (xMax - xStep)) break;
             // Path-Segment #1 (lead in)
             rv.push_back(new WSArc({tangentLI.X(), tangentLI.Y(), centerMain.Z()}
                                  , ptMain0
                                  , leadInCenter
                                  , true));
             }
          // Path-Segment #2 (main)
          rv.push_back(new WSArc(ptMain0, ptMain1, centerMain, true));
          ptLast   = ptMain1;
          geomLast = geomNE;
          }
      geomLast.Nullify();
      rv.push_back(new WSTraverse(ptLast
                                , {ptLast.X(), ptLast.Y(), op->safeZ1()}));
      ptLast.SetZ(op->safeZ1());
      }
  return rv;
  }
