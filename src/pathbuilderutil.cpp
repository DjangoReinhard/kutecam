/* 
 * **************************************************************************
 * 
 *  file:       pathbuilderutil.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    26.7.2022 by Django Reinhard
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
#include "pathbuilderutil.h"
#include "gocontour.h"
#include "graphicobject.h"
#include "gocircle.h"
#include "goline.h"
#include "pocketpathbuilder.h"
#include "profitmillingbuilder.h"
#include "sweeppathbuilder.h"
#include "wsarc.h"
#include "wsstraightmove.h"
#include "wstraverse.h"
#include "kuteCAM.h"
#include <Bnd_Box.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS_Edge.hxx>
#include <QDebug>


PathBuilderUtil::PathBuilderUtil()
 : ppb(nullptr)
 , pmb(nullptr)
 , spb(nullptr) {
  }


void PathBuilderUtil::cleanup(std::vector<Workstep*>& tp) {
  for (int i=0; i < tp.size(); ++i) {
      Workstep* ws = tp.at(i);

      if (kute::isEqual(ws->startPos(), ws->endPos())) {
         tp.erase(tp.begin() + i--);
         }
      }
  }


//TODO: block region
gp_Pnt PathBuilderUtil::genInterMove(std::vector<Workstep*>& ws, const gp_Pnt& from, const gp_Pnt& to, const gp_Pnt& center, const Bnd_Box& bb, double xtend) {
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


gp_Pnt PathBuilderUtil::genRoundInterMove(std::vector<Workstep*>& ws, const gp_Pnt& from, const gp_Pnt& to, const Bnd_Box& bb, double xtend) {
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


PocketPathBuilder* PathBuilderUtil::pocketPathBuilder() {
  if (!ppb) ppb = new PocketPathBuilder(this);
  return ppb;
  }


ProfitMillingBuilder* PathBuilderUtil::profitMillingBuilder() {
  if (!pmb) pmb = new ProfitMillingBuilder(this);
  return pmb;
  }


gp_Pnt PathBuilderUtil::processContour(std::vector<Workstep*>& tp, GOContour* c) {
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
  return ws ? ws->endPos() : gp_Pnt();
  }


int PathBuilderUtil::quadrant(const gp_Pnt &p, const gp_Pnt& center) const {
  if (p.X() < center.X()) {
     if (p.Y() < center.Y()) return 2;
     return 1;
     }
  if (p.Y() < center.Y()) return 3;

  return 0;
  }


int PathBuilderUtil::region(const gp_Pnt& p, const Bnd_Box& bb) const {
  int rv = Inside;

  if      (p.X() < bb.CornerMin().X()) rv |= Left;
  else if (p.X() > bb.CornerMax().X()) rv |= Right;
  if      (p.Y() < bb.CornerMin().Y()) rv |= Bottom;
  else if (p.Y() > bb.CornerMax().Y()) rv |= Top;

  return rv;
  }


SweepPathBuilder* PathBuilderUtil::sweepPathBuilder() {
  if (!spb) spb = new SweepPathBuilder(this);
  return spb;
  }

const int PathBuilderUtil::Inside = 0;
const int PathBuilderUtil::Left   = 1;
const int PathBuilderUtil::Right  = 2;
const int PathBuilderUtil::Bottom = 4;
const int PathBuilderUtil::Top    = 8;
