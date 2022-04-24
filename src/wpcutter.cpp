/* 
 * **************************************************************************
 * 
 *  file:       wpcutter.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    19.4.2022 by Django Reinhard
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
#include "wpcutter.h"
#include "core.h"
#include "gocontour.h"
#include "occtviewer.h"
#include "util3d.h"

#include <Bnd_Box.hxx>
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <ElCLib.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Line.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>

#include <QDebug>
static bool wantDebug = true;
static double PIx2    = M_PI + M_PI;


WPCutter::WPCutter(Handle(AIS_Shape) workPiece, QObject* parent)
 : workPiece(workPiece)
 , cutShape(workPiece->Shape())
 , bbWP(workPiece->BoundingBox())
 , rMaster(fmax((bbWP.CornerMax().X() - bbWP.CornerMin().X()) / 2
              , (bbWP.CornerMax().Y() - bbWP.CornerMin().Y()) / 2))
 , isRound(true) {
  determineCutShape();
  }


/*
 * determine shape of workpiece, which will be used to cut curves
 */
void WPCutter::determineCutShape() {
  if (cutShape.ShapeType() == TopAbs_EDGE) {
     if (wantDebug) qDebug() << "source of intersection is an edge";
     TopoDS_Edge  edge = TopoDS::Edge(cutShape);

     if (BRep_Tool::IsGeometric(edge)) {
        double first, last;
        Handle(Geom_Curve) c = BRep_Tool::Curve(edge, first, last);

        if (c->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
           Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(c);

           cMaster  = circle->Position().Location();
           dMaster  = circle->Position().Direction();
           rMaster  = circle->Radius();
           isRound = true;

           if (wantDebug) qDebug() << "DCS orig:" << cMaster.Z() << " <> "
                                   << (bbWP.CornerMin().Z() + (bbWP.CornerMax().Z() - bbWP.CornerMin().Z()) / 2);
           cMaster.SetZ(bbWP.CornerMin().Z() + (bbWP.CornerMax().Z() - bbWP.CornerMin().Z()) / 2);
           }
        }
     }
  else if (cutShape.ShapeType() == TopAbs_SOLID) {
     std::vector<TopoDS_Edge> edges = Core().helper3D()->allEdgesWithin(cutShape);

     if (wantDebug) qDebug() << "Ok, got solid for intersection";
     isRound = false;
     for (auto e : edges) {
         if (BRep_Tool::IsGeometric(e)) {
            double first, last;
            Handle(Geom_Curve) c = BRep_Tool::Curve(e, first, last);

            if (c->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
               Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(c);

               cMaster  = circle->Position().Location();
               rMaster  = circle->Radius();
               isRound = true;

               if (wantDebug) qDebug() << "DCS orig:" << cMaster.Z() << " <> "
                                       << (bbWP.CornerMin().Z() + (bbWP.CornerMax().Z() - bbWP.CornerMin().Z()) / 2);
               cMaster.SetZ(bbWP.CornerMin().Z() + (bbWP.CornerMax().Z() - bbWP.CornerMin().Z()) / 2);
               break;
               }
            }
         }
     }
  else {
     if (wantDebug) qDebug() << "unknown source for intersection (ShapeType: " << cutShape.ShapeType() << ")";
     }
  }


/*
 * curve segment is an arc. Check intersectoin agains cutShape
 *
 * returns: -1 (no intersection)
 *           0 (possible intersection)
 *           1 (completely inside cutShape)
 */
int WPCutter::checkCIntersection(const gp_Pnt& center, double radius) {
  if (isRound) {
     double d = center.Distance(cMaster);

     if (wantDebug) qDebug() << "check center:" << center.X() << " / " << center.Y() << " / " << center.Z() << "R:" << radius
                             << "master:" << cMaster.X() << " / " << cMaster.Y() << " / " << cMaster.Z() << "R:" << rMaster;

     if (Core().helper3D()->isEqual(d, 0)) {        // segment circle has same center than cutShape
        if (radius > rMaster) return -1;
        else return 1;
        }
     if (d > (rMaster + radius)) return -1;
     if ((d + radius) > rMaster) return 0;
     return 1;
     }
  else {
     //TODO:
     }
  return 0;
  }


/*
 * curve segment is a straight line. Check intersection agains cutShape
 *
 * returns: -1 (no intersection)
 *           0 (possible intersection)
 *           1 (completely inside cutShape)
 */
int WPCutter::checkLIntersection(const gp_Pnt& p0, const gp_Pnt& p1) {
  if (isRound) {
     double d0 = p0.Distance(cMaster);
     double d1 = p1.Distance(cMaster);

     if (d0 > rMaster && d1 > rMaster) return -1;
     if (d0 > rMaster || d1 > rMaster) return 0;
     return 1;
     }
  else {
     int r0 = clipRegion(p0);
     int r1 = clipRegion(p1);

     if (!r0 && !r1) return  1;
     if (r0  ==  r1) return -1;
     return 0;
     }
  return 0;
  }


int WPCutter::clipRegion(const gp_Pnt& p) {
  int rv = Inside;

  if      (p.X() < bbWP.CornerMin().X()) rv = Left;
  else if (p.X() > bbWP.CornerMax().X()) rv = Right;
  if      (p.Y() < bbWP.CornerMin().Y()) rv = Bottom;
  else if (p.Y() > bbWP.CornerMax().Y()) rv = Top;

  return rv;
  }


// clipping with radical line theorem (Kimberling 1998, p. 224)
TopoDS_Edge WPCutter::cutCircleCircle(Handle(Geom_Curve) c, double first, double last, int orientation, const gp_Pnt& endPoint, bool showCutPoints) {
  Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(c);
  TopoDS_Edge edge;
  gp_Pnt      center = circle->Axis().Location();
  double r   = circle->Radius();
  gp_Pnt p0  = c->Value(first);                                   // endpoints of arc
  gp_Pnt p1  = c->Value(last);
  double d   = center.Distance(cMaster);                          // distance between both centers
//  double moc = first + (last - first) / 2;                        // parameter of midpoint of arc
  TopoDS_Edge cL = BRepBuilderAPI_MakeEdge(cMaster, center);
  double parm0, parm1;
  Handle(Geom_Curve) gLin = BRep_Tool::Curve(cL, parm0, parm1);
  Handle(Geom_Line)  line = Handle(Geom_Line)::DownCast(gLin);    // line from center to center
  double dRL =  (d * d + rMaster * rMaster - r * r) / (2.0 * d);  // distances of radical line from center
  double rlH = sqrt(rMaster * rMaster - dRL * dRL);               // 1/2 length of radical line
  gp_Pnt cRL = gLin->Value(parm0 + dRL);                          // cutpoint between center/center and RL
  gp_Dir dirRL = line->Position().Direction(); // a
  gp_Dir dZ    = gp_Dir(0, 0, 1);              // b
  gp_Dir nRL(dirRL.Y() * dZ.Z() - dirRL.Z() * dZ.Y()              // calculate normal of center/center line
           , dirRL.Z() * dZ.X() - dirRL.X() * dZ.Z()
           , dirRL.X() * dZ.Y() - dirRL.Y() * dZ.X());
  Geom_Line lRL(gp_Ax1(cRL, nRL));                                // create radical line
  gp_Pnt cP0 = lRL.Value(rlH);                                    // cutpoints radical line with circle
  gp_Pnt cP1 = lRL.Value(-rlH);
  double pr0 = ElCLib::Parameter(circle->Circ(), {cP0.X(), cP0.Y(), p0.Z()});
  double pr1 = ElCLib::Parameter(circle->Circ(), {cP1.X(), cP1.Y(), p0.Z()});
  double d0 = p0.Distance(cMaster);
  double d1 = p1.Distance(cMaster);

  pr0 = last > PIx2 && pr0 < 1 ? pr0 + PIx2 : pr0;
  pr1 = last > PIx2 && pr1 < 1 ? pr1 + PIx2 : pr1;

  if (showCutPoints) {
     Core().view3D()->createAxisCross(cP0, 2, nullptr, Quantity_NOC_GREEN3);
     Core().view3D()->createAxisCross(cP1, 2, nullptr, Quantity_NOC_RED3);
     }
  if (pr0 >= first && pr0 <= last) {
     if (wantDebug) qDebug() << "<<< first cutpoint is part of line segment ==> "
                             << first << " <> "  << pr0 << " <> " << " <> " << last << "orientation:" << orientation;
     if (Core().helper3D()->isEqual(endPoint, p1) || d1 < d0) {
        if (wantDebug) qDebug() << "\tsecond part is part of interest";
        edge = BRepBuilderAPI_MakeEdge(c, {cP0.X(), cP0.Y(), p0.Z()}, p1);
        }
     else {
        if (wantDebug) qDebug() << "\tfirst part is part of interest";
        edge = BRepBuilderAPI_MakeEdge(c, p0, {cP0.X(), cP0.Y(), p0.Z()});
        }
     }
  else if (pr1 >= first && pr1 <= last) {
     if (wantDebug) qDebug() << ">>> second cutpoint is part of line segment ==> "
                             << first << " <> "  << pr1 << " <> " << last << "orientation:" << orientation;
     if (Core().helper3D()->isEqual(endPoint, p1) || d1 < d0) {
        if (wantDebug) qDebug() << "\tsecond part is part of interest";
        edge = BRepBuilderAPI_MakeEdge(c, {cP1.X(), cP1.Y(), p0.Z()}, p1);
        }
     else {
        if (wantDebug) qDebug() << "\tfirst part is part of interest";
        edge = BRepBuilderAPI_MakeEdge(c, p0, {cP1.X(), cP1.Y(), p0.Z()});
        }
     }
  else {
     // no cutpoints and both endpoints inside circle -> whole segment is inside
     if (d0 < rMaster && d1 < rMaster) edge = BRepBuilderAPI_MakeEdge(c, first, last);
     else {
        if (wantDebug) qDebug() << "?!?!? - none of the calculated cutpoints is inside of line parameters?!?"
                                << first << " <> " << pr0 << "|" << pr1 << " <> " << last << "orientation:" << orientation;
        }
     }
  return edge;
  }


TopoDS_Edge WPCutter::cutCircleRect(Handle(Geom_Curve) c, double first, double last, int orientation, const gp_Pnt& endPoint) {
  Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(c);
  TopoDS_Edge edge   = BRepBuilderAPI_MakeEdge(c, first, last);

  return edge;
  }


/*
 * as checkIntersection does a rough estimation only, here
 * we have to do exact calculations
 */
TopoDS_Edge WPCutter::cutCurve(Handle(Geom_Curve) c, double first, double last, int orientation, const gp_Pnt& endPoint, bool showCutPoints) {
  TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(c, first, last);
  Handle(AIS_Shape) rv;

  if (c->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
     if (isRound) edge = cutCircleCircle(c, first, last, orientation, endPoint, showCutPoints);
     else         edge = cutCircleRect(c, first, last, orientation, endPoint);
     }
  else if (c->DynamicType() == STANDARD_TYPE(Geom_Line)) {
     if (isRound) edge = cutLineCircle(c, first, last, orientation, endPoint);
     else         edge = cutLineRect(c, first, last, orientation, endPoint);
     }
  else {
     if (wantDebug) qDebug() << "--- cutCurve (NOT arc NOR line):" << first << " | " << last << " <> " << orientation;
     edge = BRepBuilderAPI_MakeEdge(c, first, last);
     }
  return edge;
  }


// clipping with radical line theorem (Kimberling 1998, p. 224)
TopoDS_Edge WPCutter::cutLineCircle(Handle(Geom_Curve) c, double first, double last, int orientation, const gp_Pnt& endPoint) {
  TopoDS_Edge edge; // = BRepBuilderAPI_MakeEdge(c, first, last);
  gp_Pnt p0 = c->Value(first);
  gp_Pnt p1 = c->Value(last);
  Handle(Geom_Line) line = Handle(Geom_Line)::DownCast(c);
  double d   = line->Lin().Distance(cMaster);

  if (d > rMaster) return edge; // no intersection
  double rlH  = sqrt(rMaster * rMaster - d * d);
  GeomAPI_ProjectPointOnCurve ppAlgo(cMaster, line);
  double prmCP = ppAlgo.Parameter(1);
  double pr0   = prmCP - rlH;
  double pr1   = prmCP + rlH;
  double d0    = p0.Distance(cMaster);
  double d1    = p1.Distance(cMaster);

  if (pr0 >= first && pr0 <= last) {
     if (wantDebug) qDebug() << "<<< first cutpoint is part of line segment ==> "
                             << first << " <> "  << pr0 << " <> " << last << "orientation:" << orientation;
     if (d1 < d0) {
        if (wantDebug) qDebug() << "\tsecond part is part of interest";
        edge = BRepBuilderAPI_MakeEdge(c, line->Value(pr0), p1);
        }
     else {
        if (wantDebug) qDebug() << "\tfirst part is part of interest";
        edge = BRepBuilderAPI_MakeEdge(c, p0, line->Value(pr0));
        }
     }
  else if (pr1 >= first && pr1 <= last) {
     if (wantDebug) qDebug() << ">>> second cutpoint is part of line segment ==> "
                             << first << " <> "  << pr1 << " <> " << last << "orientation:" << orientation;
     if (d1 < d0) {
        if (wantDebug) qDebug() << "\tsecond part is part of interest";
        edge = BRepBuilderAPI_MakeEdge(c, line->Value(pr1), p1);
        }
     else {
        if (wantDebug) qDebug() << "\tfirst part is part of interest";
        edge = BRepBuilderAPI_MakeEdge(c, p0, line->Value(pr1));
        }
     }
  else {
     if (wantDebug) qDebug() << "?!?!? - none of the calculated cutpoints is inside of line parameters?!?"
                             << first << " <> " << pr0 << "|" << pr1 << " <> " << last << "orientation:" << orientation;
//     edge = BRepBuilderAPI_MakeEdge(c, first, last);
     }
  return edge;
  }


// clip by Cohen-Sutherland algorithm
TopoDS_Edge WPCutter::cutLineRect(Handle(Geom_Curve) c, double first, double last, int orientation, const gp_Pnt& endPoint) {
  TopoDS_Edge edge;
  bool   accept = false;
  gp_Pnt p0 = c->Value(first);
  gp_Pnt p1 = c->Value(last);
  gp_Pnt pC;
  int    rC0 = clipRegion(p0);
  int    rC1 = clipRegion(p1);
  double dx  = p1.X() - p0.X();
  double dy  = p1.Y() - p0.Y();

  while (true) {
        if (!rC0 && !rC1) {
           accept = true;
           break;
           }
        else if (rC0 & rC1) break;
        else {
           int rcOut = rC0 ? rC0 : rC1;
           double x, y;

           if (rcOut & Top) {
              x = p0.X() + dx * (bbWP.CornerMax().Y() - p0.Y()) / dy;
              y = bbWP.CornerMax().Y();
              }
           else if (rcOut & Bottom) {
              x = p0.X() + dx * (bbWP.CornerMin().Y() - p0.Y()) / dy;
              y = bbWP.CornerMin().Y();
              }
           else if (rcOut & Right) {
              y = p0.Y() + dy * (bbWP.CornerMax().X() - p0.X()) / dx;
              x = bbWP.CornerMax().X();
              }
           else if (rcOut & Left) {
              y = p0.Y() + dy * (bbWP.CornerMin().X() - p0.X()) / dx;
              x = bbWP.CornerMin().X();
              }

           if (rcOut == rC0) {
              p0.SetX(x);
              p0.SetY(y);
              rC0 = clipRegion(p0);
              }
           else {
              p1.SetX(x);
              p1.SetY(y);
              rC1 = clipRegion(p1);
              }
           }
        }
  if (accept) edge = BRepBuilderAPI_MakeEdge(c, first, last);

  return edge;
  }


/*
 * process a curve (possibly a wire created from polyline)
 * As boolean of opencascade fails to create intersection curves,
 * we have to do it.
 * So check each segment of a curve whether it is inside of cutShape,
 * outside of cutShape or hit by contour of cutShape. The latter then
 * will be cut into a part that is inside the cutShape and the rest.
 *
 * Returns: all segments (clipped or original) that lie inside cutPart are
 *          assembled to build a new curve (own data structure)
 */
std::vector<GOContour*> WPCutter::processShape(TopoDS_Shape curve, const gp_Pnt& center) {
  std::vector<TopoDS_Edge> segments = Core().helper3D()->allEdgesWithin(curve);
  int mx = segments.size() - 1;
  int check;

  bbWP.Add(gp_Pnt(0, 0, -100));
  bbWP.Add(gp_Pnt(0, 0,  100));
  Core().view3D()->setBounds(bbWP);
  std::vector<GOContour*>   contours;
  GOContour*                contour = new GOContour(center);

  if (wantDebug) qDebug() << "bb:" << bbWP.CornerMin().X() << " / " << bbWP.CornerMin().Y() << " / " << bbWP.CornerMin().Z()
                     << "   <>   " << bbWP.CornerMax().X() << " / " << bbWP.CornerMax().Y() << " / " << bbWP.CornerMax().Z();

  for (int i=0; i <= mx; ++i) {
      TopoDS_Edge e = segments.at(i);

      if (BRep_Tool::IsGeometric(e)) {
         double first, last;
         Handle(Geom_Curve) c = BRep_Tool::Curve(e, first, last);
         int orientation = e.Orientation();
         gp_Pnt p0 = c->Value(first);
         gp_Pnt p1 = c->Value(last);
         double d  = p0.Distance(p1);
         double d0 = p0.Distance(cMaster);
         double d1 = p1.Distance(cMaster);

         if (c->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
            Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(c);
            const gp_Pnt&       center = circle->Position().Location();
            const gp_Dir&       dir    = circle->Position().Direction();
            double              radius = circle->Radius();

            check = checkCIntersection(center, radius);
            }
         else if (c->DynamicType() == STANDARD_TYPE(Geom_Line)) {
            check = checkLIntersection(p0, p1);
            }
         else {
            throw "unsupported geom type!";
            }
         if (wantDebug) qDebug() << "\tcheck intersection returned:\t" << check;

         switch (check) {
           case 0: {                  // 0 means one endpoint of segment is inside, the other is outside
                e = cutCurve(c, first, last, orientation, contour->endPoint());
                if (e.IsNull()) {
                   qDebug() << "OUPS - empty segment ?!?";
                   break;  // TODO: start new wire?!?
                   }
                if (!contour->add(e)) {
                   qDebug() << ">>>>>>>>>>> segment does not match contour!!! <<<<<<<<<<<<<<<<<" ;
                   break;
                   }
                continue;
                }

           case 1:                    // 1 means both endpoints of segment are inside
                if (!contour->add(e)) {
                   qDebug() << ">>>>>>>>>>> segment does not match contour!!! <<<<<<<<<<<<<<<<<" ;
                   break;
                   }
                continue;

           default:                   // rest means no endpoint of segment is inside
                                      // We don't need to check for intersection, as offset curves
                                      // are always parallel to workpiece contour!
                if (contour->size()) {
                   if (contour->startPoint() < contour->endPoint())
                      contour->invert();
                   contours.push_back(contour);
                   contour = new GOContour(center);
                   }
                break;
           }
         }
      }  
  if (contour->size()) {
     if (contour->startPoint() < contour->endPoint())
        contour->invert();

     // last cut-sequence may be part of first cut-sequence ...
     if (contours.size() && Core().helper3D()->isEqual(contour->endPoint(), contours.at(0)->startPoint())) {
        contours.at(0)->add(contour);
        }
     else {
        contours.push_back(contour);
        }
     }
  qDebug() << "\nresult of cutting shape:" << contours.size() << "subshapes\n";
  std::sort(contours.begin(), contours.end(), compContour);  

  return contours;
  }
