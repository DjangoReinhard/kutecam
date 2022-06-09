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
#include "contourtargetdefinition.h"
#include "sweeptargetdefinition.h"
#include "toolentry.h"
#include "toollistmodel.h"
#include "util3d.h"
#include "work.h"
#include "workstep.h"
#include "wstraverse.h"
#include "wsstraightmove.h"
#include "wsarc.h"
#include "wscycle.h"
#include <AIS_Shape.hxx>
#include <Bnd_Box.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRep_Tool.hxx>
#include <GeomAPI_ExtremaCurveCurve.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <QDebug>


static bool cmpContour(GOContour* l, GOContour* r) {
  if (!l || !r) return false;
  if (l->order() == r->order())
     return l->a0() < r->a0();
  return l->order() < r->order();
  }


PathBuilder::PathBuilder() {
  }


double PathBuilder::calcAdditionalOffset(SweepTargetDefinition* std, GOContour* c) {
  double rv = 0;

  qDebug() << "requested additional offset for border processing!";
  if (!std || !c) return rv;
  switch (kute::getDominantAxis(std->baseDir())) {
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


void PathBuilder::cleanup(std::vector<Workstep*>& tp) {
  for (int i=0; i < tp.size(); ++i) {
      Workstep* ws = tp.at(i);

      if (kute::isEqual(ws->startPos(), ws->endPos())) {
         tp.erase(tp.begin() + i--);
         }
      }
  }


void dump(std::vector<std::vector<std::vector<GOContour*>>> cPool) {
  int n = 0;

  qDebug() << "<< ================ dump clipped parts =======================";
  for (auto& cutPaths : cPool) {
      int i=0;

      qDebug() << ">> dump layer #" << ++n;

      for (auto& p : cutPaths) {
          int j=0;

          qDebug() << ">> dump path #" << ++i;

          for (auto& c : p) {
              qDebug() << ">> contour #" << ++j << c->sType()
                       << "   from " << c->startPoint().X() << " / " << c->startPoint().Y()
                       << "   to   " << c->endPoint().X() << " / " << c->endPoint().Y();
              qDebug() << "   real - a0:" << c->a0() << " <> a1:" << c->a1();
              qDebug() << c->toString();
              }
          }
      }
  qDebug() << "<< ================ dump clipped parts =======================";
  }


std::vector<Workstep*> PathBuilder::genBasicPath(std::vector<std::vector<GOContour*>> levelParts) {
  std::vector<Workstep*> toolPath;

  for (int j=0; j < levelParts.size(); ++j) {
      auto& contours = levelParts.at(j);

      for (int k=0; k < contours.size(); ++k) {
          auto& c = contours.at(k);

          processContour(toolPath, c);
          }
      }
  return toolPath;
  }


std::vector<Workstep*> PathBuilder::genFlatPaths(Operation* op, std::vector<Handle(AIS_Shape)> cutPlanes, std::vector<std::vector<std::vector<GOContour*>>> clippedParts, double curZ, double xtend, int level) {
  std::vector<std::vector<GOContour*>> pool;
  std::vector<Workstep*> toolPath;

  if (level < 0) {
     for (auto& lp: clippedParts) {                            // flatten clippedParts ...
         std::vector<GOContour*> contours;

         for (auto& p : lp) {
             for (auto& c : p) {
                 contours.push_back(c);
                 }
             }
         pool.push_back(contours);
         }
     }
  else {
     auto& lp = clippedParts.at(level);
     std::vector<GOContour*> contours;

     for (auto& p : lp) {
         for (auto& c : p) {
             contours.push_back(c);
             }
         }
     pool.push_back(contours);
     }

  qDebug() << "collected " << pool.size() << " contours";
  gp_Pnt s = gp_Pnt(0, 0, 300), e = s;
  int    lmi=0, lmx = pool.size();

  //TODO: split curves?
  for (int l=lmi; l < lmx; ++l) {
      auto& lp = pool.at(l);
      Bnd_Box bb = cutPlanes.at(l)->BoundingBox();
      int mx   = fmin(lp.size() - 1, 994);
      int mi   = 0; //fmax(0, lp.size() - 3);

      std::sort(lp.begin(), lp.end(), cmpContour);
      for (int i=mx; i >= mi; --i) {
          GOContour* c = lp.at(i);
          double a0 = c->a0();
          double a1 = c->a1();

          while (a0 < 0) a0 += 2*M_PI;
          while (a1 < 0) a1 += 2*M_PI;
          if (op->direction()) {
             if (a0 > a1) c->invert();
             }
          else {
             if (a0 < a1) c->invert();
             }
//          c->simplify(curZ);
          s = c->startPoint();

          //TODO: block region for intermediate moves!

                            //TODO: remove debug offset from xtend
          genInterMove(toolPath, e, s, c->centerPoint(), bb, xtend + i);
//          drawDebugContour(op, c, curZ);
          e = processContour(toolPath, c);
//          e = c->endPoint();
          }
      s = e;
      e.SetZ(op->topZ() + op->safeZ1());
      toolPath.push_back(new WSTraverse(s, e));
      curZ -= op->cutDepth();
      if (curZ < op->finalDepth()) break;
      }
  return toolPath;
  }


//TODO: block region
gp_Pnt PathBuilder::genInterMove(std::vector<Workstep*>& ws, const gp_Pnt& from, const gp_Pnt& to, const gp_Pnt& center, const Bnd_Box& bb, double xtend) {
  int reg0 = region(from, bb);
  int reg1 = region(to, bb);
  gp_Pnt   e=to, s=from, tmp=from;

//  center.SetZ(to.Z());
  if (reg1 & Top    && e.Y() < (bb.CornerMax().Y() + xtend))
     e.SetY(bb.CornerMax().Y() + xtend);
  else if (reg1 & Bottom && e.Y() > (bb.CornerMin().Y() - xtend))
     e.SetY(bb.CornerMin().Y() - xtend);
  else if (reg1 & Left   && e.X() > (bb.CornerMin().X() - xtend))
     e.SetX(bb.CornerMin().X() - xtend);
  else if (reg1 & Right  && e.X() < (bb.CornerMax().X() + xtend))
     e.SetX(bb.CornerMax().X() + xtend);
  double check0 = e.Z() - tmp.Z();
  double check1 = tmp.Z() - e.Z();

  if ((tmp.Z() - e.Z()) > kute::MinDelta) {
     tmp = e;
     tmp.SetZ(s.Z());
     if (!kute::isEqual(s, tmp)) {
        ws.push_back(new WSTraverse(s, tmp));
        s = tmp;
        }
     }
  else {
     if (reg0 & Top && tmp.Y()    < (bb.CornerMax().Y() + xtend))
        s.SetY(bb.CornerMax().Y() + xtend);
     else if (reg0 & Bottom && tmp.Y() > (bb.CornerMin().Y() - xtend))
        s.SetY(bb.CornerMin().Y() - xtend);
     else if (reg0 & Left && tmp.X()   > (bb.CornerMin().X() - xtend))
        s.SetX(bb.CornerMin().X() - xtend);
     else if (reg0 & Right && tmp.X()  < (bb.CornerMax().X() + xtend))
        s.SetX(bb.CornerMax().X() + xtend);
     else if (!reg0 && reg1) {
        if (reg1 & Top && s.Y() > center.Y())
           s.SetY(bb.CornerMax().Y() + xtend);
        else if (reg1 & Bottom && s.Y() < center.Y())
           s.SetY(bb.CornerMin().Y() - xtend);
        else if (reg1 & Left && s.X() < center.X())
           s.SetX(bb.CornerMin().X() - xtend);
        else if (reg1 & Right && s.X() > center.X())
           s.SetX(bb.CornerMax().X() + xtend);
        }
     if (!kute::isEqual(s, from)) {
        ws.push_back(new WSStraightMove(from, s));
        tmp = s;
        }
     }

  if (!reg0 && !reg1) {                         // both points are inside workpiece!
     ws.push_back(new WSStraightMove(from, to));
     }
  else if (reg0 & reg1) {                         // both points are inside same region outside
                                                  // s and e are both in safe area
     if (!kute::isEqual(s, e))  ws.push_back(new WSTraverse(s, e));
     if (!kute::isEqual(e, to)) ws.push_back(new WSStraightMove(e, to));
     }
  else if (!kute::isEqual(s.Z(), e.Z())) {
     ws.push_back(new WSTraverse(s, e));
     if (!kute::isEqual(e, to)) ws.push_back(new WSStraightMove(e, to));
     }
  else {                                        // both points are in different regions
     // have to move from s to e (both in safe area)
     if (reg0 & Top) {
        int q1 = 0;

        if (!reg1) q1 = quadrant(e) + 1;
        if (reg1 & Left || q1 == 2) {
           tmp.SetX(bb.CornerMin().X() - xtend);
           if (!kute::isEqual(s, tmp)) {
              ws.push_back(new WSTraverse(s, tmp));
              s = tmp;
              }
           tmp.SetY(e.Y());
           }
        else if (reg1 & Right || q1 == 1) {
           tmp.SetX(bb.CornerMax().X() + xtend);
           if (!kute::isEqual(s, tmp)) {
              ws.push_back(new WSTraverse(s, tmp));
              s = tmp;
              }
           tmp.SetY(e.Y());
           }
        else {
           if (e.X() < center.X()) tmp.SetX(bb.CornerMin().X() - xtend);
           else                    tmp.SetX(bb.CornerMax().X() + xtend);
           if (!kute::isEqual(s, tmp)) {
              ws.push_back(new WSTraverse(s, tmp));
              s = tmp;
              }
           tmp.SetY(bb.CornerMin().Y() - xtend);
           if (!kute::isEqual(s, tmp)) {
              ws.push_back(new WSTraverse(s, tmp));
              s = tmp;
              }
           tmp.SetX(e.X());
           }
        }
     else if (reg0 & Bottom) {
        int q1 = 0;

        if (!reg1) q1 = quadrant(e) + 1;
        if (reg1 & Left || q1 == 3) {
           tmp.SetX(bb.CornerMin().X() - xtend);
           if (!kute::isEqual(s, tmp)) {
              ws.push_back(new WSTraverse(s, tmp));
              s = tmp;
              }
           tmp.SetY(e.Y());
           }
        else if (reg1 & Right || q1 == 4) {
           tmp.SetX(bb.CornerMax().X() + xtend);
           if (!kute::isEqual(s, tmp)) {
              ws.push_back(new WSTraverse(s, tmp));
              s = tmp;
              }
           tmp.SetY(e.Y());
           }
        else {
           if (e.X() < center.X()) tmp.SetX(bb.CornerMin().X() - xtend);
           else                    tmp.SetX(bb.CornerMax().X() + xtend);
           if (!kute::isEqual(s, tmp)) {
              ws.push_back(new WSTraverse(s, tmp));
              s = tmp;
              }
           tmp.SetY(bb.CornerMax().Y() + xtend);
           if (!kute::isEqual(s, tmp)) {
              ws.push_back(new WSTraverse(s, tmp));
              s = tmp;
              }
           tmp.SetX(e.X());
           }
        }
     else if (reg0 & Left) {
        int q1 = 0;

        if (!reg1) q1 = quadrant(e) + 1;
        if (reg1 & Top || q1 == 2) {
           tmp.SetY(bb.CornerMax().Y() + xtend);
           if (!kute::isEqual(s, tmp)) {
              ws.push_back(new WSTraverse(s, tmp));
              s = tmp;
              }
           tmp.SetX(e.X());
           }
        else if (reg1 & Bottom || q1 == 3) {
           tmp.SetY(bb.CornerMin().Y() - xtend);
           if (!kute::isEqual(s, tmp)) {
              ws.push_back(new WSTraverse(s, tmp));
              s = tmp;
              }
           tmp.SetX(e.X());
           }
        else {
           if (e.Y() < center.Y()) tmp.SetY(bb.CornerMin().Y() - xtend);
           else                    tmp.SetY(bb.CornerMax().Y() + xtend);
           if (!kute::isEqual(s, tmp)) {
              ws.push_back(new WSTraverse(s, tmp));
              s = tmp;
              }
           tmp.SetX(bb.CornerMax().X() + xtend);
           if (!kute::isEqual(s, tmp)) {
              ws.push_back(new WSTraverse(s, tmp));
              s = tmp;
              }
           tmp.SetY(e.Y());
           }
        }
     else if (reg0 & Right) {
        int q1 = 0;

        if (!reg1) q1 = quadrant(e) + 1;
        if (reg1 & Top || q1 == 1) {
           tmp.SetY(bb.CornerMax().Y() + xtend);
           if (!kute::isEqual(s, tmp)) {
              ws.push_back(new WSTraverse(s, tmp));
              s = tmp;
              }
           tmp.SetX(e.X());
           }
        else if (reg1 & Bottom || q1 == 4) {
           tmp.SetY(bb.CornerMin().Y() - xtend);
           if (!kute::isEqual(s, tmp)) {
              ws.push_back(new WSTraverse(s, tmp));
              s = tmp;
              }
           tmp.SetX(e.X());
           }
        else {
           if (e.Y() < center.Y()) tmp.SetY(bb.CornerMin().Y() - xtend);
           else                    tmp.SetY(bb.CornerMax().Y() + xtend);
           if (!kute::isEqual(s, tmp)) {
              ws.push_back(new WSTraverse(s, tmp));
              s = tmp;
              }
           tmp.SetX(bb.CornerMin().X() - xtend);
           if (!kute::isEqual(s, tmp)) {
              ws.push_back(new WSTraverse(s, tmp));
              s = tmp;
              }
           tmp.SetY(e.Y());
           }
        }
     if (!kute::isEqual(s, tmp)) ws.push_back(new WSTraverse(s, tmp));
     if (!kute::isEqual(tmp, e)) {
        if (!reg1) ws.push_back(new WSStraightMove(tmp, e));
        else       ws.push_back(new WSTraverse(tmp, e));
        }
     if (!kute::isEqual(e, to)) ws.push_back(new WSStraightMove(e, to));
     }
  return to;
  }


std::vector<Workstep*> PathBuilder::genPath4Pockets(Operation* op, const Bnd_Box& bb, const gp_Dir& baseNorm, const std::vector<std::vector<GOPocket*>>& pool, double curZ, double xtend) {
  double radius = 0;
  gp_Pnt center(bb.CornerMin().X() + (bb.CornerMax().X() - bb.CornerMin().X()) / 2
              , bb.CornerMin().Y() + (bb.CornerMax().Y() - bb.CornerMin().Y()) / 2
              , bb.CornerMin().Z() + (bb.CornerMax().Z() - bb.CornerMin().Z()) / 2);
  bool roundWorkPiece = Core().workData()->roundWorkPiece;
  std::vector<Workstep*> toolPath;
  int iMin = 0, iMax = 99;
  gp_Pnt  s(0, 0, 300);
  gp_Pnt  e = s, tmp;

  for (auto levelParts : pool) {
      int mxI = fmin(iMax, levelParts.size());

      for (int i=iMin; i < mxI; ++i) {
          GOPocket* p = levelParts.at(i);
          int mx = p->contours().size();

          p->dump();
          for (int j=0; j < mx; ++j) {
              GOContour* c   = p->contours().at(j);

              if (kute::isEqual(c->startPoint(), c->endPoint()))
                 c->changeStart2Close(e);
              c->simplify(curZ);
              s = c->startPoint();
              if (roundWorkPiece) genRoundInterMove(toolPath, e, s, bb, xtend + i + j);
              else                genInterMove(toolPath, e, s, center, bb, xtend + i + j);
//              drawDebugContour(op, c, curZ);
              processContour(toolPath, c);
              e = c->endPoint();
              }
          s = e;
          e.SetZ(bb.CornerMax().Z() + op->safeZ1());
          if (!kute::isEqual(s, e)) toolPath.push_back(new WSTraverse(s, e));
          }
      curZ -= op->cutDepth();
      }
  cleanup(toolPath);

  return toolPath;
  }


gp_Pnt PathBuilder::genRoundInterMove(std::vector<Workstep*>& ws, const gp_Pnt& from, const gp_Pnt& to, const Bnd_Box& bb, double xtend) {
  gp_Pnt c(bb.CornerMin().X() + (bb.CornerMax().X() - bb.CornerMin().X()) / 2
         , bb.CornerMin().Y() + (bb.CornerMax().Y() - bb.CornerMin().Y()) / 2
         , bb.CornerMin().Z() + (bb.CornerMax().Z() - bb.CornerMin().Z()) / 2);
  double r = (bb.CornerMax().X() - bb.CornerMin().X()) / 2;
  int reg0 = quadrant(from);
  int reg1 = quadrant(to);
  double   safeR = r + xtend;
  gp_Pnt   e=to, s=from, tmp=from;

  qDebug() << "need inter move from (" << reg0 << ")" << s.X() << " / " << s.Y() << " / " << s.Z()
                        << "   to   (" << reg1 << ")" << e.X() << " / " << e.Y() << " / " << e.Z();

  c.SetZ(to.Z());
  if (kute::isEqual(from.Z(), to.Z()) && s.Distance(c) < safeR) {      // ensure startpoint is in safe area
     double p0, p1;
     TopoDS_Edge        edge = BRepBuilderAPI_MakeEdge(c, s);
     Handle(Geom_Curve) c    = BRep_Tool::Curve(edge, p0, p1);

     s = c->Value(safeR);
     }
  if (e.Distance(c) < safeR) {      // ensure endpoint is in safe area
     double p0, p1;
     TopoDS_Edge        edge = BRepBuilderAPI_MakeEdge(c, e);
     Handle(Geom_Curve) c    = BRep_Tool::Curve(edge, p0, p1);

     e = c->Value(safeR);
     }
  if (!kute::isEqual(s, from)) {
     ws.push_back(new WSStraightMove(from, s));
     tmp = s;
     }

  if (tmp.Z() > e.Z()) {            // if startpoint has higher z, it might be safe,
     tmp = e;                       // so start direct move
     tmp.SetZ(s.Z());
     ws.push_back(new WSTraverse(s, tmp));
     s = tmp;                       // start point is now above endpoint
     }
  else if (reg0 == reg1) {          // both points in same region - direct move
     tmp = s;
     switch (reg0) {
       case 0:
            tmp.SetY(fmax(s.Y(), e.Y()));
            break;
       case 1:
            tmp.SetX(fmin(s.X(), e.X()));
            break;
       case 2:
            tmp.SetY(fmin(s.Y(), e.Y()));
            break;
       case 3:
            tmp.SetX(fmax(s.X(), e.X()));
            break;
       }
     }
  else {                            // both points are in different regions. Do safe moves
     tmp = s;
     switch (reg0) {
       case 1:
            if (!reg1) {
               tmp.SetY(bb.CornerMax().Y() + xtend);
               ws.push_back(new WSTraverse(s, tmp));
               s = tmp;

               tmp.SetX(e.X());
               }
            else {
               tmp.SetX(bb.CornerMin().X() - xtend);
               ws.push_back(new WSTraverse(s, tmp));
               s = tmp;

               tmp.SetY(bb.CornerMin().Y() - xtend);
               ws.push_back(new WSTraverse(s, tmp));
               s = tmp;

               tmp.SetX(e.X());
               }
            break;
       case 2:
            if (reg1 < 2) {
               tmp.SetX(bb.CornerMin().X() - xtend);
               ws.push_back(new WSTraverse(s, tmp));
               s = tmp;

               tmp.SetY(bb.CornerMax().Y() + xtend);
               ws.push_back(new WSTraverse(s, tmp));
               s = tmp;

               tmp.SetX(e.X());
               }
            else {
               tmp.SetY(bb.CornerMin().Y() - xtend);
               ws.push_back(new WSTraverse(s, tmp));
               s = tmp;

               tmp.SetX(e.X());
               }
            break;
       case 3:
            if (reg1 == 2) {
               tmp.SetY(bb.CornerMin().Y() - xtend);
               ws.push_back(new WSTraverse(s, tmp));
               s = tmp;

               tmp.SetX(e.X());
               }
            else {
               tmp.SetX(bb.CornerMax().X() + xtend);
               ws.push_back(new WSTraverse(s, tmp));
               s = tmp;

               tmp.SetY(bb.CornerMax().Y() + xtend);
               ws.push_back(new WSTraverse(s, tmp));
               s = tmp;

               tmp.SetX(e.X());
               }
       default:
            if (reg1 == 3) {
               tmp.SetX(bb.CornerMax().X() + xtend);
               ws.push_back(new WSTraverse(s, tmp));
               s = tmp;

               tmp.SetY(e.Y());
               }
            else {
               tmp.SetY(bb.CornerMax().Y() + xtend);
               ws.push_back(new WSTraverse(s, tmp));
               s = tmp;

               tmp.SetX(bb.CornerMin().X() - xtend);
               ws.push_back(new WSTraverse(s, tmp));
               s = tmp;

               tmp.SetY(e.Y());
               }
            break;
       }
     }
  if (!kute::isEqual(s, tmp)) {
     ws.push_back(new WSTraverse(s, tmp));
     }
  if (!kute::isEqual(tmp, e)) {
     ws.push_back(new WSTraverse(tmp, e));
     }
  if (!kute::isEqual(e, to)) {
     ws.push_back(new WSStraightMove(e, to));
     }
  return e;
  }


std::vector<Workstep*> PathBuilder::genRoundToolpaths(Operation* op, const std::vector<Handle(AIS_Shape)>& cutPlanes) {
  std::vector<Workstep*> workSteps;
  int                      mx = cutPlanes.size();
  gp_Pnt                   from, tmp, to, startXXPos;
  ToolEntry*               activeTool  = Core().toolListModel()->tool(Core().toolListModel()->findToolNum(op->toolNum()));
  TargetDefinition*        td  = op->targets.at(0);
  ContourTargetDefinition* ctd = dynamic_cast<ContourTargetDefinition*>(td);
  SweepTargetDefinition*   std = dynamic_cast<SweepTargetDefinition*>(td);
  double                   xtend       = activeTool->fluteDiameter() * 0.8;
  double                   topZ        = fmax(op->topZ(), op->upperZ());
  double                   safeZ0      = topZ + op->safeZ0();
//  double                   safeZ1      = topZ + op->safeZ1();
  double                   rMin        = ctd ? ctd->minRadius() + op->cutWidth() : 0;
  double                   rMax        = ctd ? ctd->maxRadius() - op->offset() - activeTool->fluteDiameter() / 2 : 0;
  double                   curR        = rMin;
  gp_Pnt                   pTC(0, 0, 300);
  gp_Pnt                   oC          = td->pos();
  gp_Pnt                   c           = oC;
  bool                     insideOut   = ctd && !op->isOutside();
  bool                     ccw         = op->direction() != 1;
  TopoDS_Edge              e;
  Handle(AIS_Shape)        path;

  from = pTC;
  if (!insideOut) {
     if (ctd) {
        rMin = ctd->minRadius() + op->offset() + activeTool->fluteDiameter() / 2;
        rMax = ctd->maxRadius();
        // on contour processing we're bound to circles
        // so we approach upside down.
        to = gp_Pnt(oC.X(), oC.Y() + curR, pTC.Z());
        }
     else if (std) {
        rMax = (op->wpBounds.CornerMax().X() - op->wpBounds.CornerMin().X()) / 2;
        rMax += activeTool->fluteDiameter() / 2 - op->cutWidth();
        // with sweep we're NOT bound to circles, so we can approach
        // from outside of circle, which saves cutting time ...
        to = gp_Pnt(oC.X(), oC.Y() + rMax + xtend, pTC.Z());
        }
     curR = rMax;
     ccw  = !ccw;
     }
  else {
     to = gp_Pnt(oC.X(), oC.Y() + curR, pTC.Z());
     }
  workSteps.push_back(new WSTraverse(from, to));
  from = to;
  to.SetZ(safeZ0);
  if (std) {
     // on sweeps we're outside of workpiece, so let's advance to cutplane height
     Handle(AIS_Shape) s  = cutPlanes.at(0);
     Bnd_Box           bb = s->BoundingBox(); bb.SetGap(0);

     to.SetZ(bb.CornerMin().Z());
     }
  workSteps.push_back(new WSTraverse(from, to));
  c    = to;
  from = to;

  if (std) {
     // lead in for sweep operations
     to.SetY(oC.Y() + curR);
     c.SetY(oC.Y() + curR + xtend / 2);

     // lead in turns opposite direction
     workSteps.push_back(new WSArc(from, to, c, !ccw));
     from = to;
     }
  qDebug() << "rMin:" << rMin << "rMax:" << rMax << "topZ:" << topZ << "lastZ:" << op->lowerZ();

  for (int i=0; i < mx; ++i, curR = insideOut ? rMin : rMax) {
      Handle(AIS_Shape) s      = cutPlanes.at(i);
      Bnd_Box           bb     = s->BoundingBox(); bb.SetGap(0);
      double            nextZ  = bb.CornerMin().Z() + (topZ - bb.CornerMin().Z()) / 2;

      qDebug() << "round cut plane: "
               << bb.CornerMin().X() << "/" << bb.CornerMin().Y() << "/" << bb.CornerMin().Z()
               << "\tto\t"
               << bb.CornerMax().X() << "/" << bb.CornerMax().Y() << "/" << bb.CornerMax().Z();

      if (from.Y() > oC.Y()) {
         to = gp_Pnt(oC.X(), oC.Y() - curR, nextZ);
         c.SetY(from.Y() - (from.Y() - to.Y()) / 2);
//         c.SetZ(nextZ);
         workSteps.push_back(new WSArc(from, to, c, ccw));
//         genArc(from, to, c, ccw);

         from = to;
         }
      to = gp_Pnt(oC.X(), oC.Y() + curR, bb.CornerMin().Z());
      c.SetY(from.Y() - (from.Y() - to.Y()) / 2);
      c.SetZ(from.Z());
      workSteps.push_back(new WSArc(from, to, c, ccw));
//      genArc(from, to, c, ccw);

      from = to;
      to   = gp_Pnt(oC.X(), oC.Y() - curR, bb.CornerMin().Z());
      c.SetY(from.Y() - (from.Y() - to.Y()) / 2);
      c.SetZ(from.Z());
      workSteps.push_back(new WSArc(from, to, c, ccw));
//      genArc(from, to, c, ccw);

      from = to;
      to   = gp_Pnt(oC.X(), oC.Y() + curR, bb.CornerMin().Z());
      c.SetY(from.Y() - (from.Y() - to.Y()) / 2);
      workSteps.push_back(new WSArc(from, to, c, ccw));
//      genArc(from, to, c, ccw);

      from = to;
      if (insideOut) {
         while ((curR += op->cutWidth()) < rMax) {
               to = gp_Pnt(oC.X(), oC.Y() - curR, bb.CornerMin().Z());
               c.SetY(from.Y() - (from.Y() - to.Y()) / 2);
               workSteps.push_back(new WSArc(from, to, c, ccw));
//               genArc(from, to, c, ccw);

               from = to;
   //            if ((curR += op->cutWidth()) > rMax) break;
               to   = gp_Pnt(oC.X(), oC.Y() + curR, bb.CornerMin().Z());
               c.SetY(from.Y() - (from.Y() - to.Y()) / 2);
               workSteps.push_back(new WSArc(from, to, c, ccw));
//               genArc(from, to, c, ccw);
               from = to;
               }
         }
      else {
         while ((curR -= op->cutWidth()) > rMin) {
               to = gp_Pnt(oC.X(), oC.Y() - curR, bb.CornerMin().Z());
               c.SetY(from.Y() - (from.Y() - to.Y()) / 2);
               workSteps.push_back(new WSArc(from, to, c, ccw));
//               genArc(from, to, c, ccw);

               from = to;
   //            if ((curR += op->cutWidth()) > rMax) break;
               to   = gp_Pnt(oC.X(), oC.Y() + curR, bb.CornerMin().Z());
               c.SetY(from.Y() - (from.Y() - to.Y()) / 2);
               workSteps.push_back(new WSArc(from, to, c, ccw));
//               genArc(from, to, c, ccw);
               from = to;
               }
         }
      curR = insideOut ? rMax : rMin;
      to = gp_Pnt(oC.X(), oC.Y() - curR, bb.CornerMin().Z());
      c.SetY(from.Y() - (from.Y() - to.Y()) / 2);
      workSteps.push_back(new WSArc(from, to, c, ccw));
//      genArc(from, to, c, ccw);

      from = to;
      to   = gp_Pnt(oC.X(), oC.Y() + curR, bb.CornerMin().Z());
      c.SetY(from.Y() - (from.Y() - to.Y()) / 2);
      workSteps.push_back(new WSArc(from, to, c, ccw));
//      genArc(from, to, c, ccw);

      from = to;
      to = gp_Pnt(oC.X(), oC.Y() - curR, bb.CornerMin().Z());
      c.SetY(from.Y() - (from.Y() - to.Y()) / 2);
      workSteps.push_back(new WSArc(from, to, c, ccw));
//      genArc(from, to, c, ccw);
      from = to;
      }
  to.SetZ(pTC.Z());
  workSteps.push_back(new WSTraverse(from, to));

  return workSteps;
  }


std::vector<Workstep*> PathBuilder::genToolPath(Operation* op, Handle(AIS_Shape) cutPart, bool wantPockets) {
  TargetDefinition*        td  = op->targets.at(0);
  SweepTargetDefinition*   std = dynamic_cast<SweepTargetDefinition*>(td);
  ContourTargetDefinition* ctd = dynamic_cast<ContourTargetDefinition*>(td);
  std::vector<Workstep*>   toolPath;

  if (!td) return toolPath;
//  gp_Pnt center(op->wpBounds.CornerMin().X() + (op->wpBounds.CornerMax().X() - op->wpBounds.CornerMin().X()) / 2
//              , op->wpBounds.CornerMin().Y() + (op->wpBounds.CornerMax().Y() - op->wpBounds.CornerMin().Y()) / 2
//              , op->wpBounds.CornerMin().Z() + (op->wpBounds.CornerMax().Z() - op->wpBounds.CornerMin().Z()) / 2);
  gp_Pnt     center  = td->pos();
  GOContour* contour = td->contour();

  if (!contour) return toolPath;
  std::vector<std::vector<std::vector<GOContour*>>> clippedParts;
  std::vector<Handle(AIS_Shape)> cutPlanes;
  ToolEntry*             activeTool    = Core().toolListModel()->tool(Core().toolListModel()->findToolNum(op->toolNum()));
  double                 xtend         = activeTool->fluteDiameter() * 0.8;
  double                 firstOffset   = op->offset() + activeTool->fluteDiameter() / 2;
  Bnd_Box                bb            = cutPart->BoundingBox();
  bool                   curveIsBorder = std ? std->isBaseBorder() : false;
  double                 lastZ         = fmax(bb.CornerMax().Z() - activeTool->cuttingDepth()
                                            , op->finalDepth() + op->offset());
  int                    steps         = (bb.CornerMax().Z() - lastZ) / op->cutDepth();
  double                 startZ        = lastZ + steps * op->cutDepth();
  double                 curZ          = startZ;
  int                    n=0;

  if (std && curveIsBorder) firstOffset += abs(calcAdditionalOffset(std, contour));
  contour->simplify(curZ);
  qDebug() << "cutting contour:" << contour->toString();
  qDebug() << "center of workpiece:" << center.X() << " / " << center.Y();

  // prepare raw toolpaths
  lastZ -=  2 * kute::MinDelta;
  while (curZ > lastZ) {
        qDebug() << "cut depth is" << curZ;
        std::vector<std::vector<GOContour*>> levelContours = processCurve(op, contour, curveIsBorder, center, /* xtend, */ firstOffset, curZ);

        if (levelContours.size()) clippedParts.push_back(levelContours);
        gp_Pln pln({0, 0, curZ}, {0, 0, 1});
        BRepBuilderAPI_MakeFace mf(pln, -500, 500, -500, 500);

        cutPlanes.push_back(new AIS_Shape(Core().helper3D()->intersect(op->cutPart->Shape(), mf.Shape())));

        curZ -= op->cutDepth();
        }
  //TODO: show clippedParts without additional paths!
//  dump(clippedParts);
//  std::vector<Workstep*>   tP0 = genBasicPath(clippedParts.at(0));

//  cleanup(tP0);
//  toolPath = genFlatPaths(op, cutPlanes, clippedParts, curZ, xtend, 1);
//  cleanup(toolPath);
//  toolPath.insert(toolPath.begin(), tP0.begin(), tP0.end());
//  cleanup(toolPath);

//  return toolPath;

  qDebug() << "have" << clippedParts.size() << "cut stages\n";

  if (!clippedParts.size()) return toolPath;

  qDebug() << "center: " << center.X() << " / " << center.Y(); // << "   extend: " << xtend;
  qDebug() << "workpiece: " << op->wpBounds.CornerMin().X() << " / " << op->wpBounds.CornerMin().Y()
           << "   to   "    << op->wpBounds.CornerMax().X() << " / " << op->wpBounds.CornerMax().Y();
  dump(clippedParts);
  curZ = startZ;

  if (wantPockets) {
     std::vector<std::vector<GOPocket*>> pool = splitCurves(op, clippedParts);
     gp_Dir workDir = std ? std->baseDir() : gp_Dir(0, 0, 1);

     toolPath = genPath4Pockets(op, op->wpBounds, workDir, pool, curZ, xtend);
     }
  else {
     toolPath = genFlatPaths(op, cutPlanes, clippedParts, curZ, xtend);
     }
//  cleanup(toolPath);

  return toolPath;
  }


void PathBuilder::drawDebugContour(Operation* op, GOContour* c, double z) {
//  qDebug() << "process debug contour" << c->toString();
  std::vector<GraphicObject*> segments = c->segments();
  int mx = segments.size() - 1;

  for (int i=0; i <= mx; ++i) {
      Handle(AIS_Shape) s = segments.at(i)->toShape();

      if (!i)           s->SetColor(Quantity_NOC_GREEN);
      else if (i == 1)  s->SetColor(Quantity_NOC_YELLOW);
      else if (i == mx) s->SetColor(Quantity_NOC_RED);
      else              s->SetColor(Quantity_NOC_BLUE);
      s->SetWidth(3);

//      Core().view3D()->showShape(s);
      op->cShapes.push_back(s);
      }
  }


gp_Pnt PathBuilder::processContour(std::vector<Workstep*>& tp, GOContour* c) {
  qDebug() << "process contour" << c->toString();
  Workstep* ws = nullptr;

  for (auto& go : c->segments()) {
      switch (go->type()) {
        case GTLine:
             ws = new WSStraightMove(go->startPoint(), go->endPoint());
             ws->setColor(Quantity_NOC_PURPLE1);
             tp.push_back(ws);
             break;
        case GTCircle: {
             GOCircle* gc = dynamic_cast<GOCircle*>(go);

             ws = new WSArc(go->startPoint(), go->endPoint(), gc->center(), gc->isCCW());
             ws->setColor(Quantity_NOC_PURPLE1);
             tp.push_back(ws);
             } break;
        default:
             throw std::domain_error(QString("unsupported graphic-type %1").arg(go->type()).toStdString());
             break;
        }
      }
  return ws->endPos();
  }


// all contiguous segments form a contour (instance of GOContour).
// A cutted offset curve may lead to several subcontours (vector of GOContour)
// processCurve processes all contour(-segments) of same z-level
std::vector<std::vector<GOContour*>> PathBuilder::processCurve(Operation* op, GOContour* curve, bool curveIsBorder, const gp_Pnt& center, /* double xtend, */ double firstOffset, double curZ) {
  BRepOffsetAPI_MakeOffset              offMaker(TopoDS::Wire(curve->toWire(curZ)));
  TopoDS_Shape                          rawPath;
  Bnd_Box                               bbCut = op->cutPart->BoundingBox();
  std::vector<std::vector<GOContour*>>  levelContours;
  bool                                  debug = false;

  for (int i=0; true; ++i) {
      std::vector<GOContour*> pathParts;
      double offset = firstOffset + i * op->cutWidth();

      qDebug() << "create offset contour with offset:" << offset;

      offMaker.Perform(offset, 0);
      if (offMaker.IsDone()) {
         Handle(TopTools_HSequenceOfShape) edgePool = new TopTools_HSequenceOfShape;
         TopoDS_Shape                      offWire  = Core().helper3D()->allEdgesWithin(offMaker.Shape(), edgePool);
         GOContour*                        path     = new GOContour(center, i);
         GraphicObject*                    goSeg    = nullptr;

         if (debug) {
            Handle(AIS_Shape) aw = new AIS_Shape(offWire);

            aw->SetColor(Quantity_NOC_ORANGE2);
            aw->SetWidth(2);
            op->cShapes.push_back(aw);
            if (i > 3) break;
            continue;
            }

         if (!i && !curveIsBorder && curve->isClosed()) {
            std::vector<TopoDS_Edge> segments = Core().helper3D()->allEdgesWithin(offWire);

            for (auto s : segments) {
                goSeg = GOContour::occ2GO(s);

                if (kute::isEqual(goSeg->startPoint(), goSeg->endPoint())) {
                   delete goSeg;
                   continue;
                   }
                path->add(goSeg);   // keep all segments for last workpath
                }
//            stripPath(path, curve);
            path->extendBy(5);
            path->simplify(curZ);
            pathParts.push_back(path);
            }
         else {
            rawPath = BRepAlgoAPI_Common(offWire, op->cutPart->Shape());
            std::vector<TopoDS_Edge> segments = Core().helper3D()->allEdgesWithin(rawPath);

            if (!segments.size()) break;
            for (int j=0; j < segments.size(); ++j) {
                goSeg = GOContour::occ2GO(segments.at(j));

                if (kute::isEqual(goSeg->startPoint(), goSeg->endPoint())) {
                   delete goSeg;
                   continue;
                   }
                if (path->add(goSeg)) continue;
                if (path->size()) {
                   path->extendBy(5);
                   path->simplify(curZ);
                   pathParts.push_back(path);
                   path = new GOContour(center, i);
                   if (goSeg) path->add(goSeg);
                   }
                }
            if (path->size()) {
               path->extendBy(5);
               path->simplify(curZ);
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


int PathBuilder::quadrant(const gp_Pnt &p, const gp_Pnt& center) const {
  if (p.X() < center.X()) {
     if (p.Y() < center.Y()) return 2;
     return 1;
     }
  if (p.Y() < center.Y()) return 3;

  return 0;
  }


int PathBuilder::region(const gp_Pnt& p, const Bnd_Box& bb) const {
  int rv = Inside;

  if      (p.X() < bb.CornerMin().X()) rv |= Left;
  else if (p.X() > bb.CornerMax().X()) rv |= Right;
  if      (p.Y() < bb.CornerMin().Y()) rv |= Bottom;
  else if (p.Y() > bb.CornerMax().Y()) rv |= Top;

  return rv;
  }


std::vector<std::vector<GOPocket*>> PathBuilder::splitCurves(const Operation* op, const std::vector<std::vector<std::vector<GOContour*>>>& pool) {
  std::vector<std::vector<GOPocket*>> levels;
  auto& c = pool.at(0);
  int nL = 0; //, nC = 0, nCC = 0;

  qDebug() << "pool has" << pool.size() << "Z-levels";
  for (auto lp : pool) {
      std::vector<GOPocket*> pockets;
      double size  = 0;
      int count    = 0;
      int maxCount = 0;

      qDebug() << "\n" << ++nL << "level has parts of" << lp.size() << "offset contours";

      for (int i=0; i < lp.size(); ++i) {
          auto& cp = lp.at(i);

          for (int j=0; j < cp.size(); ++j) {
              auto&  c = cp.at(j);
              double s = c->a0() - c->a1();

              if (s > size) {
                 size = s;
                 count = i;
                 }
              }
          }
      qDebug() << "biggest contour parts at level #" << count << "   found with size" << size;
      auto cp = lp.at(count);

      // first create all pockets necessary
      for (auto c : cp) {
          double a0 = c->a0();
          double a1 = c->a1();

          qDebug() << "create pocket for contour from" << c->startPoint().X() << " / " << c->startPoint().Y()
                   << "   to   "   << c->endPoint().X() << " / " << c->endPoint().Y();
          qDebug() << "   check: a0 == " << a0 << "   a1 == " << a1;

          if (!op->direction()) {
             if (a0 < a1) pockets.push_back(new GOPocket(c->endPoint(), c->startPoint(), c->centerPoint()));
             else         pockets.push_back(new GOPocket(c->startPoint(), c->endPoint(), c->centerPoint()));
             }
          else {
             if (a0 > a1) pockets.push_back(new GOPocket(c->endPoint(), c->startPoint(), c->centerPoint()));
             else         pockets.push_back(new GOPocket(c->startPoint(), c->endPoint(), c->centerPoint()));
             }
          }

      // process levelParts from out to in and fill created pockets
      for (auto i = lp.rbegin(); i != lp.rend(); ++i) {
          auto cp = *i;

          for (int n=0; n < cp.size(); ++n) {
              auto   c     = cp.at(n);
              double ac0   = c->a0();
              double ac1   = c->a1();
              bool   match = false;

              for (int j=0; j < pockets.size(); ++j) {
                  double ap0 = pockets.at(j)->a0();
                  double ap1 = pockets.at(j)->a1();

                  if (ac0 <= ap0 && ac0 >= ap1 && ac1 <= ap0 && ac1 >= ap1) {
                     pockets.at(j)->add(c);
                     match = true;
                     break;
                     }
                  else qDebug() << "no match for pocket (" << ap0 << " <> " << ap1 << ")";
                  }
              if (!match) {
                 qDebug() << "OUPS - lost contour (" << ac0 << " <> " << ac1 << ")";
                 }
              }
          }
      // pockets with lowest item count should be first to be processed
      std::sort(pockets.begin(), pockets.end(), compPocket);
      levels.push_back(pockets);
      }
  return levels;
  }


// TODO: rebuild!
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

                if (d0.IsEqual(d1, kute::MinDelta)) {
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


const int PathBuilder::Inside = 0;
const int PathBuilder::Left   = 1;
const int PathBuilder::Right  = 2;
const int PathBuilder::Bottom = 4;
const int PathBuilder::Top    = 8;
