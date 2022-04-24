/* 
 * **************************************************************************
 * 
 *  file:       pathbuilder.cpp
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
#include "pathbuilder.h"
#include "core.h"
#include "gocircle.h"
#include "gocontour.h"
#include "goline.h"
#include "gopocket.h"
#include "kuteCAM.h"
#include "occtviewer.h"
#include "operation.h"
#include "sweeptargetdefinition.h"
#include "toolentry.h"
#include "toollistmodel.h"
#include "util3d.h"
#include "workstep.h"
#include "wstraverse.h"
#include "wsstraightmove.h"
#include "wsarc.h"
#include "wscycle.h"
#include <AIS_Shape.hxx>
#include <Bnd_Box.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <QDebug>


PathBuilder::PathBuilder() {
  }


double PathBuilder::calcAdditionalOffset(SweepTargetDefinition* std, GOContour* c) {
  double rv = 0;

  qDebug() << "requested additional offset for border processing!";
  if (!std || !c) return rv;
  switch (getDominantAxis(std->baseDir())) {
    case  1:
    case -1:
         rv = (std->baseDir().X() > 0 ? std->baseDelim().CornerMax().X()
                                      : std->baseDelim().CornerMin().X())
            - fmax(c->startPoint().X(), c->endPoint().X());
         break;
    case  2:
    case -2:
         rv = (std->baseDir().Y() > 0 ? std->baseDelim().CornerMax().Y()
                                      : std->baseDelim().CornerMin().Y())
            - fmax(c->startPoint().Y(), c->endPoint().Y());
         break;
    case  3:
    case -3:
         rv = (std->baseDir().Z() > 0 ? std->baseDelim().CornerMax().Z()
                                      : std->baseDelim().CornerMin().Z())
            - fmax(c->startPoint().Z(), c->endPoint().Z());
         break;
    default: break;
    }
  return rv;
  }


int PathBuilder::calcSafeOffsets(double& safeX, double& safeY, const gp_Dir& baseNorm, Operation* op, double xtend) {
  int    dominantAxis = getDominantAxis(baseNorm);

  switch (dominantAxis) {
    case -1:
         safeX = op->wpBounds.CornerMin().X() - xtend;
         safeY = fmax(op->wpBounds.CornerMin().Y() - xtend
                    , op->wpBounds.CornerMax().Y() + xtend);
         break;
    case  1:
         safeX = op->wpBounds.CornerMax().X() + xtend;
         safeY = fmax(op->wpBounds.CornerMin().Y() - xtend
                    , op->wpBounds.CornerMax().Y() + xtend);
         break;
    case -2:
         safeX = fmax(op->wpBounds.CornerMin().X() - xtend
                    , op->wpBounds.CornerMax().X() + xtend);
         safeY = op->wpBounds.CornerMin().Y() - xtend;
         break;
    case  2:
         safeX = fmax(op->wpBounds.CornerMin().X() - xtend
                    , op->wpBounds.CornerMax().X() + xtend);
         safeY = op->wpBounds.CornerMax().Y() + xtend;
         break;
    default:
         safeX = fmax(op->wpBounds.CornerMin().X() - xtend
                    , op->wpBounds.CornerMax().X() + xtend);
         safeY = fmax(op->wpBounds.CornerMin().Y() - xtend
                    , op->wpBounds.CornerMax().Y() + xtend);
         break;
    }
  return dominantAxis;
  }


std::vector<Workstep*> PathBuilder::genPath4Pockets(Operation* op, const Bnd_Box& bb, const gp_Dir& baseNorm, const std::vector<std::vector<GOPocket*>>& pool, double xtend) {
  double safeX, safeY;
  gp_Pnt center(op->wpBounds.CornerMin().X() + (op->wpBounds.CornerMax().X() - op->wpBounds.CornerMin().X()) / 2
              , op->wpBounds.CornerMin().Y() + (op->wpBounds.CornerMax().Y() - op->wpBounds.CornerMin().Y()) / 2
              , op->wpBounds.CornerMin().Z() + (op->wpBounds.CornerMax().Z() - op->wpBounds.CornerMin().Z()) / 2);
  int dominantAxis = calcSafeOffsets(safeX, safeY, baseNorm, op, xtend);
  std::vector<Workstep*> workSteps;
  gp_Pnt  startPos(0, 0, 300);
  gp_Pnt  endPos = startPos;

  for (auto levelParts : pool) {
      int n=0;

      for (GOPocket* p : levelParts) {
          int mx = p->contours().size();

          ++n;
          for (int i=0; i < mx; ++i) {
              GOContour* c   = p->contours().at(i);
              int        mxJ = c->size() - 1;

              if (Core().helper3D()->isEqual(startPos, endPos)) {
                 endPos = c->startPoint();
                 endPos.SetZ(startPos.Z());

                 workSteps.push_back(new WSTraverse(startPos, endPos));
                 }
              else {
                 // set startpos to endpoint of last cutpath (contour)
                 // need 3 moves to startpoint of next cutpath, as both
                 // endpoints of cutpath are in safe area
                 startPos = endPos;
                 endPos   = c->startPoint();
                 gp_Pnt tmp = startPos;
                 double debugOffset = i*3 + n*3;

                 if (dominantAxis == 1 || dominantAxis == -1) { // first move to safe x
                    if (dominantAxis > 0) tmp.SetX(safeX + debugOffset);
                    else                  tmp.SetX(safeX - debugOffset);

                    workSteps.push_back(new WSTraverse(startPos, tmp));
                    startPos = tmp;
                    tmp.SetY(endPos.Y());
                    }
                 else {                                         // first move to safe y
                    if (dominantAxis > 0) tmp.SetY(safeY + debugOffset);
                    else                  tmp.SetY(safeY - debugOffset);

                    workSteps.push_back(new WSTraverse(startPos, tmp));
                    startPos = tmp;
                    tmp.SetX(endPos.X());
                    }
                 if (Core().helper3D()->isEqual(startPos, tmp)) tmp.SetX(tmp.X() + Core().MinDelta);
                 workSteps.push_back(new WSTraverse(startPos, tmp));
                 endPos = tmp;
                 }
              startPos = endPos;
              endPos = c->startPoint();

              workSteps.push_back(new WSTraverse(startPos, endPos));
              for (int j=0; j <= mxJ; ++j) {
                  GraphicObject* go = c->segments().at(j);
                  GOCircle*      gc = static_cast<GOCircle*>(go);
                  Workstep*      ws;

                  switch (go->type()) {
                    case GTLine:
                         ws = new WSStraightMove(go->startPoint(), go->endPoint());
                         break;

                    case GTCircle:
                         ws = new WSArc(go->startPoint()
                                      , go->endPoint()
                                      , gc->center()
                                      , gc->isCCW());
                      break;
                    default: break;
                    }
                  workSteps.push_back(ws);
                  }
              endPos = c->endPoint();
              }
          }
      }
  return workSteps;
  }


std::vector<Workstep*> PathBuilder::genRoundToolpaths(Operation* op, const std::vector<Handle(AIS_Shape)>& cutPlanes) {
  std::vector<Workstep*> workSteps;
  int                    mx = cutPlanes.size();
  gp_Pnt                 from, to, startXXPos;
  ToolEntry*             activeTool  = Core().toolListModel()->tool(Core().toolListModel()->findToolNum(op->toolNum()));
  double                 xtend       = activeTool->fluteDiameter() * 0.8;
  TopoDS_Edge            e;
  Handle(AIS_Shape)      path;
  bool                   againstFeed = op->direction() == 1;

  for (int i=0; i < mx; ++i) {
      Handle(AIS_Shape) s      = cutPlanes.at(i);
      Bnd_Box           bb     = s->BoundingBox(); bb.SetGap(0);
      gp_Pnt            center = gp_Pnt((bb.CornerMin().X() + bb.CornerMax().X()) / 2
                                      , (bb.CornerMin().Y() + bb.CornerMax().Y()) / 2
                                      , bb.CornerMin().Z());
      double            radius = (bb.CornerMax().X() - bb.CornerMin().X()) / 2;
      double            xStart = radius + activeTool->fluteDiameter() * 0.6;
      double            rStart = radius - op->cutWidth() + activeTool->fluteDiameter() / 2;
      double            lastY  = rStart;
      double            curZ   = bb.CornerMin().Z();
      double            curR   = rStart;

      qDebug() << "round cut plane: "
               << bb.CornerMin().X() << "/" << bb.CornerMin().Y() << "/" << bb.CornerMin().Z()
               << "\tto\t"
               << bb.CornerMax().X() << "/" << bb.CornerMax().Y() << "/" << bb.CornerMax().Z();

      if (i) {
         from = to;
         to   = gp_Pnt(startXXPos.X(), startXXPos.Y(), from.Z());
         workSteps.push_back(new WSTraverse(from, to));

         from = to;
         to   = gp_Pnt(startXXPos.X(), startXXPos.Y(), curZ);
         workSteps.push_back(new WSTraverse(from, to));
         }
      from = againstFeed ? gp_Pnt(center.X() + xStart, center.Y() + rStart, curZ)
                         : gp_Pnt(center.X() - xStart, center.Y() + rStart, curZ);
      if (!i) startXXPos = from;
      to = gp_Pnt(center.X(), center.Y() + rStart, curZ);

      qDebug() << "start line from:" << from.X() << "/"
                                     << from.Y() << "/"
                                     << from.Z()
               << "   to   "         << to.X()   << "/"
                                     << to.Y()   << "/"
                                     << to.Z();
      workSteps.push_back(new WSStraightMove(from, to));
      from = to;
      workSteps.push_back(new WSArc(from, to, center, againstFeed));
      lastY = startXXPos.Y();
      curR -= op->cutWidth();

      while (curR > 0) {
            from = to;
            to   = gp_Pnt(center.X()
                        , -curR
                        , curZ);
            center.SetY(lastY - (lastY + curR) / 2);
            workSteps.push_back(new WSArc(from, to, center, againstFeed));
            from = to;
            to   = gp_Pnt(center.X(), curR, curZ);
            center.SetY(0);
            workSteps.push_back(new WSArc(from, to, center, againstFeed));
            curR -= op->cutWidth();
            lastY = to.Y();
            }
      from = to;
      to   = gp_Pnt(from.X(), from.Y(), from.Z() + 5);
      workSteps.push_back(new WSTraverse(from, to));
      }
  return workSteps;
  }


std::vector<Workstep*> PathBuilder::genToolPath(Operation* op, Handle(AIS_Shape) cutPart) {
  SweepTargetDefinition* std = static_cast<SweepTargetDefinition*>(op->targets.at(0));
  gp_Pnt center(op->wpBounds.CornerMin().X() + (op->wpBounds.CornerMax().X() - op->wpBounds.CornerMin().X()) / 2
              , op->wpBounds.CornerMin().Y() + (op->wpBounds.CornerMax().Y() - op->wpBounds.CornerMin().Y()) / 2
              , op->wpBounds.CornerMin().Z() + (op->wpBounds.CornerMax().Z() - op->wpBounds.CornerMin().Z()) / 2);
  GOContour*             contour = std->contour();
  std::vector<Workstep*> toolPath;

  if (!contour) return toolPath;    // no vertical toolpath without contour!
  std::vector<std::vector<std::vector<GOContour*>>> clippedParts;
  ToolEntry*             activeTool    = Core().toolListModel()->tool(Core().toolListModel()->findToolNum(op->toolNum()));
  double                 xtend         = activeTool->fluteDiameter() * 0.8;
  double                 firstOffset   = op->offset() + activeTool->fluteDiameter() / 2;
  Bnd_Box                bb            = cutPart->BoundingBox();
  bool                   curveIsBorder = std->isBaseBorder();
  double                 startZ        = bb.CornerMax().Z();
  double                 lastZ         = fmax(startZ - activeTool->cuttingDepth()
                                            , op->finalDepth() + op->offset());
  double                 curZ          = startZ;
  int                    n=0;

  if (curveIsBorder) firstOffset += abs(calcAdditionalOffset(std, contour));
  contour->simplify(curZ);
  qDebug() << "cutting contour:" << contour->toString();
  qDebug() << "center of workpiece:" << center.X() << " / " << center.Y();
  curZ -= op->cutDepth();

  // prepare raw toolpaths
  while (curZ > lastZ) {
        qDebug() << "cut depth is" << curZ;
        std::vector<std::vector<GOContour*>> levelContours = processCurve(op, contour, curveIsBorder, center, xtend, firstOffset, curZ);

        if (levelContours.size()) clippedParts.push_back(levelContours);
        curZ -= op->cutDepth();
        }
  curZ = lastZ;
  std::vector<std::vector<GOContour*>> levelContours = processCurve(op, contour, curveIsBorder, center, xtend, firstOffset, curZ);
  if (levelContours.size()) clippedParts.push_back(levelContours);

  qDebug() << "have" << clippedParts.size() << "cut stages\n";

  if (!clippedParts.size()) return toolPath;
  std::vector<std::vector<GOPocket*>> pool = splitCurves(clippedParts);
  toolPath = genPath4Pockets(op, bb, std->baseDir(), pool, xtend);

  return toolPath;
  }


// all contiguous segments form a contour (instance of GOContour).
// A cutted offset curve can lead to several subcontours (vector of GOContour)
std::vector<std::vector<GOContour*>> PathBuilder::processCurve(Operation* op, GOContour* curve, bool curveIsBorder, const gp_Pnt& center, double xtend, double firstOffset, double curZ) {
  BRepOffsetAPI_MakeOffset              offMaker(TopoDS::Wire(curve->toWire(curZ)));
  TopoDS_Shape                          rawPath;
  std::vector<std::vector<GOContour*>>  levelContours;
  bool                                  debug = false;

  for (int i=0; true; ++i) {
      std::vector<GOContour*> pathParts;

      offMaker.Perform(firstOffset + i * op->cutWidth(), 0);
      if (offMaker.IsDone()) {
         Handle(TopTools_HSequenceOfShape) edgePool = new TopTools_HSequenceOfShape;
         TopoDS_Shape                      offWire  = Core().helper3D()->allEdgesWithin(offMaker.Shape(), edgePool);
         GOContour*                        path     = new GOContour(center);
         GraphicObject*                    goSeg    = nullptr;

         if (debug) {
            Handle(AIS_Shape) aw = new AIS_Shape(offWire);

            aw->SetColor(Quantity_NOC_ORANGE2);
            aw->SetWidth(3);
            op->cShapes.push_back(aw);
            if (i > 13) break;
            continue;
            }
         if (!i && !curveIsBorder) {
            std::vector<TopoDS_Edge> segments = Core().helper3D()->allEdgesWithin(offWire);

            for (auto s : segments) path->add(s);
            path->simplify(curZ);
            stripPath(path, curve);
            path->extendBy(xtend);
            pathParts.push_back(path);
            }
         else {
            rawPath = BRepAlgoAPI_Common(offWire, op->cutPart->Shape());
            std::vector<TopoDS_Edge> segments = Core().helper3D()->allEdgesWithin(rawPath);

            if (!segments.size()) break;
            for (int j=0; j < segments.size(); ++j) {
                goSeg = path->add(segments.at(j));

                if (!goSeg) continue;
                if (path->size()) {
                   path->extendBy(xtend);
                   pathParts.push_back(path);
                   path = new GOContour(center);
                   if (goSeg) path->add(goSeg);
                   }
                }
            if (path->size()) {
               path->extendBy(xtend);
               pathParts.push_back(path);
               }
            }
         }
      else {
//         throw new std::domain_error("failed to create offset curve");
         return levelContours;
         }
      if (pathParts.size()) {
         std::sort(pathParts.begin(), pathParts.end(), compContour);
         levelContours.push_back(pathParts);
         }
      // next offset curve of same Z-value
      }
  // next Z-value

  return levelContours;
  }


std::vector<std::vector<GOPocket*>> PathBuilder::splitCurves(const std::vector<std::vector<std::vector<GOContour*>>>& pool) {
  std::vector<std::vector<GOPocket*>> levels;
  auto& c = pool.at(0);
  int nL = 0, nC = 0, nCC = 0;

  qDebug() << "pool has" << pool.size() << "Z-levels";
  for (auto lp : pool) {
      std::vector<GOPocket*> pockets;
      int count    = 0;
      int maxCount = 0;

      qDebug() << "\n" << ++nL << "level has parts of" << lp.size() << "offset contours";

      // level-vector with most items is base for pocket generation
      for (int i=0; i < lp.size(); ++i) {
          if (lp.at(i).size() > count) {
             count    = lp.at(i).size();
             maxCount = i;
             }
          }
      qDebug() << "level" << maxCount << "has" << count << "pockets";
      auto cp = lp.at(maxCount);

      // first create all pockets necessary
      for (auto c : cp) pockets.push_back(new GOPocket(c->angStart(), c->angEnd()));

      // process levelParts from out to in and fill created pockets
      for (auto i = lp.rbegin(); i != lp.rend(); ++i) {
          auto cp = *i;

          for (int n=0; n < cp.size(); ++n) {
              auto   c  = cp.at(n);
              double a0 = atan2(c->startPoint().Y(), c->startPoint().X());
              double a1 = atan2(c->endPoint().Y(), c->endPoint().X());

              if (a0 < a1)
                 c->invert();
              a0 = atan2(c->startPoint().Y(), c->startPoint().X());
              a1 = atan2(c->endPoint().Y(), c->endPoint().X());
              qDebug() << "\tcontour from"
                       << c->startPoint().X() << " / " << c->startPoint().Y() << " / " << c->startPoint().Z()
                       << "   to   "
                       << c->endPoint().X() << " / " << c->endPoint().Y() << " / " << c->endPoint().Z()
                       << "   with a0:" << a0 << " and a1:" << a1;

              pockets.at(n)->contours().push_back(c);
              }
          }
      // pockets with lowest item count should be first to be processed
      std::sort(pockets.begin(), pockets.end(), compPocket);
      levels.push_back(pockets);
      }
  return levels;
  }


// dumb but fast
void PathBuilder::stripPath(GOContour *firstContour, GOContour *masterContour) {
  if (!firstContour || !masterContour) return;

  for (int i=0; i < firstContour->segments().size(); ++i) {
      GraphicObject* gl = firstContour->segments().at(i);

      if (gl->type() == GTLine) {
         GOLine* line0 = static_cast<GOLine*>(gl);
         bool found = false;

         for (int j=0; j < masterContour->segments().size(); ++j) {
             GraphicObject* gr = masterContour->segments().at(j);

             if (gr->type() == GTLine) {
                GOLine* line1 = static_cast<GOLine*>(gr);
                gp_Dir  d0    = line0->direction();
                gp_Dir  d1    = line1->direction();

                if (d0.IsEqual(d1, Core().MinDelta)) {
                   found = true;
                   break;
                   }
                }
             }
         if (!found) {
            if (i && firstContour->segments().at(i-1)->type() != GTLine) {
               firstContour->segments().erase(firstContour->segments().begin() + i - 1);
               --i;
               }
            firstContour->segments().erase(firstContour->segments().begin() + i--);
            }
         }
      else {
         if (!i) firstContour->segments().erase(firstContour->segments().begin() + i--);
         }
      }
  }
