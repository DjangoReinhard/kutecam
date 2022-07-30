/* 
 * **************************************************************************
 * 
 *  file:       sweeppathbuilder.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    27.7.2022 by Django Reinhard
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
#include "sweeppathbuilder.h"
#include "core.h"
#include "kuteCAM.h"
#include "operation.h"
#include "toolentry.h"
#include "toollistmodel.h"
#include "work.h"
#include "wsstraightmove.h"
#include "wstraverse.h"
#include <QDebug>


SweepPathBuilder::SweepPathBuilder(PathBuilderUtil* pbu)
 : pbu(pbu) {
  }


// prepare toolpath creation for sweepBigC...
void SweepPathBuilder::createHorizontalToolpaths(Operation* op, const std::vector<Handle(AIS_Shape)>& cutPlanes) {
  Work*  work = Core().workData();
  ToolEntry* activeTool = op->toolEntry();
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

         if (deltaY < op->cutWidth()) {
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
               op->workSteps().push_back(new WSTraverse(lastTO, from));
               }
            op->workSteps().push_back(new WSStraightMove(from, to));
            }
         else {
            if (op->direction() == 1) to = sweepBigCounterClockwise(op, activeTool, bb, lastTO);
            else                      to = sweepBigClockwise(op, activeTool, bb, lastTO);
            }
         }
      else {
         // y toggle, x advance
         double startY = bb.CornerMin().Y() - activeTool->fluteDiameter() * 0.6;
         double endY   = bb.CornerMax().Y() + activeTool->fluteDiameter() * 0.6;

         if (deltaX < op->cutWidth()) {
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
              op->workSteps().push_back(new WSTraverse(lastTO, from));
              }
            op->workSteps().push_back(new WSStraightMove(from, to));
            }
         else {
            if (op->direction() == 1) to = sweepBigCounterClockwise(op, activeTool, bb, lastTO);
            else                      to = sweepBigClockwise(op, activeTool, bb, lastTO);
            }
         }
      lastTO = to;
      }
  }




gp_Pnt SweepPathBuilder::sweepBigClockwise(Operation* op, ToolEntry* activeTool, const Bnd_Box& bb, const gp_Pnt& lastTO) {
  double xMin = bb.CornerMin().X() + op->cutWidth() - activeTool->fluteDiameter() / 2;
  double xMax = bb.CornerMax().X() - op->cutWidth() + activeTool->fluteDiameter() / 2;
  double yMin = bb.CornerMin().Y() + op->cutWidth() - activeTool->fluteDiameter() / 2;
  double yMax = bb.CornerMax().Y() - op->cutWidth() + activeTool->fluteDiameter() / 2;
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
     op->workSteps().push_back(new WSTraverse(from, to));
     from = to;
     to.SetX(startPos.X());
     to.SetY(startPos.Y());
     op->workSteps().push_back(new WSTraverse(from, to));
     from = to;
     to   = startPos;
     op->workSteps().push_back(new WSTraverse(from, to));
     }
  do {
     from = to;
     to = gp_Pnt(curX1, curY0, curZ);
     op->workSteps().push_back(new WSStraightMove(from, to));
     curX1 -= op->cutWidth();

     from = to;
     to   = gp_Pnt(curX0, curY0, curZ);
     op->workSteps().push_back(new WSStraightMove(from, to));
     curY0 += op->cutWidth();

     from = to;
     to   = gp_Pnt(curX0, curY1, curZ);
     op->workSteps().push_back(new WSStraightMove(from, to));
     curX0 += op->cutWidth();

     if (curX1 < curX0) {
        qDebug() << "leave cutplane before closing rectangle - last y"
                 << curY1 << " - y before:" << curY0;
        from = to;
        to.SetY(cycle ? curY0 : startPos.Y());
        op->workSteps().push_back(new WSStraightMove(from, to));
        break;
        }
     from = to;
     to   = gp_Pnt(curX1, curY1, curZ);
     op->workSteps().push_back(new WSStraightMove(from, to));
     curY1 -= op->cutWidth();

     from = to;
     ++cycle;
     } while (curY1 > curY0);
  return to;
  }


gp_Pnt SweepPathBuilder::sweepBigCounterClockwise(Operation* op, ToolEntry* activeTool, const Bnd_Box& bb, const gp_Pnt& lastTO) {
  double xMin = bb.CornerMin().X() + op->cutWidth() - activeTool->fluteDiameter() / 2;
  double xMax = bb.CornerMax().X() - op->cutWidth() + activeTool->fluteDiameter() / 2;
  double yMin = bb.CornerMin().Y() + op->cutWidth() - activeTool->fluteDiameter() / 2;
  double yMax = bb.CornerMax().Y() - op->cutWidth() + activeTool->fluteDiameter() / 2;
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
     op->workSteps().push_back(new WSTraverse(from, to));
     from = to;
     to.SetX(startPos.X());
     to.SetY(startPos.Y());
     op->workSteps().push_back(new WSTraverse(from, to));
     from = to;
     to   = startPos;
     op->workSteps().push_back(new WSTraverse(from, to));
     }
  do {
     from = to;
     to = gp_Pnt(curX0, curY1, curZ);
     op->workSteps().push_back(new WSStraightMove(from, to));
     curY1 -= op->cutWidth();

     from = to;
     to   = gp_Pnt(curX0, curY0, curZ);
     op->workSteps().push_back(new WSStraightMove(from, to));
     curX0 += op->cutWidth();

     from = to;
     to   = gp_Pnt(curX1, curY0, curZ);
     op->workSteps().push_back(new WSStraightMove(from, to));
     curY0 += op->cutWidth();

     if (curY1 < curY0) {
        qDebug() << "leave cutplane before closing rectangle - last y"
                 << curY1 << " - y before:" << curY0;
        from = to;
        to.SetY(cycle ? curY0 : startPos.Y());
        op->workSteps().push_back(new WSStraightMove(from, to));
        break;
        }
     from = to;
     to   = gp_Pnt(curX1, curY1, curZ);
     op->workSteps().push_back(new WSStraightMove(from, to));
     curX1 -= op->cutWidth();

     from = to;
     ++cycle;
     } while (curX1 > curX0);
  return to;
  }
