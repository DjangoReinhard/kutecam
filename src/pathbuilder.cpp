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


// return  0 - direct move ok
//        -1 - move toward negative X
//         1 - move toward positive X
//        -2 - move toward negative Y
//         2 - move toward positive Y
int PathBuilder::calcMainDir(const gp_Pnt& s, const gp_Pnt& e, const Bnd_Box& bb, double extend) {
  double d0 = abs(s.X() - e.X());
  double d1 = abs(s.Y() - e.Y());
  double dX0 = abs(bb.CornerMax().X() - s.X());
  double dX1 = abs(s.X() - bb.CornerMin().X());
  double dY0 = abs(bb.CornerMax().Y() - s.Y());
  double dY1 = abs(s.Y() - bb.CornerMin().Y());

  if (s.X() < bb.CornerMin().X()) {
     if (e.X() < bb.CornerMin().X()) {
        if (bb.CornerMin().X() - s.X() > extend && bb.CornerMin().X() - e.X() > extend) return 0;
        if (d0 < d1)
           return dX0 < dX1 ? 1 : -1;
        return dY0 < dY1 ? 2 : -2;
        }
     }
  if (s.X() > bb.CornerMax().X()) {
     if (e.X() > bb.CornerMax().X()) {
        if (s.X() - bb.CornerMax().X() > extend && e.X() - bb.CornerMax().X() > extend) return 0;
        if (d0 < d1)
           return dX0 < dX1 ? 1 : -1;
        return dY0 < dY1 ? 2 : -2;
        }
     }
  if (s.Y() < bb.CornerMin().Y()) {
     if (e.Y() < bb.CornerMin().Y()) {
        if (bb.CornerMin().Y() - s.Y() > extend && bb.CornerMin().Y() - e.Y() > extend) return 0;
        if (d0 < d1)
           return dX0 < dX1 ? 1 : -1;
        return dY0 < dY1 ? 2 : -2;
        }
     }
  if (s.Y() > bb.CornerMax().Y()) {
     if (e.Y() > bb.CornerMax().Y()) {
        if (s.Y() - bb.CornerMax().Y() > extend && e.Y() - bb.CornerMax().Y() > extend) return 0;
        if (d0 < d1)
           return dX0 < dX1 ? 1 : -1;
        return dY0 < dY1 ? 2 : -2;
        }
     }
  if (d0 < d1)
     return dX0 < dX1 ? 1 : -1;
  return dY0 < dY1 ? 2 : -2;
  }


int PathBuilder::calcRegion(const gp_Pnt& p, const Bnd_Box& bb, double xtend) {
  if (p.X() < bb.CornerMin().X()) {
     if (bb.CornerMin().X() - p.X() < xtend) return -1;
     return 0;
     }
  if (p.X() > bb.CornerMax().X()) {
     if (p.X() - bb.CornerMax().X() < xtend) return 1;
     return 0;
     }
  if (p.Y() < bb.CornerMin().Y()) {
     if (bb.CornerMin().Y() - p.Y() < xtend) return -2;
     return 0;
     }
  if (p.Y() > bb.CornerMax().Y()) {
     if (p.Y() - bb.CornerMax().Y() < xtend) return 2;
     return 0;
     }
  return 0;
  }


int PathBuilder::calcSafeOffsets(double& safeX, double& safeY, const gp_Dir& baseNorm, Operation* op, double xtend) {
  int    dominantAxis = kute::getDominantAxis(baseNorm);

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


//  dump layer # 1
//  dump path # 1
//  contour # 1    from  68  /  -44.1    to    68  /  -44.1
//     check - a0: -0.641872  <> a1: -0.641872
//  "3;68.0000/-44.1000/55.1000;68.0000/-44.1000/55.1000;9.0000/0.0000/-20.0000|1;68.0000/-44.1000/55.1000;-16.0000/-44.1000/55.1000|2;-16.0000/-44.1000/55.1000;-22.1000/-38.0000/55.1000;-16.0000/-38.0000/55.1000;0.0000/0.0000/-1.0000;6.1000|1;-22.1000/-38.0000/55.1000;-22.1000/33.0000/55.1000|2;-22.1000/33.0000/55.1000;-16.0000/39.1000/55.1000;-16.0000/33.0000/55.1000;0.0000/0.0000/-1.0000;6.1000|1;-16.0000/39.1000/55.1000;68.0000/39.1000/55.1000|2;68.0000/39.1000/55.1000;74.1000/33.0000/55.1000;68.0000/33.0000/55.1000;0.0000/0.0000/-1.0000;6.1000|1;74.1000/33.0000/55.1000;74.1000/-38.0000/55.1000|2;74.1000/-38.0000/55.1000;68.0000/-44.1000/55.1000;68.0000/-38.0000/55.1000;0.0000/0.0000/-1.0000;6.1000"
//  dump path # 2
//  contour # 1    from  80.2857  /  -29.9731    to    80.2857  /  24.9731
//     check - a0: -0.398023  <> a1: 0.336963
//  "3;80.2857/-29.9731/55.1000;80.2857/24.9731/55.1000;9.0000/0.0000/-20.0000|1;80.2857/-29.9731/55.1000;79.0000/-39.4866/55.1000|2;79.0000/-39.4866/55.1000;68.0000/-49.1000/55.1000;68.0000/-38.0000/55.1000;0.0000/0.0000/-1.0000;11.1000|1;68.0000/-49.1000/55.1000;-16.0000/-49.1000/55.1000|2;-16.0000/-49.1000/55.1000;-27.1000/-38.0000/55.1000;-16.0000/-38.0000/55.1000;0.0000/0.0000/-1.0000;11.1000|1;-27.1000/-38.0000/55.1000;-27.1000/33.0000/55.1000|2;-27.1000/33.0000/55.1000;-16.0000/44.1000/55.1000;-16.0000/33.0000/55.1000;0.0000/0.0000/-1.0000;11.1000|1;-16.0000/44.1000/55.1000;68.0000/44.1000/55.1000|2;68.0000/44.1000/55.1000;79.0000/34.4866/55.1000;68.0000/33.0000/55.1000;0.0000/0.0000/-1.0000;11.1000|1;79.0000/34.4866/55.1000;80.2857/24.9731/55.1000"
//  dump path # 3
//  contour # 1    from  86.01  /  -43.1973    to    86.0099  /  38.1973
//     check - a0: -0.511197  <> a1: 0.460446
//  "3;86.0100/-43.1973/55.1000;86.0099/38.1973/55.1000;9.0000/0.0000/-20.0000|1;86.0100/-43.1973/55.1000;79.0000/-49.7563/55.1000|2;79.0000/-49.7563/55.1000;68.0000/-54.1000/55.1000;68.0000/-38.0000/55.1000;0.0000/0.0000/-1.0000;16.1000|1;68.0000/-54.1000/55.1000;-16.0000/-54.1000/55.1000|2;-16.0000/-54.1000/55.1000;-32.1000/-38.0000/55.1000;-16.0000/-38.0000/55.1000;0.0000/0.0000/-1.0000;16.1000|1;-32.1000/-38.0000/55.1000;-32.1000/33.0000/55.1000|2;-32.1000/33.0000/55.1000;-16.0000/49.1000/55.1000;-16.0000/33.0000/55.1000;0.0000/0.0000/-1.0000;16.1000|1;-16.0000/49.1000/55.1000;68.0000/49.1000/55.1000|2;68.0000/49.1000/55.1000;79.0000/44.7563/55.1000;68.0000/33.0000/55.1000;0.0000/0.0000/-1.0000;16.1000|1;79.0000/44.7563/55.1000;86.0099/38.1973/55.1000"
//  dump path # 4
//  contour # 1    from  87.1922  /  46.0011    to    -20.7638  /  -60.6865
//     check - a0: 0.531778  <> a1: -2.02678
//  "3;87.1922/46.0011/55.1000;-20.7638/-60.6865/55.1000;9.0000/0.0000/-20.0000|1;87.1922/46.0011/55.1000;79.0000/51.0058/55.1000|2;79.0000/51.0058/55.1000;68.0000/54.1000/55.1000;68.0000/33.0000/55.1000;0.0000/0.0000/1.0000;21.1000|1;68.0000/54.1000/55.1000;-16.0000/54.1000/55.1000|2;-16.0000/54.1000/55.1000;-37.1000/33.0000/55.1000;-16.0000/33.0000/55.1000;0.0000/0.0000/1.0000;21.1000|1;-37.1000/33.0000/55.1000;-37.1000/-38.0000/55.1000|2;-37.1000/-38.0000/55.1000;-28.4984/-55.0000/55.1000;-16.0000/-38.0000/55.1000;0.0000/0.0000/1.0000;21.1000|1;-28.4984/-55.0000/55.1000;-20.7638/-60.6865/55.1000"
//  dump path # 5
//  contour # 1    from  -21.9512  /  60.1653    to    -29.5514  /  -62.2843
//     check - a0: 2.04593  <> a1: -2.12504
//  "3;-21.9512/60.1653/55.1000;-29.5514/-62.2843/55.1000;9.0000/0.0000/-20.0000|1;-21.9512/60.1653/55.1000;-30.0431/55.0000/55.1000|2;-30.0431/55.0000/55.1000;-42.1000/33.0000/55.1000;-16.0000/33.0000/55.1000;0.0000/0.0000/1.0000;26.1000|1;-42.1000/33.0000/55.1000;-42.1000/-38.0000/55.1000|2;-42.1000/-38.0000/55.1000;-35.8043/-55.0000/55.1000;-16.0000/-38.0000/55.1000;0.0000/0.0000/1.0000;26.1000|1;-35.8043/-55.0000/55.1000;-29.5514/-62.2843/55.1000"
//  dump path # 6
//  contour # 1    from  -31.191  /  61.7854    to    -36.7949  /  -63.0388
//     check - a0: 2.14752  <> a1: -2.19906
//  "3;-31.1910/61.7854/55.1000;-36.7949/-63.0388/55.1000;9.0000/0.0000/-20.0000|1;-31.1910/61.7854/55.1000;-37.9820/55.0000/55.1000|2;-37.9820/55.0000/55.1000;-47.1000/33.0000/55.1000;-16.0000/33.0000/55.1000;0.0000/0.0000/1.0000;31.1000|1;-47.1000/33.0000/55.1000;-47.1000/-38.0000/55.1000|2;-47.1000/-38.0000/55.1000;-42.0425/-55.0000/55.1000;-16.0000/-38.0000/55.1000;0.0000/0.0000/1.0000;31.1000|1;-42.0425/-55.0000/55.1000;-36.7949/-63.0388/55.1000"
//  dump path # 7
//  contour # 1    from  -38.7714  /  62.6113    to    -43.3259  /  -63.4689
//     check - a0: 2.22256  <> a1: -2.26026
//  "3;-38.7714/62.6113/55.1000;-43.3259/-63.4689/55.1000;9.0000/0.0000/-20.0000|1;-38.7714/62.6113/55.1000;-44.6218/55.0000/55.1000|2;-44.6218/55.0000/55.1000;-52.1000/33.0000/55.1000;-16.0000/33.0000/55.1000;0.0000/0.0000/1.0000;36.1000|1;-52.1000/33.0000/55.1000;-52.1000/-38.0000/55.1000|2;-52.1000/-38.0000/55.1000;-47.8467/-55.0000/55.1000;-16.0000/-38.0000/55.1000;0.0000/0.0000/1.0000;36.1000|1;-47.8467/-55.0000/55.1000;-43.3259/-63.4689/55.1000"
//  dump path # 8
//  contour # 1    from  -45.5775  /  63.1089    to    -49.4486  /  -63.7403
//     check - a0: 2.28383  <> a1: -2.31291
//  "3;-45.5775/63.1089/55.1000;-49.4486/-63.7403/55.1000;9.0000/0.0000/-20.0000|1;-45.5775/63.1089/55.1000;-50.7161/55.0000/55.1000|2;-50.7161/55.0000/55.1000;-57.1000/33.0000/55.1000;-16.0000/33.0000/55.1000;0.0000/0.0000/1.0000;41.1000|1;-57.1000/33.0000/55.1000;-57.1000/-38.0000/55.1000|2;-57.1000/-38.0000/55.1000;-53.4194/-55.0000/55.1000;-16.0000/-38.0001/55.1000;0.0000/0.0000/1.0000;41.1000|1;-53.4194/-55.0000/55.1000;-49.4486/-63.7403/55.1000"
//  dump path # 9
//  contour # 1    from  -63.0846  /  -38.6396    to    -55.3109  /  -63.9234
//     check - a0: -2.64954  <> a1: -2.35922
//  "3;-63.0846/-38.6396/55.1000;-55.3109/-63.9234/55.1000;9.0000/0.0000/-20.0000|1;-63.0846/-38.6396/55.1000;-61.0000/-48.0105/55.1000|2;-61.0000/-48.0105/55.1000;-58.8510/-55.0000/55.1000;-16.0006/-38.0003/55.1000;0.0000/0.0000/1.0000;46.0994|1;-58.8510/-55.0000/55.1000;-55.3109/-63.9234/55.1000"
//  contour # 2    from  -51.9305  /  63.4363    to    -63.0846  /  33.6396
//     check - a0: 2.33605  <> a1: 2.70496
//  "3;-51.9305/63.4363/55.1000;-63.0846/33.6396/55.1000;9.0000/0.0000/-20.0000|1;-51.9305/63.4363/55.1000;-56.5119/55.0000/55.1000|2;-56.5119/55.0000/55.1000;-61.0000/43.0105/55.1000;-16.0000/32.9999/55.1000;0.0000/0.0000/1.0000;46.1000|1;-61.0000/43.0105/55.1000;-63.0846/33.6396/55.1000"
//  pool has 1 Z-levels

//   1 level has parts of 9 offset contours
//  level 8 has 2 pockets
//  create pocket for contour from -63.0846  /  -38.6396    to    -55.3109  /  -63.9234
//     check: a0 ==  3.63365    a1 ==  3.92397
//  create pocket for contour from -51.9305  /  63.4363    to    -63.0846  /  33.6396
//     check: a0 ==  2.33605    a1 ==  2.70496
//  need fast move from -51.9305  /  63.4363  /  55.1    to    -55.3109  /  -63.9234  /  55.1
//  dominantAxis is: -1
//  need fast move from -63.0846  /  -38.6396  /  55.1    to    -45.5775  /  63.1089  /  55.1
//  dominantAxis is: -1
//  need fast move from -49.4486  /  -63.7403  /  55.1    to    -38.7714  /  62.6113  /  55.1
//  dominantAxis is: -1
//  need fast move from -43.3259  /  -63.4689  /  55.1    to    -31.191  /  61.7854  /  55.1
//  dominantAxis is: -1
//  need fast move from -36.7949  /  -63.0388  /  55.1    to    -21.9512  /  60.1653  /  55.1
//  dominantAxis is: -1
//  need fast move from -29.5514  /  -62.2843  /  55.1    to    87.1922  /  46.0011  /  55.1
//  dominantAxis is: -2
//  need fast move from -20.7638  /  -60.6865  /  55.1    to    86.01  /  -43.1973  /  55.1
//  dominantAxis is: -2
//  need fast move from 86.0099  /  38.1973  /  55.1    to    80.2857  /  -29.9731  /  55.1
//  dominantAxis is: 1
//  need fast move from 80.2857  /  24.9731  /  55.1    to    68  /  -44.1  /  55.1
//  dominantAxis is: 1
void dump(std::vector<std::vector<std::vector<GOContour*>>> cPool) {
  int n = 0;

  for (auto& cutPaths : cPool) {
      int i=0;

      qDebug() << "dump layer #" << ++n;

      for (auto& p : cutPaths) {
          int j=0;

          qDebug() << "dump path #" << ++i;

          for (auto& c : p) {
              qDebug() << "contour #" << ++j
                       << "   from " << c->startPoint().X() << " / " << c->startPoint().Y()
                       << "   to   " << c->endPoint().X() << " / " << c->endPoint().Y();
              qDebug() << "   real - a0:" << c->angStart() << " <> a1:" << c->angEnd();
              qDebug() << c->toString();
              }
          }
      }
  }


gp_Pnt PathBuilder::genInterMove(std::vector<Workstep*>& ws, const gp_Pnt& from, const gp_Pnt& to, const Bnd_Box& bb, double xtend) {
  gp_Pnt center(bb.CornerMin().X() + (bb.CornerMax().X() - bb.CornerMin().X()) / 2
              , bb.CornerMin().Y() + (bb.CornerMax().Y() - bb.CornerMin().Y()) / 2
              , bb.CornerMin().Z() + (bb.CornerMax().Z() - bb.CornerMin().Z()) / 2);
  int reg0 = region(from, bb);
  int reg1 = region(to, bb);
  gp_Pnt   e=to, s=from, tmp=from;

  if (!reg0 && !reg1) {                         // both points are inside workpiece!
     ws.push_back(new WSStraightMove(from, to));
//     Core().view3D()->showShape(Core().helper3D()->genWorkLine(from, to)); //TODO: change to circle?
     }
  else if ((reg0 & Top    && reg1 & Top)
        || (reg0 & Bottom && reg1 & Bottom)
        || (reg0 & Left   && reg1 & Left)
        || (reg0 & Right  && reg1 & Right)) {   // both points are inside same region outside
                                                // 3 regions are combined, so top includes
                                                // left, middle and right of top
     if ((reg0 & reg1) == Top) {
        if (tmp.Y() - bb.CornerMax().Y() < xtend) {
           tmp.SetY(bb.CornerMax().Y() + xtend);
           e.SetY(bb.CornerMax().Y() + xtend);
           }
        }
     else if ((reg0 & reg1) == Bottom) {
        if (bb.CornerMin().Y() - tmp.Y() < xtend) {
           tmp.SetY(bb.CornerMin().Y() - xtend);
           e.SetY(bb.CornerMin().Y() - xtend);
           }
        }
     else if ((reg0 & reg1) == Right) {
        if (tmp.X() - bb.CornerMax().X() < xtend) {
           tmp.SetX(bb.CornerMax().X() + xtend);
           e.SetX(bb.CornerMax().X() + xtend);
           }
        }
     else if ((reg0 & reg1) == Left) {
        if (bb.CornerMin().X() - tmp.X() < xtend) {
           tmp.SetX(bb.CornerMin().X() - xtend);
           e.SetX(bb.CornerMin().X() - xtend);
           }
        }
     if (!Core().helper3D()->isEqual(tmp, s)) {
//        Core().view3D()->showShape(Core().helper3D()->genWorkLine(s, tmp));
        ws.push_back(new WSStraightMove(s, tmp));
        s = tmp;
        }
     ws.push_back(new WSTraverse(s, e));
//     Core().view3D()->showShape(Core().helper3D()->genFastMove(s, e));
     if (!Core().helper3D()->isEqual(e, to)) {
//        Core().view3D()->showShape(Core().helper3D()->genWorkLine(e, to));
        ws.push_back(new WSStraightMove(e, to));
        }
     }
  else {                                        // both points are in different regions
                                                // first pass handles startpoint to safe area
     // calc endpoint to safe area cause we need that safe point before last move
     if (reg1 & Top) {
        if (to.Y() - bb.CornerMax().Y() < xtend) {
           e.SetY(bb.CornerMax().Y() + xtend);
           }
        }
     else if (reg1 & Bottom) {
        if (bb.CornerMin().Y() - to.Y() < xtend) {
           e.SetY(bb.CornerMin().Y() - xtend);
           }
        }
     else if (reg1 & Right) {
        if (to.X() - bb.CornerMax().X() < xtend) {
           e.SetX(bb.CornerMax().X() + xtend);
           }
        }
     else if (reg1 & Left) {
        if (bb.CornerMin().X() - to.X() < xtend) {
           e.SetX(bb.CornerMin().X() - xtend);
           }
        }
     else if (!reg1) {
        // need workmove to inside
        double d[] = { to.X() - bb.CornerMin().X()
                     , to.X() - bb.CornerMax().X()
                     , to.Y() - bb.CornerMin().Y()
                     , to.Y() - bb.CornerMax().Y() };
        int i=0, j=3;

        while (i < j) {
              if (abs(d[i]) < abs(d[j])) {
                 --j;
                 }
              else {
                 ++i;
                 }
              }
        switch (i) {
          case 0: e.SetX(bb.CornerMin().X() - xtend); break;
          case 1: e.SetX(bb.CornerMax().X() + xtend); break;
          case 2: e.SetY(bb.CornerMin().Y() - xtend); break;
          case 3: e.SetY(bb.CornerMax().Y() + xtend); break;
          }
        }

     if (tmp.Z() > e.Z()) {
        tmp = e;
        tmp.SetZ(from.Z());
        }
     else {
        if (reg0 & Top) {
           if (tmp.Y() - bb.CornerMax().Y() < xtend) {
              tmp.SetY(bb.CornerMax().Y() + xtend);
              }
           }
        else if (reg0 & Bottom) {
           if (bb.CornerMin().Y() - tmp.Y() < xtend) {
              tmp.SetY(bb.CornerMin().Y() - xtend);
              }
           }
        else if (reg0 & Right) {
           if (tmp.X() - bb.CornerMax().X() < xtend) {
              tmp.SetX(bb.CornerMax().X() + xtend);
              }
           }
        else if (reg0 & Left) {
           if (bb.CornerMin().X() - tmp.X() < xtend) {
              tmp.SetX(bb.CornerMin().X() - xtend);
              }
           }
        else if (!reg0) {
           qDebug() << ">>> TODO: need workmove outside of workpiece ...";
           }

        if (!Core().helper3D()->isEqual(tmp, s)) {
   //        if (tmp.Z() > to.Z()) Core().view3D()->showShape(Core().helper3D()->genFastMove(s, tmp));
   //        else                  Core().view3D()->showShape(Core().helper3D()->genWorkLine(s, tmp));
           if (tmp.Z() > to.Z()) ws.push_back(new WSTraverse(s, tmp));
           else                  ws.push_back(new WSStraightMove(s, tmp));
           s = tmp;
           }
        // now s is in safe area

        // possibly 3 moves to e position - e is calculated to safe area
        // region is a 4bit mask with: top | bottom | right | left
        if (reg0 & Top || reg0 & Bottom) {
           if (s.X() < center.X()) tmp.SetX(bb.CornerMin().X() - xtend);
           else                    tmp.SetX(bb.CornerMax().X() + xtend);

           ws.push_back(new WSTraverse(s, tmp));
   //       Core().view3D()->showShape(Core().helper3D()->genFastMove(s, tmp));
           s = tmp;
           tmp.SetY(e.Y());
           }
        else if (reg0 & Right || reg0 & Left) {
           if (e.Y() < center.Y()) {
              tmp.SetY(bb.CornerMin().Y() - xtend);
              }
           else {
              tmp.SetY(bb.CornerMax().Y() + xtend);
              }
           ws.push_back(new WSTraverse(s, tmp));
   //         Core().view3D()->showShape(Core().helper3D()->genFastMove(s, tmp));
           s = tmp;
           tmp.SetX(e.X());
           }
        }
     if (!Core().helper3D()->isEqual(tmp, s)) {
        ws.push_back(new WSTraverse(s, tmp));
//        Core().view3D()->showShape(Core().helper3D()->genFastMove(s, tmp));
        s = tmp;
        }
     if (!Core().helper3D()->isEqual(s, e)) {
        ws.push_back(new WSTraverse(s, e));
//         Core().view3D()->showShape(Core().helper3D()->genFastMove(s, e));
        }
     if (!Core().helper3D()->isEqual(e, to)) {
        ws.push_back(new WSStraightMove(e, to));
//         Core().view3D()->showShape(Core().helper3D()->genWorkLine(e, to));
        }
     }
  return to;
  }


gp_Pnt PathBuilder::genXTraverse(std::vector<Workstep*>& ws, int dir, const gp_Pnt& startPos, const gp_Pnt& endPos, const Bnd_Box& bbWP, double xtend) {
  Bnd_Box bb  = bbWP;
  gp_Pnt  s   = startPos;
  gp_Pnt  e   = endPos;
  gp_Pnt  tmp = s;

  bb.Enlarge(xtend);
  // ensure safe start position for fast move
  if (s.Y() < bbWP.CornerMin().Y() && s.Y() > bb.CornerMin().Y()) {
     tmp.SetY(bb.CornerMin().Y());
     ws.push_back(new WSStraightMove(s, tmp)); // 1
     s = tmp;
     }
  else if (s.Y() > bbWP.CornerMax().Y() && s.Y() < bb.CornerMax().Y()) {
     tmp.SetY(bb.CornerMax().Y());
     ws.push_back(new WSStraightMove(s, tmp)); // 1
     s = tmp;
     }
  else if (s.Y() > bbWP.CornerMin().Y() && s.Y() < bbWP.CornerMax().Y()) {
     if (dir > 0) tmp.SetX(bb.CornerMax().X());
     else         tmp.SetX(bb.CornerMin().X());
     ws.push_back(new WSStraightMove(s, tmp)); // 2
     s = tmp;
     }

  // y is safe, so move to safe x now
  if (Core().helper3D()->isEqual(tmp.X(), startPos.X())) {
     if (dir > 0) tmp.SetX(bb.CornerMax().X());
     else         tmp.SetX(bb.CornerMin().X());
     ws.push_back(new WSTraverse(s, tmp));  // 2
     s = tmp;
     }

  // check safe Y area for endpoint
  if (e.Y() < bbWP.CornerMin().Y() && e.Y() > bb.CornerMin().Y()) {
     e.SetY(bb.CornerMin().Y());
     }
  else if (e.Y() > bbWP.CornerMax().Y() && e.Y() < bb.CornerMax().Y()) {
     e.SetY(bb.CornerMax().Y());
     }
  tmp.SetY(e.Y());
  ws.push_back(new WSTraverse(s, tmp)); // 3
  s = tmp;

  if (!Core().helper3D()->isEqual(e.Y(), endPos.Y())) {
     tmp.SetX(e.X());
     ws.push_back(new WSTraverse(s, tmp)); // 4
     s = tmp;
     }

  if (endPos.Y() > bb.CornerMin().Y() && endPos.Y() < bb.CornerMax().Y()) {
     ws.push_back(new WSStraightMove(s, endPos));
     }
  else {
     ws.push_back(new WSTraverse(s, endPos));
     }
  return endPos;
  }


//gp_Pnt PathBuilder::genXTraverse(std::vector<Workstep*>& ws, int dir, const gp_Pnt& startPos, const gp_Pnt& endPos, const Bnd_Box& bb, double xtend) {
//  gp_Pnt s   = startPos;
//  gp_Pnt e   = endPos;
//  gp_Pnt tmp = s;

//  // check whether we have to move in y first
//  if (s.Y() < bb.CornerMin().Y()) {
//     if (bb.CornerMin().Y() - s.Y() < xtend) {
//        // ok, have to move to negative Y
//        tmp.SetY(bb.CornerMin().Y() - xtend);
//        ws.push_back(new WSTraverse(s, tmp));
//        s = tmp;
//        }
//     }
//  else if (s.Y() > bb.CornerMax().Y()) {
//     if (s.Y() - bb.CornerMax().Y() < xtend) {
//        // ok, have to move to positive Y
//        tmp.SetY(bb.CornerMax().Y() + xtend);
//        ws.push_back(new WSStraightMove(s, tmp));
//        s = tmp;
//        }
//     }
//  if (dir > 0) {
//     tmp.SetX(fmax(bb.CornerMax().X() + xtend, startPos.X()));
//     if (s.X() - bb.CornerMax().X() < xtend) ws.push_back(new WSStraightMove(s, tmp));
//     else                                    ws.push_back(new WSTraverse(s, tmp));
//     s = tmp;
//     }
//  else {
//     tmp.SetX(fmin(bb.CornerMin().X() - xtend, startPos.X()));
//     if (bb.CornerMin().X() - startPos.X() < xtend) ws.push_back(new WSStraightMove(s, tmp));
//     else                                           ws.push_back(new WSTraverse(s, tmp));
//     s = tmp;
//     }
////  ws.push_back(new WSTraverse(s, tmp)); // 2
////  s = tmp;
//  if (e.Y() < bb.CornerMin().Y()) {
//     if (bb.CornerMin().Y() - e.Y() < xtend) {
//        tmp.SetY(bb.CornerMin().Y() - xtend);
//        ws.push_back(new WSTraverse(s, tmp)); // 3
//        s = tmp;
//        }
//     tmp.SetX(e.X());
//     ws.push_back(new WSTraverse(s, tmp)); // 4
//     s = tmp;
//     if (bb.CornerMin().Y() - e.Y() < xtend) {
//        ws.push_back(new WSStraightMove(s, e)); // 5
//        s = e;
//        }
//     }
//  else if (e.Y() > bb.CornerMax().Y()) {
//     if (e.Y() - bb.CornerMax().Y() < xtend) {
//        tmp.SetY(bb.CornerMax().Y() + xtend);
//        ws.push_back(new WSTraverse(s, tmp)); // 5
//        s = tmp;
//        }
//     }
//  else {
//     tmp.SetY(e.Y());
//     ws.push_back(new WSTraverse(s, tmp)); // 3
//     s = tmp;
//     }
//  return s;
//  }


gp_Pnt PathBuilder::genYTraverse(std::vector<Workstep*>& ws, int dir, const gp_Pnt& startPos, const gp_Pnt& endPos, const Bnd_Box& bbWP, double xtend) {
  Bnd_Box bb  = bbWP;
  gp_Pnt  s   = startPos;
  gp_Pnt  e   = endPos;
  gp_Pnt  tmp = s;

  bb.Enlarge(xtend);
  // ensure safe start position for fast move
  // ensure safe start position for fast move
  if (s.X() < bbWP.CornerMin().X() && s.X() > bb.CornerMin().X()) {
     tmp.SetY(bb.CornerMin().X());
     ws.push_back(new WSStraightMove(s, tmp)); // 1
     s = tmp;
     }
  else if (s.X() > bbWP.CornerMax().X() && s.X() < bb.CornerMax().X()) {
     tmp.SetY(bb.CornerMax().X());
     ws.push_back(new WSStraightMove(s, tmp)); // 1
     s = tmp;
     }
  else if (s.X() > bbWP.CornerMin().X() && s.X() < bbWP.CornerMax().X()) {
     if (dir > 0) tmp.SetY(bb.CornerMax().Y());
     else         tmp.SetY(bb.CornerMin().Y());
     ws.push_back(new WSStraightMove(s, tmp)); // 2
     s = tmp;
     }

  // y is safe, so move to safe x now
  if (Core().helper3D()->isEqual(tmp.Y(), startPos.Y())) {
     if (dir > 0) tmp.SetY(bb.CornerMax().Y());
     else         tmp.SetY(bb.CornerMin().Y());
     ws.push_back(new WSTraverse(s, tmp));  // 2
     s = tmp;
     }

  // check safe Y area for endpoint
  if (e.X() < bbWP.CornerMin().X() && e.X() > bb.CornerMin().X()) {
     e.SetY(bb.CornerMin().X());
     }
  else if (e.X() > bbWP.CornerMax().X() && e.X() < bb.CornerMax().X()) {
     e.SetY(bb.CornerMax().X());
     }
  tmp.SetY(e.X());
  ws.push_back(new WSTraverse(s, tmp)); // 3
  s = tmp;

  if (!Core().helper3D()->isEqual(e.X(), endPos.X())) {
     tmp.SetY(e.Y());
     ws.push_back(new WSTraverse(s, tmp)); // 4
     s = tmp;
     }

  if (endPos.X() > bb.CornerMin().X() && endPos.X() < bb.CornerMax().X()) {
     ws.push_back(new WSStraightMove(s, endPos));
     }
  else {
     ws.push_back(new WSTraverse(s, endPos));
     }
  return endPos;
  }


std::vector<Workstep*> PathBuilder::genPath4Pockets(Operation* op, const Bnd_Box& bb, const gp_Dir& baseNorm, const std::vector<std::vector<GOPocket*>>& pool, double xtend) {
  double safeX, safeY;
  gp_Pnt center(bb.CornerMin().X() + (bb.CornerMax().X() - bb.CornerMin().X()) / 2
              , bb.CornerMin().Y() + (bb.CornerMax().Y() - bb.CornerMin().Y()) / 2
              , bb.CornerMin().Z() + (bb.CornerMax().Z() - bb.CornerMin().Z()) / 2);
  std::vector<Workstep*> workSteps;
  gp_Pnt  startPos(0, 0, 300);
  gp_Pnt  endPos = startPos, tmp;

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
                 if (!workSteps.size()) {
                    int region = calcRegion(endPos, bb, xtend);

                    switch (region) {
                      case 1:
                           if (endPos.X() - bb.CornerMax().X() < xtend) {
                              tmp = startPos;
                              tmp.SetX(bb.CornerMax().X() + xtend);
                              }
                           break;
                      case -1:
                           if (bb.CornerMin().X() - endPos.X() < xtend) {
                              tmp = startPos;
                              tmp.SetX(bb.CornerMin().X() - xtend);
                              }
                           break;
                      case  2:
                           if (endPos.Y() - bb.CornerMax().Y() < xtend) {
                              tmp = startPos;
                              tmp.SetY(bb.CornerMax().Y() + xtend);
                              }
                           break;
                      case -2:
                           if (bb.CornerMin().Y() - endPos.Y() < xtend) {
                              tmp = startPos;
                              tmp.SetY(bb.CornerMin().Y() - xtend);
                              }
                           break;
                      default:
                           tmp = endPos;
                           tmp.SetZ(startPos.Z());
                           break;
                      }
                    workSteps.push_back(new WSTraverse(startPos, tmp));
                    startPos = tmp;
                    tmp.SetZ(endPos.Z());
                    workSteps.push_back(new WSTraverse(startPos, tmp));
                    startPos = tmp;
                    if (region) workSteps.push_back(new WSStraightMove(startPos, endPos));
                    }
                 else {
                    // start equals end on closed contours, so try
                    // straight work move from end to start
                    qDebug() << "move from closed contour to next contour?!?";
                    endPos = startPos;
                    }
                 }
              else {
                 // set startpos to endpoint of last cutpath (contour)
                 // need 3 moves to startpoint of next cutpath, as both
                 // endpoints of cutpath are in safe area
                 startPos = endPos;
                 endPos   = c->startPoint();
//                 double debugOffset = i*3 + n*3;

                 xtend += 3; // debug separation of moves

                 qDebug() << "need fast move from" << startPos.X() << " / " << startPos.Y() << " / " << startPos.Z()
                          << "   to   " << endPos.X() << " / " << endPos.Y() << " / " << endPos.Z();
                 int dominantAxis = calcMainDir(startPos, endPos, bb, xtend);

                 qDebug() << "dominantAxis is:" << dominantAxis;
                 if (!dominantAxis) {
                    endPos = startPos;          // direct move possible, so reset last change
                    }
                 else if (dominantAxis == 1 || dominantAxis == -1) {  // main move to X
                    endPos = genXTraverse(workSteps, dominantAxis, startPos, endPos, bb, xtend);
                    }
                 else if (dominantAxis == 2 || dominantAxis == -2) {  // main move to Y
                    endPos = genYTraverse(workSteps, dominantAxis, startPos, endPos, bb, xtend);
                    }
//                 if (!Core().helper3D()->isEqual(startPos, endPos)) {
//                    workSteps.push_back(new WSTraverse(startPos, endPos));
//                    }
                 }
              startPos = endPos;
              endPos   = c->startPoint();
              if (!Core().helper3D()->isEqual(startPos, endPos)) {
                 workSteps.push_back(new WSTraverse(startPos, endPos));
                 startPos = endPos;
                 }
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


static bool cmpContour(GOContour* l, GOContour* r) {
  if (!l || !r) return false;
  if (l->order() == r->order())
     return l->angStart() < r->angStart();
  return l->order() < r->order();
  }


std::vector<Workstep*> PathBuilder::genToolPath(Operation* op, Handle(AIS_Shape) cutPart) {
  TargetDefinition*      td  = op->targets.at(0);
  SweepTargetDefinition* std = dynamic_cast<SweepTargetDefinition*>(td);
//  CCTargetDefinition* std = dynamic_cast<SweepTargetDefinition*>(td);
  std::vector<Workstep*> toolPath;

  if (!td) return toolPath;
  gp_Pnt center(op->wpBounds.CornerMin().X() + (op->wpBounds.CornerMax().X() - op->wpBounds.CornerMin().X()) / 2
              , op->wpBounds.CornerMin().Y() + (op->wpBounds.CornerMax().Y() - op->wpBounds.CornerMin().Y()) / 2
              , op->wpBounds.CornerMin().Z() + (op->wpBounds.CornerMax().Z() - op->wpBounds.CornerMin().Z()) / 2);
  GOContour* contour = td->contour();

  if (!contour) return toolPath;    // no vertical toolpath without contour!
  std::vector<std::vector<std::vector<GOContour*>>> clippedParts;
  ToolEntry*             activeTool    = Core().toolListModel()->tool(Core().toolListModel()->findToolNum(op->toolNum()));
  double                 xtend         = activeTool->fluteDiameter() * 0.8;
  double                 firstOffset   = op->offset() + activeTool->fluteDiameter() / 2;
  Bnd_Box                bb            = cutPart->BoundingBox();
  bool                   curveIsBorder = std ? std->isBaseBorder() : false;
  bool                   wantPockets   = false;
  int                    steps         = (td->zMax() - td->zMin()) / op->cutDepth();
  double                 startZ        = bb.CornerMin().Z() + steps * op->cutDepth();
  double                 lastZ         = fmax(startZ - activeTool->cuttingDepth()
                                            , op->finalDepth() + op->offset());
  double                 curZ          = startZ;
  int                    n=0;

  if (std && curveIsBorder) firstOffset += abs(calcAdditionalOffset(std, contour));
  contour->extendBy(10);
  contour->simplify(curZ);
  qDebug() << "cutting contour:" << contour->toString();
  qDebug() << "center of workpiece:" << center.X() << " / " << center.Y();
//  curZ -= op->cutDepth();

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

//  center:  9  /  0    extend:  9.6
//  workpiece:  -61  /  -55    to    79  /  55
  qDebug() << "center: " << center.X() << " / " << center.Y() << "   extend: " << xtend;
  qDebug() << "workpiece: " << op->wpBounds.CornerMin().X() << " / " << op->wpBounds.CornerMin().Y()
           << "   to   "    << op->wpBounds.CornerMax().X() << " / " << op->wpBounds.CornerMax().Y();
  dump(clippedParts);

  if (wantPockets) {
     std::vector<std::vector<GOPocket*>> pool = splitCurves(clippedParts);
     gp_Dir workDir = std ? std->baseDir() : gp_Dir(0, 0, 1);

     bb.Add(op->wpBounds);
     toolPath = genPath4Pockets(op, bb, workDir, pool, xtend);
     }
  else {
     // flatten clippedParts ...
     std::vector<std::vector<GOContour*>> pool;

     for (auto& lp: clippedParts) {
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

     curZ = startZ;
     for (auto& lp : pool) {
         int mx = lp.size() - 1;

         std::sort(lp.begin(), lp.end(), cmpContour);
         for (int i=mx; i >= 0; --i) {
             GOContour* c = lp.at(i);

             c->simplify(curZ);
             s = c->startPoint();
             genInterMove(toolPath, e, s, op->wpBounds, xtend);
             processContour(toolPath, c);
             e = c->endPoint();
             }
         s = e;
         e.SetZ(op->wpBounds.CornerMax().Z() + op->safeZ1());
         toolPath.push_back(new WSTraverse(s, e));
         curZ -= op->cutDepth();
         }
     }
  return toolPath;
  }


void PathBuilder::processContour(std::vector<Workstep*>& ws, GOContour* c) {
  qDebug() << "process contour" << c->toString();

  for (auto& go : c->segments()) {
      switch (go->type()) {
        case GTLine:
             ws.push_back(new WSStraightMove(go->startPoint(), go->endPoint()));
             break;
        case GTCircle: {
             GOCircle* gc = dynamic_cast<GOCircle*>(go);

             ws.push_back(new WSArc(go->startPoint(), go->endPoint(), gc->center(), gc->isCCW()));
             } break;
        default:
             qDebug() << "unsupported graphic-type" << go->type();
             break;
        }
      }
  }


// all contiguous segments form a contour (instance of GOContour).
// A cutted offset curve may lead to several subcontours (vector of GOContour)
// processCurve processes all contour(-segments) of same z-level
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
         GOContour*                        path     = new GOContour(center, i);
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

            for (auto s : segments) path->add(s);   // keep all segments for last workpath
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
                   path = new GOContour(center, i);
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


int PathBuilder::region(const gp_Pnt& p, const Bnd_Box& bb) const {
  int rv = Inside;

  if      (p.X() < bb.CornerMin().X()) rv |= Left;
  else if (p.X() > bb.CornerMax().X()) rv |= Right;
  if      (p.Y() < bb.CornerMin().Y()) rv |= Bottom;
  else if (p.Y() > bb.CornerMax().Y()) rv |= Top;

  return rv;
  }


std::vector<std::vector<GOPocket*>> PathBuilder::splitCurves(const std::vector<std::vector<std::vector<GOContour*>>>& pool) {
  std::vector<std::vector<GOPocket*>> levels;
  auto& c = pool.at(0);
  int nL = 0; //, nC = 0, nCC = 0;

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
      int  n=0;

      // first create all pockets necessary
      for (auto c : cp) {
          double a0 = c->angStart();
          double a1 = c->angEnd();

          while (a0 < 0) a0 += 2*M_PI;
          while (a1 < 0) a1 += 2*M_PI;

          qDebug() << "create pocket for contour from" << c->startPoint().X() << " / " << c->startPoint().Y()
                   << "   to   "   << c->endPoint().X() << " / " << c->endPoint().Y();
          qDebug() << "   check: a0 == " << a0 << "   a1 == " << a1;

          if (a0 < a1) pockets.push_back(new GOPocket(c->endPoint(), c->startPoint()));
          else         pockets.push_back(new GOPocket(c->startPoint(), c->endPoint()));
          }

      // process levelParts from out to in and fill created pockets
      for (auto i = lp.rbegin(); i != lp.rend(); ++i) {
          auto cp = *i;

          for (int n=0; n < cp.size(); ++n) {
              auto   c  = cp.at(n);

              pockets.at(n)->add(c);
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


const int PathBuilder::Inside = 0;
const int PathBuilder::Left   = 1;
const int PathBuilder::Right  = 2;
const int PathBuilder::Bottom = 4;
const int PathBuilder::Top    = 8;
