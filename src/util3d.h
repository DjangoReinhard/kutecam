/* 
 * **************************************************************************
 * 
 *  file:       util3d.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    10.4.2022 by Django Reinhard
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
#ifndef UTIL3D_H
#define UTIL3D_H
#include <QObject>
#include <AIS_Shape.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <gp_Pnt.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include "cavc/polyline.hpp"
#include <vector>
#include <set>
class QString;


extern bool operator<(const gp_Pnt& l, const gp_Pnt& r);


class Util3D : public QObject
{
  Q_OBJECT

public:
  std::vector<TopoDS_Face>       allFacesWithin(const TopoDS_Shape& shape);
  std::vector<TopoDS_Edge>       allEdgesWithin(const TopoDS_Shape& shape);
  TopoDS_Shape                   allEdgesWithin(const TopoDS_Shape& shape, Handle(TopTools_HSequenceOfShape) v);
  std::set<gp_Pnt>               allVertexCoordinatesWithin(const TopoDS_Shape& shape);
  std::vector<TopoDS_Wire>       allWiresWithin(const TopoDS_Shape& shape);
  gp_Pnt                         calcCircle(Handle(Geom_Circle) c, double param) const;
  gp_Pnt                         calcEllipse(Handle(Geom_Ellipse) e, double param) const;
  gp_Pnt                         calcLine(Handle(Geom_Line) l, double param) const;
  gp_Pnt                         calcPlane(Handle(Geom_Plane) p, double param0, double param1);
  TopoDS_Edge                    createArc(const gp_Pnt& from, const gp_Pnt& to, double radius, bool ccw = false);
  Handle(AIS_Shape)              createArc(const gp_Pnt& from, const gp_Pnt& to, const gp_Pnt& center, bool ccw = false);
  Handle(AIS_Shape)              createLine(const gp_Pnt& from, const gp_Pnt& to);
  Handle(AIS_Shape)              cut(const TopoDS_Shape& src, const TopoDS_Shape& tool);
  double                         deburr(double v);
  gp_Pnt                         deburr(const gp_Pnt& p);
  gp_Dir                         deburr(const gp_Dir& d);
  void                           dumpEdges(std::vector<TopoDS_Edge>& edges);
  void                           dumpPolyline(const cavc::Polyline<double>& pl);
  void                           dumpVertices(const std::set<gp_Pnt>& pool);
  std::vector<Handle(AIS_Shape)> explodeShape(const TopoDS_Shape& shape);
  Handle(AIS_Shape)              fixLocation(const TopoDS_Shape& s, double x, double y, double z);
  Handle(AIS_Shape)              fixRotation(const TopoDS_Shape& s, double angA, double angB, double angC);
  Handle(AIS_Shape)              genFastMove(const gp_Pnt& from, const gp_Pnt& to);
  Handle(AIS_Shape)              genWorkArc(const gp_Pnt& from, const gp_Pnt& to, const gp_Pnt& center, bool ccw);
  Handle(AIS_Shape)              genWorkLine(const gp_Pnt& from, const gp_Pnt& to);
  TopoDS_Shape                   intersect(const TopoDS_Shape& src, const TopoDS_Shape& tls);
  bool                           isEqual(double a, double b, double minDelta = MinDelta);
  bool                           isVertical(const gp_Dir& d) const;
  TopoDS_Shape                   loadBRep(const QString& fileName);
  TopoDS_Shape                   loadStep(const QString& fileName);
  TopoDS_Shape                   makeCube(const Standard_Real width, const Standard_Real height, const Standard_Real depth);
  TopoDS_Shape                   makeCube(const gp_Pnt& p0, const gp_Pnt& p1);
  gp_Vec                         normalOfFace(const TopoDS_Shape& face);
  std::vector<Handle(AIS_Shape)> pl2Shape(const cavc::Polyline<double>& pline, double commonZ, Quantity_Color c, double width=2);
  TopoDS_Shape                   pl2Wire(const cavc::Polyline<double>& pline, double commonZ);
//  std::vector<TopoDS_Edge>       sortEdges(std::vector<TopoDS_Edge>& re);
  cavc::Polyline<double>         toPolyline(const std::vector<TopoDS_Edge>& edges);
  cavc::Polyline<double>         toPolyline(TopoDS_Shape wire);
  static const double            MinDelta;
  };
#endif // UTIL3D_H
