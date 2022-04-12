/* 
 * **************************************************************************
 * 
 *  file:       selectionhandler.cpp
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
#include "selectionhandler.h"
#include "core.h"
#include "occtviewer.h"
#include "operation.h"
#include "sweeptargetdefinition.h"
#include "toolentry.h"
#include "util3d.h"
#include "work.h"
#include "kuteCAM.h"

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Splitter.hxx>
#include <BRep_Tool.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Iterator.hxx>
#include <QDebug>


SelectionHandler::SelectionHandler() {
  }


cavc::Polyline<double> SelectionHandler::createContourFromSelection(Operation* op) {
  std::vector<TopoDS_Shape> selection = Core().view3D()->selection();
  cavc::Polyline<double>    res;

  if (!selection.size()) return res;
  Handle(TopTools_HSequenceOfShape) ss = new TopTools_HSequenceOfShape;
  Handle(TopTools_HSequenceOfShape) wires;
  ShapeAnalysis_FreeBounds          fb;
  double                            curZ = 0;

  for (auto& s : selection) {
      Handle(Geom_Surface) selectedFace = BRep_Tool::Surface(TopoDS::Face(s));
      Handle(AIS_Shape)    asTmp = new AIS_Shape(s);
      Bnd_Box              bb    = asTmp->BoundingBox();
      GeomAdaptor_Surface  surf(selectedFace);

      if (!curZ) curZ = (bb.CornerMax().Z() - bb.CornerMin().Z()) / 2 + bb.CornerMin().Z();
      gp_Pnt pos = Core().helper3D()->deburr(bb.CornerMin());
      if (abs(op->waterlineDepth()) < Core().helper3D()->MinDelta)
         curZ = op->waterlineDepth();
      pos.SetZ(curZ);
      gp_Pln                   cutPlane(pos, {0, 0, 1});
      BRepBuilderAPI_MakeFace  mf(cutPlane, -500, 500, -500, 500);
      TopoDS_Shape             cs = Core().helper3D()->intersect(s, mf.Shape());
      std::vector<TopoDS_Edge> ce = Core().helper3D()->allEdgesWithin(cs);

      ss->Append(ce.at(0));
      }
  fb.ConnectEdgesToWires(ss, Core().helper3D()->MinDelta, false, wires);
  std::vector<TopoDS_Edge> edges    = Core().helper3D()->allEdgesWithin(wires->First());

  res = Core().helper3D()->toPolyline(edges);

  return res;
  }


Handle(AIS_Shape) SelectionHandler::createCutPart(Operation* op) {
  std::vector<TopoDS_Shape> selection = Core().view3D()->selection();
  TopTools_ListOfShape      joinArgs;
  Handle(AIS_Shape)         res;

  if (!selection.size()) return res;
  double curZ = 0, deltaZ;

  for (auto& s : selection) {
      Handle(Geom_Surface)   selectedFace = BRep_Tool::Surface(TopoDS::Face(s));
      GeomAdaptor_Surface    surf(selectedFace);
      Handle(AIS_Shape)      asTmp    = new AIS_Shape(s);
      Bnd_Box                bb       = asTmp->BoundingBox(); bb.SetGap(0);
      SweepTargetDefinition* std      = nullptr;
      int                    reverted = s.Orientation();

      //TODO: may be use value from spOff
      if (!curZ) curZ = (bb.CornerMax().Z() - bb.CornerMin().Z()) / 2 + bb.CornerMin().Z();
      deltaZ = Core().helper3D()->deburr(bb.CornerMax().Z() - bb.CornerMin().Z());

      qDebug() << "cut-face - curZ:" << curZ << " - deltaZ:" << deltaZ;

      if (selectedFace->IsKind(STANDARD_TYPE(Geom_Plane))) {
         gp_Pln                  pln = surf.Plane();
         gp_Pnt                  pos = Core().helper3D()->deburr(pln.Location());
         gp_Dir                  dir = Core().helper3D()->deburr(pln.Axis().Direction());

         if (reverted) dir.Reverse();
         gp_Pln                  cutPlane(pos, dir);
         BRepBuilderAPI_MakeFace mf(cutPlane, -500, 500, -500, 500);

         qDebug() << "selected plane - pos:" << pos.X() << " / " << pos.Y() << " / " << pos.Z()
                  <<                "\tdir:" << dir.X() << " / " << dir.Y() << " / " << dir.Z();

         if (abs(dir.Z()) < Core().helper3D()->MinDelta) { // vertical plane
            op->setVertical(true);
            std = new SweepTargetDefinition(pos, dir);
            std->setZMin(bb.CornerMin().Z());
            std->setZMax(bb.CornerMax().Z());
            joinArgs.Append(createCutPart(op, std)->Shape());
            }
         else if ((1 - abs(dir.Z())) < Core().helper3D()->MinDelta) { // horizontal plane
            op->setVertical(false);
            std = new SweepTargetDefinition(pos, dir);
            std->setZMin(bb.CornerMin().Z());
            std->setZMax(bb.CornerMax().Z());
            joinArgs.Append(createCutPart(op, std)->Shape());
            }
         }
      else if (selectedFace->IsKind(STANDARD_TYPE(Geom_CylindricalSurface))) {
         gp_Cylinder cs  = surf.Cylinder();
         gp_Pnt      pos = Core().helper3D()->deburr(cs.Location());
         gp_Dir      dir = Core().helper3D()->deburr(cs.Axis().Direction());

         qDebug() << "selected cylinder - pos:" << pos.X() << " / " << pos.Y() << " / " << pos.Z()
                  <<                   "\tdir:" << dir.X() << " / " << dir.Y() << " / " << dir.Z();

         if (Core().helper3D()->isVertical(dir)) {
            op->setVertical(true);
            std = new SweepTargetDefinition(pos, dir, cs.Radius());
            std->setZMin(bb.CornerMin().Z());
            std->setZMax(bb.CornerMax().Z());
            joinArgs.Append(createCutPart(op, std)->Shape());
            }
         }
      }
  if (joinArgs.Size() > 1) {
     TopoDS_Shape s0 = joinArgs.First();
     TopoDS_Shape s1;

     qDebug() << "joinArgs.size: " << joinArgs.Size();

     joinArgs.RemoveFirst();
     s1 = joinArgs.First();
     BRepAlgoAPI_Fuse  join(s0, s1);
     Handle(AIS_Shape) cutWP;

     join.SetTools(joinArgs);
     join.SetFuzzyValue(1.e-5);
     join.Build();

     if (!join.IsDone()) {
        qDebug() << "join failed: ";
        join.DumpErrors(std::cerr);
        }
     if (join.HasWarnings()) {
        qDebug() << "Warnings : ";
        join.DumpErrors(std::cerr);
        }
     if (join.IsDone()) {
        join.SimplifyResult();

        res = new AIS_Shape(join.Shape());
        }
     }
  else {
     res = new AIS_Shape(joinArgs.First());
     }
  res->SetColor(Quantity_NOC_CYAN);
  res->SetTransparency(0.8);

  return res;
  }


Handle(AIS_Shape) SelectionHandler::createCutPart(Operation* op, SweepTargetDefinition* std) {
  Handle(AIS_Shape) rv;

  if (!op) return rv;
  gp_Dir dir    = std->dir();
  gp_Pnt cutPos = std->pos();

  if (op->isVertical()) {
     if (Core().helper3D()->isVertical(dir)) dir = gp_Dir(1, 1, 0);
     cutPos.SetZ(std->zMin() + (std->zMax() - std->zMin()) / 2);
     }
  Work*                   work  = Core().workData();
  Handle(AIS_Shape)       curWP = Core().helper3D()->fixRotation(work->workPiece->Shape()
                                                               , op->operationA()
                                                               , op->operationB()
                                                               , op->operationC());

  gp_Pln                  pln(cutPos, dir);
  BRepBuilderAPI_MakeFace mf(pln, -500, 500, -500, 500);
  BRepAlgoAPI_Splitter    splitter;
  TopTools_ListOfShape    splitArgs;
  TopTools_ListOfShape    splitTools;

  qDebug() << "------------------------ determine cut part -----------------------------";
  qDebug() << "cut-point:" << cutPos.X() << "/"
                           << cutPos.Y() << "/"
                           << cutPos.Z();
  qDebug() << "plane direction:" << dir.X() << "/"
                                 << dir.Y() << "/"
                                 << dir.Z();
  qDebug() << "-------------------------------------------------------------------------";
  splitArgs.Append(curWP->Shape());
  splitter.SetArguments(splitArgs);

  splitTools.Append(mf.Shape());
  splitter.SetTools(splitTools);

  splitter.SetNonDestructive(Standard_True);
  splitter.SetCheckInverted(Standard_False);
  splitter.SetUseOBB(Standard_True);
  splitter.SetFuzzyValue(1.e-5);
  splitter.Build();

  if (splitter.IsDone()) {
     splitter.SimplifyResult();
     TopoDS_Shape    result = splitter.Shape();
     TopoDS_Iterator it(result);
     // first shape contains model, second shape is rest of workpiece
     TopoDS_Shape      s0  = it.Value(); it.Next();
     TopoDS_Shape      s1  = it.Value();
     Handle(AIS_Shape) as0 = new AIS_Shape(s0);
     Handle(AIS_Shape) as1 = new AIS_Shape(s1);
     Bnd_Box           bb0 = as0->BoundingBox();
     Bnd_Box           bb1 = as1->BoundingBox();

     qDebug() << "bb of first shape:" << bb0.CornerMin().X() << "/"
                                      << bb0.CornerMin().Y() << "/"
                                      << bb0.CornerMin().Z()
              << " ==> "              << bb0.CornerMax().X() << "/"
                                      << bb0.CornerMax().Y() << "/"
                                      << bb0.CornerMax().Z();
     qDebug() << "bb of second shape:" << bb1.CornerMin().X() << "/"
                                       << bb1.CornerMin().Y() << "/"
                                       << bb1.CornerMin().Z()
              << " ==> "               << bb1.CornerMax().X() << "/"
                                       << bb1.CornerMax().Y() << "/"
                                       << bb1.CornerMax().Z();
     int checkDir = getDominantAxis(dir);

     switch (checkDir) {
       case 1:  // x is dominant
            if (dir.X() < 0) {
               if (bb0.CornerMin().X() < cutPos.X()) rv = as0;
               else                                  rv = as1;
               }
            else {
               if (bb1.CornerMax().X() > cutPos.X()) rv = as1;
               else                                  rv = as0;
               }
            break;
       case 2:  // y is dominant
            if (dir.Y() < 0) {
               if (bb0.CornerMin().Y() < cutPos.Y()) rv = as0;
               else                                  rv = as1;
               }
            else {
               if (bb1.CornerMax().Y() > cutPos.Y()) rv = as1;
               else                                  rv = as0;
               }
            break;
       case 3:  // z is dominant
            if (Core().helper3D()->isEqual(cutPos.Z(), bb0.CornerMin().Z())
            && (bb0.CornerMax().Z() > cutPos.Z())) {
               rv = as0;
               }
            else if (Core().helper3D()->isEqual(cutPos.Z(), bb1.CornerMin().Z())
                 && (bb0.CornerMax().Z() > cutPos.Z())) {
               rv = as1;
               }
            break;
       default:
            throw QString("invalid direction check result %1/%2/%3").arg(dir.X())
                                                                    .arg(dir.Y())
                                                                    .arg(dir.Z());
       }
     if (!rv.IsNull()) {
        rv->SetColor(Quantity_NOC_CYAN);
        rv->SetTransparency(0.8);
        op->cShapes.push_back(rv);
        }
     }
  return rv;
  }
















































