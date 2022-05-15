/* 
 * **************************************************************************
 * 
 *  file:       subopsweep.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    24.4.2022 by Django Reinhard
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
#include "subopsweep.h"
#include "gocontour.h"
#include "gopocket.h"
#include "ui_opSub.h"
#include "ui_mainwindow.h"
#include "operationlistmodel.h"
#include "occtviewer.h"
#include "core.h"
#include "cuttingparameters.h"
#include "kuteCAM.h"
#include "pathbuilder.h"
#include "selectionhandler.h"
#include "sweeptargetdefinition.h"
#include "targetdefinition.h"
#include "targetdeflistmodel.h"
#include "toollistmodel.h"
#include "toolentry.h"
#include "util3d.h"
#include "work.h"
#include "wsarc.h"
#include "wstraverse.h"
#include "wsstraightmove.h"

#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepAlgoAPI_Splitter.hxx>
#include <Geom_Plane.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_Surface.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <TopoDS_Iterator.hxx>

#include <QAction>
#include <QStringListModel>
#include <QDebug>


SubOPSweep::SubOPSweep(OperationListModel* olm, TargetDefListModel* tdModel, QWidget *parent)
 : OperationSubPage(olm, tdModel, parent) {
  ui->lCycle->setVisible(false);
  ui->cbCycle->setVisible(false);
  ui->lRetract->setVisible(false);
  ui->spRetract->setVisible(false);
  ui->spDwell->setVisible(false);
  connect(Core().uiMainWin()->actionSweepNew, &QAction::triggered, this, &SubOPSweep::createOP);
  }


std::vector<Handle(AIS_Shape)> SubOPSweep::createCutPlanes(Operation* op) {
  std::vector<Handle(AIS_Shape)> cutPlanes;

  if (op->cutPart.IsNull()) return cutPlanes;
  if (!op->cutDepth()) {
     qDebug() << "can't increment depth without cut-depth value!";
     return cutPlanes;
     }
  Bnd_Box bb = curOP->cutPart->BoundingBox();
  double startZ = bb.CornerMax().Z();
  double lastZ  = curOP->finalDepth() + curOP->offset();
  double curZ   = startZ;
  gp_Dir dir(0, 0, 1);
  gp_Pnt pos(bb.CornerMin().X(), bb.CornerMin().Y(), curZ);
  gp_Pln plane;

  qDebug() << "VM - final depth:" << curOP->finalDepth() << "\tlast cut depth:" << lastZ;

  curZ -= op->cutDepth();
  while (curZ > lastZ) {
        qDebug() << "cut depth is" << curZ;
        pos.SetZ(curZ);
        plane = gp_Pln(pos, dir);
        BRepBuilderAPI_MakeFace mf(plane, -500, 500, -500, 500);
        Handle(AIS_Shape) wc    = new AIS_Shape(Core().helper3D()->intersect(op->cutPart->Shape(), mf.Shape()));
        Bnd_Box           bbC   = wc->BoundingBox();

        qDebug() << "cut-plane is from"
                 << bbC.CornerMin().X() << "/" << bbC.CornerMin().Y() << "/" << bbC.CornerMin().Z()
                 << "\tto\t"
                 << bbC.CornerMax().X() << "/" << bbC.CornerMax().Y() << "/" << bbC.CornerMax().Z();
        wc->SetColor(Quantity_NOC_LIGHTGOLDENRODYELLOW);
        cutPlanes.push_back(wc);
        curOP->cShapes.push_back(wc);
        curZ -= op->cutDepth();
        }
  if (!kute::isEqual(curZ, lastZ)) {
     qDebug() << "last cut depth is" << lastZ;
     pos.SetZ(lastZ);
     plane = gp_Pln(pos, dir);
     BRepBuilderAPI_MakeFace mf(plane, -500, 500, -500, 500);
     Handle(AIS_Shape) wc = new AIS_Shape(Core().helper3D()->intersect(op->cutPart->Shape(), mf.Shape()));

     wc->SetColor(Quantity_NOC_LIGHTGOLDENRODYELLOW);
     cutPlanes.push_back(wc);
     curOP->cShapes.push_back(wc);
     }
  return cutPlanes;
  }


// prepare toolpath creation for sweepBigC...
void SubOPSweep::createHorizontalToolpaths(const std::vector<Handle(AIS_Shape)>& cutPlanes) {
  Work*  work = Core().workData();
  gp_Pnt from, to, lastTO;
  int    cntPaths = 0;

  qDebug() << "create toolpaths for cutplanes:";

  for (auto s : cutPlanes) {
      Bnd_Box bb = s->BoundingBox(); bb.SetGap(0);

      qDebug() << "workpiece is" << (work->roundWorkPiece ? "round" : "rectangled");
      qDebug() << "cut plane: "
               << bb.CornerMin().X() << "/" << bb.CornerMin().Y() << "/" << bb.CornerMin().Z()
               << "\tto\t"
               << bb.CornerMax().X() << "/" << bb.CornerMax().Y() << "/" << bb.CornerMax().Z();
      double deltaX = bb.CornerMax().X() - bb.CornerMin().X();
      double deltaY = bb.CornerMax().Y() - bb.CornerMin().Y();

      ++cntPaths;
      if (deltaX > deltaY) {
         // x toggle, y advance
         double startX = bb.CornerMin().X() - activeTool->fluteDiameter() * 0.6;
         double endX   = bb.CornerMax().X() + activeTool->fluteDiameter() * 0.6;

         if (deltaY < curOP->cutWidth()) {
            // perform trivial toolpath creation - just one pass
            double curY = bb.CornerMin().Y() + (bb.CornerMax().Y() - bb.CornerMin().Y()) / 2;

            if (cntPaths % 2) {
               from = gp_Pnt(startX, curY, bb.CornerMin().Z());
               to   = gp_Pnt(endX,   curY, bb.CornerMin().Z());
               }
            else {
               to   = gp_Pnt(startX, curY, bb.CornerMin().Z());
               from = gp_Pnt(endX,   curY, bb.CornerMin().Z());
               }
            if (kute::isEqual(endX,   lastTO.X())
             || kute::isEqual(startX, lastTO.X())) {
               curOP->workSteps().push_back(new WSTraverse(lastTO, from));
               }
            curOP->workSteps().push_back(new WSStraightMove(from, to));
            }
         else {
            if (ui->cbDir->currentIndex() == 1) to = sweepBigCounterClockwise(bb, lastTO);
            else                                to = sweepBigClockwise(bb, lastTO);
            }
         }
      else {
         // y toggle, x advance
         double startY = bb.CornerMin().Y() - activeTool->fluteDiameter() * 0.6;
         double endY   = bb.CornerMax().Y() + activeTool->fluteDiameter() * 0.6;

         if (deltaX < curOP->cutWidth()) {
           // perform trivial toolpath creation - just one pass
            double curX = bb.CornerMin().X() + (bb.CornerMax().X() - bb.CornerMin().X()) / 2;

            if (cntPaths % 2) {
               from = gp_Pnt(curX, startY, bb.CornerMin().Z());
               to   = gp_Pnt(curX, endY,   bb.CornerMin().Z());
               }
            else {
               to   = gp_Pnt(curX, startY, bb.CornerMin().Z());
               from = gp_Pnt(curX, endY,   bb.CornerMin().Z());
               }
            if (kute::isEqual(endY, lastTO.Y())
             || kute::isEqual(startY, lastTO.Y())) {
              curOP->workSteps().push_back(new WSTraverse(lastTO, from));
              }
            curOP->workSteps().push_back(new WSStraightMove(from, to));
            }
         else {
            if (ui->cbDir->currentIndex() == 1) to = sweepBigCounterClockwise(bb, lastTO);
            else                                to = sweepBigClockwise(bb, lastTO);
            }
         }
      lastTO = to;
      }
  }


void SubOPSweep::createOP() { // TODO: save possible changes to old operation ?
  int mx = olm->rowCount();

  OperationSubPage::createOP(mx, QString(tr("Sweep #%1")).arg(mx), SweepOperation);
  connectSignals();
  }


void SubOPSweep::processSelection() {
  if (curOP->cShapes.size()) {
     Core().view3D()->removeShapes(curOP->cShapes);
     curOP->cShapes.clear();
     }
  tdModel->clear();
  if (!Core().view3D()->selection().size()) return;
  SweepTargetDefinition* std   = nullptr;
  Handle(AIS_Shape)      baseFace = Core().helper3D()->fixRotation(Core().view3D()->baseFace()->Shape()
                                                                 , curOP->operationA()
                                                                 , curOP->operationB()
                                                                 , curOP->operationC());
  gp_Vec            baseNormal = Core().helper3D()->deburr(Core().helper3D()->normalOfFace(baseFace->Shape()));
  GOContour*        contour = nullptr;
  Handle(AIS_Shape) cutPart;
  Bnd_Box           bbCP;

  if (Core().view3D()->selection().size() > 1) {
     contour = Core().selectionHandler()->createContourFromSelection(curOP);
     if (!contour) return;
     contour->extendBy(50);
     TopoDS_Shape      cutWire = contour->toShape(-500)->Shape();
     Handle(AIS_Shape) aw      = new AIS_Shape(cutWire);
     gp_Vec            prismVec(0, 0, 1000);
     TopoDS_Shape      cuttingFace = BRepPrimAPI_MakePrism(cutWire, prismVec);
     Handle(AIS_Shape) aCF         = new AIS_Shape(cuttingFace);

     cutPart = Core().selectionHandler()->createCutPart(cuttingFace, curOP);
     bbCP    = cutPart->BoundingBox();
     aCF->SetColor(Quantity_NOC_CYAN);
     aw->SetColor(Quantity_NOC_ORANGE);
     aCF->SetTransparency(0.8);
     aw->SetWidth(3);

     if (!curOP->workPiece.IsNull()) {
        TopoDS_Shape master = BRepAlgoAPI_Common(contour->toWire(0), curOP->workPiece->Shape());
        Handle(AIS_Shape) asM = new AIS_Shape(master);

        asM->SetColor(Quantity_NOC_ORANGE);
        asM->SetWidth(3);

        Core().view3D()->showShape(asM);
        //TODO: replace contour!
        contour->setContour(master);
        qDebug() << "contour of selection: " << contour->toString();
        }
     std = new SweepTargetDefinition(contour);
     curOP->setVertical(true);
     curOP->cShapes.push_back(aw);
     curOP->cShapes.push_back(aCF);
     ui->spDepth->setValue(bbCP.CornerMin().Z());
     }
  else { // only one face selected
     TopoDS_Shape            selectedShape = Core().view3D()->selection().at(0);
     Handle(Geom_Surface)    selectedFace  = BRep_Tool::Surface(TopoDS::Face(selectedShape));
     Handle(AIS_Shape)       asTmp         = new AIS_Shape(selectedShape);
     Bnd_Box                 bbSel         = asTmp->BoundingBox(); bbSel.SetGap(0);
     GeomAdaptor_Surface     selectedSurface(selectedFace);
     gp_Pln                  pln = selectedSurface.Plane();
     BRepBuilderAPI_MakeFace mf(pln, -500, 500, -500, 500);

     // sweep handles plane faces only
     if (selectedFace->IsKind(STANDARD_TYPE(Geom_Plane))) {
        gp_Pnt pos = Core().helper3D()->deburr(pln.Location());
        gp_Dir dir = Core().helper3D()->deburr(pln.Axis().Direction());

        // vertical plane (limited by spDepth)
        if (kute::isEqual(dir.Z(), 0)) {
           cutPart = Core().selectionHandler()->createCutPart(mf.Shape(), curOP);
           bbCP    = cutPart->BoundingBox();
           contour = Core().selectionHandler()->createContourFromSelection(curOP);
           if (!contour) return;
           contour->extendBy(200);
           TopoDS_Shape cutWire = contour->toShape(-100)->Shape();

           std = new SweepTargetDefinition(contour);
           curOP->setVertical(true);
           tdModel->append(std);
           ui->spDepth->setValue(curOP->finalDepth());
           }
        // horizontal plane (possibly limited by base face)
        else if (kute::isEqual(dir.Z(), 1)) {
           TopoDS_Shape master = Core().selectionHandler()->createBaseContour(pos, dir, curOP);

           contour = new GOContour(Core().helper3D()->centerOf(bbSel));
           contour->setContour(master);
           curOP->setVertical(false);
           if (contour->size()) {
              TopoDS_Shape      cutWire = contour->toWire();
              Handle(AIS_Shape) aw      = new AIS_Shape(cutWire);
              gp_Vec            prismVec(baseNormal.X(), baseNormal.Y(), 0);
              TopoDS_Shape      cuttingFace = BRepPrimAPI_MakePrism(cutWire, prismVec);
              Handle(AIS_Shape) aCF         = new AIS_Shape(cuttingFace);

              cutPart = Core().selectionHandler()->createCutPart(mf.Shape(), curOP);
              bbCP    = cutPart->BoundingBox();
              aCF->SetColor(Quantity_NOC_CYAN);
              aw->SetColor(Quantity_NOC_ORANGE);
              aCF->SetTransparency(0.8);
              aw->SetWidth(3);

              if (!curOP->workPiece.IsNull()) {
                 TopoDS_Shape master = BRepAlgoAPI_Common(contour->toWire(), curOP->workPiece->Shape());
                 //TODO: replace contour!
                 contour->setContour(master);
                 }
              std = new SweepTargetDefinition(pos, dir);
              std->setContour(contour);
              std->setBaseIsBorder(true);
              tdModel->append(std);
              curOP->cShapes.push_back(aw);
              curOP->cShapes.push_back(aCF);
              ui->spDepth->setValue(bbCP.CornerMin().Z());
              }
           else {
              cutPart = Core().selectionHandler()->createCutPart(mf.Shape(), curOP);
              cutPart->SetColor(Quantity_NOC_CYAN);
              cutPart->SetTransparency(0.8);
              curOP->cShapes.push_back(cutPart);

              std = new SweepTargetDefinition(pos, dir);
              }
           ui->spDepth->setValue(pos.Z());
           }
        else qDebug() << "ERROR: selected shape is not horizontal nor vertical!";
        }
     else qDebug() << "ERROR: selected shape is not plane!";
     }  
  bbCP = cutPart->BoundingBox();
  cutPart->SetColor(Quantity_NOC_CYAN);
  cutPart->SetTransparency(0.8);
  std->setBaseDir(baseNormal);
  std->setBaseDelim(baseFace->BoundingBox());
  std->setZMin(bbCP.CornerMin().Z());
  std->setZMax(bbCP.CornerMax().Z());
  tdModel->append(std);
  curOP->cShapes.push_back(cutPart);
  Core().view3D()->showShapes(curOP->cShapes);
  Core().view3D()->refresh();
  }


// "3;50.0000/-77.7558/-17.3484;-50.0000/-77.7558/-17.3484;0.0000/-15.2976/15.7157;0|1;50.0000/-77.7558/-17.3484;50.0000/5.0000/-17.3484|1;50.0000/5.0000/-17.3484;25.0000/30.0000/-17.3484|1;25.0000/30.0000/-17.3484;-25.0000/30.0000/-17.3484|1;-25.0000/30.0000/-17.3484;-50.0000/5.0000/-17.3484|1;-50.0000/5.0000/-17.3484;-50.0000/-77.7558/-17.3484"
// "3;50.0000/-87.7558/90.1355;-50.0000/-87.7558/90.1355;0.0000/-15.2976/15.7157;0|1;50.0000/-87.7558/90.1355;50.0000/5.0000/90.1355|1;50.0000/5.0000/90.1355;25.0000/30.0000/90.1355|1;25.0000/30.0000/90.1355;-25.0000/30.0000/90.1355|1;-25.0000/30.0000/90.1355;-50.0000/5.0000/90.1355|1;-50.0000/5.0000/90.1355;-50.0000/-87.7558/90.1355"
//
void SubOPSweep::processTargets() {
  if (!tdModel->rowCount()) return;
  //sweep operations shall have one target definition only!
  SweepTargetDefinition* std = static_cast<SweepTargetDefinition*>(tdModel->item(0));

  if (!curOP->isVertical()) {
     gp_Pln                  cutPlane(std->pos(), std->dir());
     BRepBuilderAPI_MakeFace mf(cutPlane, -500, 500, -500, 500);

     curOP->cutPart = Core().selectionHandler()->createCutPart(mf.Shape(), curOP);
     }
  else {
     if (std->contour()) {
        // cutWire is border, cutPart the part to remove
        qDebug() << "reloaded cut contour:" << std->contour()->toString();
        TopoDS_Shape cutWire = std->contour()->toShape(-100)->Shape();
        Handle(AIS_Shape) aw = new AIS_Shape(cutWire);
        gp_Vec       prismVec(0, 0, 500);
        TopoDS_Shape cuttingFace = BRepPrimAPI_MakePrism(cutWire, prismVec);
        Handle(AIS_Shape) aCF         = new AIS_Shape(cuttingFace);

        aCF->SetColor(Quantity_NOC_CYAN);
        aCF->SetTransparency(0.8);
        aw->SetColor(Quantity_NOC_ORANGE);
        aw->SetWidth(3);
        curOP->cShapes.push_back(aw);
        curOP->cShapes.push_back(aCF);
        curOP->cutPart = Core().selectionHandler()->createCutPart(cuttingFace, curOP);
        }
     else {
        gp_Pln                  cutPlane(std->pos(), std->dir());
        BRepBuilderAPI_MakeFace mf(cutPlane, -500, 500, -500, 500);

        curOP->cutPart = Core().selectionHandler()->createCutPart(mf.Shape(), curOP);
        }
     }
  if (!curOP->cutPart.IsNull()) {
     curOP->cutPart->SetColor(Quantity_NOC_CYAN);
     curOP->cutPart->SetTransparency(0.8);
     curOP->cShapes.push_back(curOP->cutPart);
     Core().view3D()->showShapes(curOP->cShapes, false);
     }
  Core().view3D()->refresh();
  }


//void SubOPSweep::showToolPath() {
//  if (!curOP->workSteps().size()) return;
//  Handle(AIS_Shape) as;

//  for (auto ws : curOP->workSteps()) {
//      ws->dump();
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
//      }
//  Core().view3D()->showShapes(curOP->toolPaths);
//  Core().view3D()->refresh();
//  }


gp_Pnt SubOPSweep::sweepBigClockwise(const Bnd_Box& bb, const gp_Pnt& lastTO) {
  double xMin = bb.CornerMin().X() + curOP->cutWidth() - activeTool->fluteDiameter() / 2;
  double xMax = bb.CornerMax().X() - curOP->cutWidth() + activeTool->fluteDiameter() / 2;
  double yMin = bb.CornerMin().Y() + curOP->cutWidth() - activeTool->fluteDiameter() / 2;
  double yMax = bb.CornerMax().Y() - curOP->cutWidth() + activeTool->fluteDiameter() / 2;
  double curX0 = xMin;
  double curX1 = xMax;
  double curY0 = yMin;
  double curY1 = yMax;
  double curZ  = bb.CornerMin().Z();
  gp_Pnt startPos(xMax, bb.CornerMax().Y() + 0.6 * activeTool->fluteDiameter(), bb.CornerMin().Z());
  gp_Pnt from = startPos, to = startPos;
  TopoDS_Edge tp, fm;
  Handle(AIS_Shape) path;
  int               cycle = 0;

  if (lastTO.X() || lastTO.Y() || lastTO.Z()) {
     to = from = lastTO;
     to.SetZ(from.Z() + 5);
     curOP->workSteps().push_back(new WSTraverse(from, to));
     from = to;
     to.SetX(startPos.X());
     to.SetY(startPos.Y());
     curOP->workSteps().push_back(new WSTraverse(from, to));
     from = to;
     to   = startPos;
     curOP->workSteps().push_back(new WSTraverse(from, to));
     }
  do {
     from = to;
     to = gp_Pnt(curX1, curY0, curZ);
     curOP->workSteps().push_back(new WSStraightMove(from, to));
     curX1 -= curOP->cutWidth();

     from = to;
     to   = gp_Pnt(curX0, curY0, curZ);
     curOP->workSteps().push_back(new WSStraightMove(from, to));
     curY0 += curOP->cutWidth();

     from = to;
     to   = gp_Pnt(curX0, curY1, curZ);
     curOP->workSteps().push_back(new WSStraightMove(from, to));
     curX0 += curOP->cutWidth();

     if (curX1 < curX0) {
        qDebug() << "leave cutplane before closing rectangle - last y"
                 << curY1 << " - y before:" << curY0;
        from = to;
        to.SetY(cycle ? curY0 : startPos.Y());
        curOP->workSteps().push_back(new WSStraightMove(from, to));
        break;
        }
     from = to;
     to   = gp_Pnt(curX1, curY1, curZ);
     curOP->workSteps().push_back(new WSStraightMove(from, to));
     curY1 -= curOP->cutWidth();

     from = to;
     ++cycle;
     } while (curY1 > curY0);
  return to;
  }


gp_Pnt SubOPSweep::sweepBigCounterClockwise(const Bnd_Box& bb, const gp_Pnt& lastTO) {
  double xMin = bb.CornerMin().X() + curOP->cutWidth() - activeTool->fluteDiameter() / 2;
  double xMax = bb.CornerMax().X() - curOP->cutWidth() + activeTool->fluteDiameter() / 2;
  double yMin = bb.CornerMin().Y() + curOP->cutWidth() - activeTool->fluteDiameter() / 2;
  double yMax = bb.CornerMax().Y() - curOP->cutWidth() + activeTool->fluteDiameter() / 2;
  double curX0 = xMin;
  double curX1 = xMax;
  double curY0 = yMin;
  double curY1 = yMax;
  double curZ  = bb.CornerMin().Z();
  gp_Pnt startPos(bb.CornerMax().X() + 0.6 * activeTool->fluteDiameter(), yMax, bb.CornerMin().Z());
  gp_Pnt from = startPos, to = startPos;
  TopoDS_Edge tp, fm;
  Handle(AIS_Shape) path;
  int               cycle = 0;

  if (lastTO.X() || lastTO.Y() || lastTO.Z()) {
     to = from = lastTO;
     to.SetZ(from.Z() + 5);
     curOP->workSteps().push_back(new WSTraverse(from, to));
     from = to;
     to.SetX(startPos.X());
     to.SetY(startPos.Y());
     curOP->workSteps().push_back(new WSTraverse(from, to));
     from = to;
     to   = startPos;
     curOP->workSteps().push_back(new WSTraverse(from, to));
     }
  do {
     from = to;
     to = gp_Pnt(curX0, curY1, curZ);
     curOP->workSteps().push_back(new WSStraightMove(from, to));
     curY1 -= curOP->cutWidth();

     from = to;
     to   = gp_Pnt(curX0, curY0, curZ);
     curOP->workSteps().push_back(new WSStraightMove(from, to));
     curX0 += curOP->cutWidth();

     from = to;
     to   = gp_Pnt(curX1, curY0, curZ);
     curOP->workSteps().push_back(new WSStraightMove(from, to));
     curY0 += curOP->cutWidth();

     if (curY1 < curY0) {
        qDebug() << "leave cutplane before closing rectangle - last y"
                 << curY1 << " - y before:" << curY0;
        from = to;
        to.SetY(cycle ? curY0 : startPos.Y());
        curOP->workSteps().push_back(new WSStraightMove(from, to));
        break;
        }
     from = to;
     to   = gp_Pnt(curX1, curY1, curZ);
     curOP->workSteps().push_back(new WSStraightMove(from, to));
     curX1 -= curOP->cutWidth();

     from = to;
     ++cycle;
     } while (curX1 > curX0);
  return to;
  }


// parent already called fixit() and cleanup calls
void SubOPSweep::toolPath() {
  if (!curOP->cutDepth()) return;
  processTargets();
  if (curOP->isVertical()) {
     qDebug() << "OP sweep - gonna create VERTICAL toolpath ...";
     curOP->workSteps() = pathBuilder->genToolPath(curOP, curOP->cutPart, false);
     }
  else {
     if (Core().workData()->roundWorkPiece) {
        std::vector<Handle(AIS_Shape)> cutPlanes = createCutPlanes(curOP);
        bool againstFeed = ui->cbDir->currentIndex() == 1;

        qDebug() << "OP sweep - gonna create ROUND toolpath ...";

        curOP->workSteps() = pathBuilder->genRoundToolpaths(curOP, cutPlanes);
        }
     else {
        if (curOP->targets.size() && curOP->targets.at(0)) {
           SweepTargetDefinition* std = static_cast<SweepTargetDefinition*>(curOP->targets.at(0));

           if (std->contour()) {
              curOP->workSteps() = pathBuilder->genToolPath(curOP, curOP->cutPart, false);
              }
           }
        if (!curOP->workSteps().size()) {
           // uses already built cutPart!
           std::vector<Handle(AIS_Shape)> cutPlanes = createCutPlanes(curOP);

           qDebug() << "OP sweep - gonna create HORIZONTAL toolpath ...";

           createHorizontalToolpaths(cutPlanes);
           }
        }
     }
  Core().view3D()->showShapes(curOP->toolPaths, false);
  if (curOP->showCutParts) Core().view3D()->showShapes(curOP->cShapes, false);
  Core().view3D()->refresh();
  showToolPath();
  }
