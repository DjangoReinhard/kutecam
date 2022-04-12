/* 
 * **************************************************************************
 * 
 *  file:       subopsweep.cpp
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
#include "subopsweep.h"
#include "ui_opSub.h"
#include "ui_mainwindow.h"
#include "operationlistmodel.h"
#include "occtviewer.h"
#include "core.h"
#include "cuttingparameters.h"
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
  TopTools_ListOfShape           joinArgs;
  Bnd_Box                        bb;

  if (curOP->isVertical()) {
     for (int i=0; i < tdModel->rowCount(); ++i) {
         SweepTargetDefinition* std = static_cast<SweepTargetDefinition*>(tdModel->item(i));

         joinArgs.Append(Core().selectionHandler()->createCutPart(op, std)->Shape());
         }
     if (curOP->cShapes.size()) curOP->cShapes.clear();
     TopoDS_Shape s0 = joinArgs.First();
     TopoDS_Shape s1;

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
        cutWP = new AIS_Shape(join.Shape());
        cutWP->SetColor(Quantity_NOC_CYAN);
        cutWP->SetTransparency(0.8);
        curOP->cShapes.push_back(cutWP);
        }
     bb = cutWP->BoundingBox();
     curOP->wpBounds = bb;

     qDebug() << "cut-part is from"
              << bb.CornerMin().X() << "/" << bb.CornerMin().Y() << "/" << bb.CornerMin().Z()
              << "\tto\t"
              << bb.CornerMax().X() << "/" << bb.CornerMax().Y() << "/" << bb.CornerMax().Z();

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
           Handle(AIS_Shape) wc = new AIS_Shape(Core().helper3D()->intersect(cutWP->Shape(), mf.Shape()));

           wc->SetColor(Quantity_NOC_LIGHTGOLDENRODYELLOW);
           cutPlanes.push_back(wc);
           curOP->cShapes.push_back(wc);
           curZ -= op->cutDepth();
           }
     if (abs(curZ - lastZ) > Core().helper3D()->MinDelta) {
        qDebug() << "last cut depth is" << lastZ;
        pos.SetZ(lastZ);
        plane = gp_Pln(pos, dir);
        BRepBuilderAPI_MakeFace mf(plane, -500, 500, -500, 500);
        Handle(AIS_Shape) wc = new AIS_Shape(Core().helper3D()->intersect(cutWP->Shape(), mf.Shape()));

        wc->SetColor(Quantity_NOC_LIGHTGOLDENRODYELLOW);
        cutPlanes.push_back(wc);
        curOP->cShapes.push_back(wc);
        }
     }
  else {
     for (int i=0; i < tdModel->rowCount(); ++i) {
         SweepTargetDefinition* std = static_cast<SweepTargetDefinition*>(tdModel->item(i));
         Handle(AIS_Shape)      cutWP = Core().selectionHandler()->createCutPart(op, std);

         bb = cutWP->BoundingBox();

         qDebug() << "cut-part is from"
                  << bb.CornerMin().X() << "/" << bb.CornerMin().Y() << "/" << bb.CornerMin().Z()
                  << "\tto\t"
                  << bb.CornerMax().X() << "/" << bb.CornerMax().Y() << "/" << bb.CornerMax().Z();

         // cut-part is from -55 / -41.6952 / 84.8569 	to	 55 / 34.0295 / 109.194
         double startZ = bb.CornerMax().Z();
         double lastZ  = bb.CornerMin().Z() + curOP->offset();
         double curZ   = startZ;
         gp_Pnt pos    = std->pos();
         gp_Pln plane;

         qDebug() << "HM - final depth:" << bb.CornerMin().Z() << "\tlast cut depth:" << lastZ;

         curZ -= op->cutDepth();
         while (curZ > lastZ) {
               qDebug() << "cut depth is" << curZ;
               pos.SetZ(curZ);
               plane = gp_Pln(pos, std->dir());
               BRepBuilderAPI_MakeFace mf(plane, -500, 500, -500, 500);
               Handle(AIS_Shape) wc = new AIS_Shape(Core().helper3D()->intersect(cutWP->Shape(), mf.Shape()));

               wc->SetColor(Quantity_NOC_LIGHTGOLDENRODYELLOW);
               cutPlanes.push_back(wc);
               curOP->cShapes.push_back(wc);
               curZ -= op->cutDepth();
               }
         if (abs(curZ - lastZ) > Core().helper3D()->MinDelta) {
            qDebug() << "last cut depth is" << lastZ;
            pos.SetZ(lastZ);
            plane = gp_Pln(pos, std->dir());
            BRepBuilderAPI_MakeFace mf(plane, -500, 500, -500, 500);
            Handle(AIS_Shape) wc = new AIS_Shape(Core().helper3D()->intersect(cutWP->Shape(), mf.Shape()));

            wc->SetColor(Quantity_NOC_LIGHTGOLDENRODYELLOW);
            cutPlanes.push_back(wc);
            curOP->cShapes.push_back(wc);
            }
         }
     }
  return cutPlanes;
  }


// prepare toolpath creation for sweepBigC...
void SubOPSweep::createHorizontalToolpaths() {
  std::vector<Handle(AIS_Shape)> cutPlanes = createCutPlanes(curOP);
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
            if (abs(endX   - lastTO.X()) < Core().helper3D()->MinDelta
             || abs(startX - lastTO.X()) < Core().helper3D()->MinDelta) {
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
            if (abs(endY   - lastTO.Y()) < Core().helper3D()->MinDelta
             || abs(startY - lastTO.Y()) < Core().helper3D()->MinDelta) {
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


void SubOPSweep::createOP() {
  int mx = olm->rowCount();
  OperationSubPage::createOP(mx, QString(tr("Sweep #%1")).arg(mx), SweepOperation);
  connectSignals();
  }


void SubOPSweep::createRoundToolpaths() {
  std::vector<Handle(AIS_Shape)> cutPlanes = createCutPlanes(curOP);
  int                            mx = cutPlanes.size();
  gp_Pnt                         from, to, startPos;
  TopoDS_Edge                    e;
  Handle(AIS_Shape)              path;
  bool                           againstFeed = ui->cbDir->currentIndex() == 1;

  for (int i=0; i < mx; ++i) {
      Handle(AIS_Shape) s      = cutPlanes.at(i);
      Bnd_Box           bb     = s->BoundingBox(); bb.SetGap(0);
      gp_Pnt            center = gp_Pnt((bb.CornerMin().X() + bb.CornerMax().X()) / 2
                                      , (bb.CornerMin().Y() + bb.CornerMax().Y()) / 2
                                      , bb.CornerMin().Z());
      double            radius = (bb.CornerMax().X() - bb.CornerMin().X()) / 2;
      double            xStart = radius + activeTool->fluteDiameter() * 0.6;
      double            rStart = radius - curOP->cutWidth() + activeTool->fluteDiameter() / 2;
      double            lastY  = rStart;
      double            curZ   = bb.CornerMin().Z();
      double            curR   = rStart;

      qDebug() << "round cut plane: "
               << bb.CornerMin().X() << "/" << bb.CornerMin().Y() << "/" << bb.CornerMin().Z()
               << "\tto\t"
               << bb.CornerMax().X() << "/" << bb.CornerMax().Y() << "/" << bb.CornerMax().Z();

      if (i) {
         from = to;
         to   = gp_Pnt(startPos.X(), startPos.Y(), from.Z());
         curOP->workSteps().push_back(new WSTraverse(from, to));

         from = to;
         to   = gp_Pnt(startPos.X(), startPos.Y(), curZ);
         curOP->workSteps().push_back(new WSTraverse(from, to));
         }
      from = againstFeed ? gp_Pnt(center.X() + xStart, center.Y() + rStart, curZ)
                         : gp_Pnt(center.X() - xStart, center.Y() + rStart, curZ);
      if (!i) startPos = from;
      to = gp_Pnt(center.X(), center.Y() + rStart, curZ);

      qDebug() << "start line from:" << from.X() << "/"
                                     << from.Y() << "/"
                                     << from.Z()
               << "   to   "         << to.X()   << "/"
                                     << to.Y()   << "/"
                                     << to.Z();
      curOP->workSteps().push_back(new WSStraightMove(from, to));
      from = to;
      curOP->workSteps().push_back(new WSArc(from, to, center, againstFeed));
      lastY = startPos.Y();
      curR -= curOP->cutWidth();

      while (curR > 0) {
            from = to;            
            to   = gp_Pnt(center.X()
                        , -curR
                        , curZ);
            center.SetY(lastY - (lastY + curR) / 2);
            curOP->workSteps().push_back(new WSArc(from, to, center, againstFeed));
            from = to;
            to   = gp_Pnt(center.X(), curR, curZ);
            center.SetY(0);
            curOP->workSteps().push_back(new WSArc(from, to, center, againstFeed));
            curR -= curOP->cutWidth();
            lastY = to.Y();
            }
      from = to;
      to   = gp_Pnt(from.X(), from.Y(), from.Z() + 5);
      curOP->workSteps().push_back(new WSTraverse(from, to));
      }
  }


void SubOPSweep::createVerticalToolpaths() {
  qDebug() << "vertical toolpaths done";
  }


Handle(AIS_Shape) SubOPSweep::genPathOffset(TopoDS_Wire wire, double off) {
  BRepOffsetAPI_MakeOffset moAlgo(wire, GeomAbs_Arc, true);
  Handle(AIS_Shape)        path;

  moAlgo.Perform(off);
  if (moAlgo.IsDone()) {
     path = new AIS_Shape(moAlgo.Shape());
     }
  return path;
  }


void SubOPSweep::processSelection() {
  if (Core().view3D()->selection().size() > 1) {
     cavc::Polyline<double> polyLine = Core().selectionHandler()->createContourFromSelection(curOP);
     TopoDS_Shape           wire     = Core().helper3D()->pl2Wire(polyLine, -100);
     Handle(AIS_Shape)      aw       = new AIS_Shape(wire);
     gp_Vec                 prismVec(0, 0, 500);
     TopoDS_Shape           cuttingFace = BRepPrimAPI_MakePrism(wire, prismVec);
     Handle(AIS_Shape)      acf         = new AIS_Shape(cuttingFace);

     acf->SetColor(Quantity_NOC_CYAN);
     aw->SetColor(Quantity_NOC_ORANGE);
     acf->SetTransparency(0.8);
     aw->SetWidth(3);
     curOP->cShapes.push_back(aw);
     curOP->cShapes.push_back(acf);
     }
  else {
     TopoDS_Shape           selectedShape = Core().view3D()->selection().at(0);
     Handle(Geom_Surface)   selectedFace  = BRep_Tool::Surface(TopoDS::Face(selectedShape));
     Handle(AIS_Shape)      asTmp         = new AIS_Shape(selectedShape);
     Bnd_Box                bbModel       = curOP->mBounds; bbModel.SetGap(0);
     Bnd_Box                bbSel         = asTmp->BoundingBox(); bbSel.SetGap(0);
     SweepTargetDefinition* std           = nullptr;

     if (selectedFace->IsKind(STANDARD_TYPE(Geom_Plane))) {
        GeomAdaptor_Surface selectedSurface(selectedFace);
        gp_Pln              pln = selectedSurface.Plane();
        gp_Pnt              pos = Core().helper3D()->deburr(pln.Location());
        gp_Dir              dir = Core().helper3D()->deburr(pln.Axis().Direction());

        if (abs(dir.Z()) < Core().helper3D()->MinDelta) { // vertical plane
           Handle(AIS_Shape) cutPart = Core().selectionHandler()->createCutPart(curOP);

           cutPart->SetColor(Quantity_NOC_CYAN);
           cutPart->SetTransparency(0.8);
           curOP->cShapes.push_back(cutPart);

//           std = new SweepTargetDefinition(pos, dir);
//           std->setZMin(bbSel.CornerMin().Z());
//           std->setZMax(bbSel.CornerMax().Z());
//           curOP->targets.push_back(std);
//           curOP->setVertical(true);
//           tdModel->append(std);
//           ui->spDepth->setValue(curZ);
           }
        else if ((1 - abs(dir.Z())) < Core().helper3D()->MinDelta) { // horizontal plane
           std = new SweepTargetDefinition(pos, dir);
           std->setZMin(bbSel.CornerMin().Z());
           std->setZMax(bbSel.CornerMax().Z());
           tdModel->append(std);
           curOP->setVertical(false);
           ui->spDepth->setValue(pos.Z());
           Handle(AIS_Shape) cutPart = Core().selectionHandler()->createCutPart(curOP);

           cutPart->SetColor(Quantity_NOC_CYAN);
           cutPart->SetTransparency(0.8);
           curOP->cShapes.push_back(cutPart);
           }
        }
     }
  Core().view3D()->showShapes(curOP->cShapes);
  Core().view3D()->refresh();
  }


void SubOPSweep::showToolPath() {
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


void SubOPSweep::toolPath() {
  qDebug() << "OP sweep - gonna create toolpath ...";

  if (!curOP->cutDepth()) return;
  if (curOP->toolPaths.size()) {
     Core().view3D()->removeShapes(curOP->toolPaths);
     curOP->toolPaths.clear();
     }
  if (curOP->cShapes.size()) {
     Core().view3D()->removeShapes(curOP->cShapes);
     curOP->cShapes.clear();
     }
  if (curOP->workSteps().size()) curOP->workSteps().clear();
  if (curOP->isVertical()) createVerticalToolpaths();
  else {
     if (Core().workData()->roundWorkPiece) createRoundToolpaths();
     else                                   createHorizontalToolpaths();
     }
  Core().view3D()->showShapes(curOP->toolPaths, false);
  if (curOP->showCutParts) Core().view3D()->showShapes(curOP->cShapes, false);
  Core().view3D()->refresh();
  showToolPath();
  }
