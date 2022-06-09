/* 
 * **************************************************************************
 * 
 *  file:       selectionhandler.cpp
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
#include "selectionhandler.h"
#include "core.h"
#include "gocontour.h"
#include "occtviewer.h"
#include "operation.h"
#include "sweeptargetdefinition.h"
#include "toolentry.h"
#include "util3d.h"
#include "work.h"
#include "kuteCAM.h"

#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepAlgoAPI_Splitter.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeShell.hxx>
#include <BRepOffsetAPI_Sewing.hxx>
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


GOContour* SelectionHandler::createContourFromSelection(Operation* op, Bnd_Box* pBB) {
  std::vector<TopoDS_Shape> selection = Core().view3D()->selection();

  if (!selection.size()) return nullptr;
  gp_Pnt center(op->wpBounds.CornerMin().X() + (op->wpBounds.CornerMax().X() - op->wpBounds.CornerMin().X()) / 2
              , op->wpBounds.CornerMin().Y() + (op->wpBounds.CornerMax().Y() - op->wpBounds.CornerMin().Y()) / 2
              , op->wpBounds.CornerMin().Z() + (op->wpBounds.CornerMax().Z() - op->wpBounds.CornerMin().Z()) / 2);
  GOContour* contour = new GOContour(center);
  Bnd_Box    bbCut;

  if (selection.at(0).ShapeType() == TopAbs_FACE) {
     BRepOffsetAPI_Sewing ms;
     double minZ = -999;
     double maxZ =  999;

     ms.SetTolerance(kute::MinDelta);
     for (auto& s : selection) {
         Handle(AIS_Shape) asTmp = new AIS_Shape(s);
         Bnd_Box           bb    = asTmp->BoundingBox();

         if (bb.CornerMin().Z() > minZ) minZ = bb.CornerMin().Z();
         if (bb.CornerMax().Z() < maxZ) maxZ = bb.CornerMax().Z();
         bbCut.Add(bb);
         ms.Add(s);
         }
     if (pBB) *pBB = bbCut;
     ms.Perform();
     TopoDS_Shape                s = ms.SewedShape();
     gp_Pln                      cutPlane({0, 0, minZ + (maxZ - minZ) / 2}, {0, 0, 1});
     BRepBuilderAPI_MakeFace     mf(cutPlane, -500, 500, -500, 500);
     std::vector<GraphicObject*> goSegs;
     TopoDS_Shape                cs = Core().helper3D()->intersect(s, mf.Shape());
     std::vector<TopoDS_Edge>    ce = Core().helper3D()->allEdgesWithin(cs);

     qDebug() << "cut line contains " << ce.size() << "segments";

     for (auto& s : ce) {
         GraphicObject*    go = GOContour::occ2GO(s);

         if (kute::isEqual(go->startPoint(), go->endPoint())) {
            delete go;
            continue;
            }
         contour->add(go);
         Handle(AIS_Shape) as = new AIS_Shape(s);

         as->SetColor(Quantity_NOC_ORANGE);
         as->SetWidth(3);
         op->cShapes.push_back(as);
         }
//     qDebug() << "check it out?!?";
     }
  else if (selection.at(0).ShapeType() == TopAbs_EDGE) {
     qDebug() << "TODO: create a wire ...";
     for (auto& s : selection) {
         GraphicObject*    go = GOContour::occ2GO(TopoDS::Edge(s));

         if (kute::isEqual(go->startPoint(), go->endPoint())) {
            delete go;
            continue;
            }
         contour->add(go);
         Handle(AIS_Shape) as = new AIS_Shape(s);

         as->SetColor(Quantity_NOC_ORANGE);
         as->SetWidth(3);
         op->cShapes.push_back(as);
         }
     }
  else {
     throw std::domain_error("SelectionHandler - invalid/unknown shape type!");
     }
  return contour;
  }


// selected face identified by position and direction (either from selection, or from stored op)
TopoDS_Shape SelectionHandler::createBaseContour(const gp_Pnt& pos, const gp_Dir& dir, Operation* op) {
  Handle(AIS_Shape)       curBF = Core().helper3D()->fixRotation(Core().view3D()->baseFace()->Shape()
                                                               , op->operationA()
                                                               , op->operationB()
                                                               , op->operationC());
  gp_Vec                  vbf = Core().helper3D()->deburr(Core().helper3D()->normalOfFace(curBF->Shape()));
  gp_Pln                  selectedPlane(pos, dir);
  BRepBuilderAPI_MakeFace mfSelected(selectedPlane, -500, 500, -500, 500);

  return BRepAlgoAPI_Section(curBF->Shape(), mfSelected.Shape());
  }


Handle(AIS_Shape) SelectionHandler::createCutPart(TopoDS_Shape cf, Operation* op, bool wantFirst) {
  Handle(AIS_Shape) curWP = Core().helper3D()->fixRotation(Core().workData()->workPiece->Shape()
                                                         , op->operationA()
                                                         , op->operationB()
                                                         , op->operationC());
  Handle(AIS_Shape) curBF = Core().helper3D()->fixRotation(Core().view3D()->baseFace()->Shape()
                                                         , op->operationA()
                                                         , op->operationB()
                                                         , op->operationC());
  gp_Vec vb = Core().helper3D()->deburr(Core().helper3D()->normalOfFace(curBF->Shape()));

  qDebug() << "SH::createCutPart - direction of baseFace:" << vb.X() << " / " << vb.Y() << " / " << vb.Z();

  Handle(AIS_Shape)    rv;
  BRepAlgoAPI_Splitter splitter;
  TopTools_ListOfShape splitArgs;
  TopTools_ListOfShape splitTools;

  splitArgs.Append(curWP->Shape());
  splitTools.Append(cf);
  splitter.SetArguments(splitArgs);
  splitter.SetTools(splitTools);
  splitter.SetFuzzyValue(kute::MinDelta);
  splitter.Build();

  if (splitter.IsDone()) {
     splitter.SimplifyResult();
     TopoDS_Shape    result = splitter.Shape();
     TopoDS_Iterator it(result);
     TopoDS_Shape      s0  = it.Value(); it.Next();
     TopoDS_Shape      s1  = it.Value();
     Handle(AIS_Shape) as0 = new AIS_Shape(s0);
     Handle(AIS_Shape) as1 = new AIS_Shape(s1);
     bool debug = false;

     if (debug) {
        as0->SetColor(Quantity_NOC_BLACK);
        as1->SetColor(Quantity_NOC_WHITE);
        as0->SetTransparency(0.7);
        as1->SetTransparency(0.7);
        op->cShapes.push_back(as0);
        op->cShapes.push_back(as1);
        }
     if (wantFirst) rv = as0;
     else           rv = as1;
     }
  return rv;
  }


Handle(AIS_Shape) SelectionHandler::createCutPart(Operation* op, SweepTargetDefinition* std) {
  Handle(AIS_Shape) rv;

  if (!op) return rv;
  gp_Dir dir    = std->dir();
  gp_Pnt cutPos = std->pos();

  if (op->isVertical()) {
     if (kute::isVertical(dir)) dir = gp_Dir(1, 1, 0);
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

  splitArgs.Append(curWP->Shape());
  splitter.SetArguments(splitArgs);

  splitTools.Append(mf.Shape());
  splitter.SetTools(splitTools);

  splitter.SetNonDestructive(Standard_True);
  splitter.SetFuzzyValue(kute::MinDelta);
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
     int               checkDir = kute::getDominantAxis(dir);

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
            if (kute::isEqual(cutPos.Z(), bb0.CornerMin().Z())
            && (bb0.CornerMax().Z() > cutPos.Z())) {
               rv = as0;
               }
            else if (kute::isEqual(cutPos.Z(), bb1.CornerMin().Z())
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
















































