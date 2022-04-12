/* 
 * **************************************************************************
 * 
 *  file:       util3d.cpp
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
#include "util3d.h"
#include <BRepAdaptor_Surface.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BOPAlgo_Splitter.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepAlgoAPI_Splitter.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <GC_MakeSegment.hxx>
#include <gp_Pnt.hxx>
#include <gp_Quaternion.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Plane.hxx>
#include <Geom_Surface.hxx>
#include <Geom_BSplineCurve.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <STEPControl_Reader.hxx>
#include <TopExp_Explorer.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include "kuteCAM.h"
#include <QString>
#include <QDebug>
#include <cmath>


bool operator<(const gp_Pnt& l,
               const gp_Pnt& r) {
  if (l.Z() == r.Z()) {
     if (l.Y() == r.Y())
        return l.X() < r.X();
     return l.Y() < r.Y();
     }
  return l.Z() < r.Z();
  }


TopoDS_Shape Util3D::allEdgesWithin(const TopoDS_Shape& shape, Handle(TopTools_HSequenceOfShape) v) {
  TopoDS_Shape rv;

  if (v.IsNull()) return rv;

  for (TopExp_Explorer edgeExplorer(shape, TopAbs_EDGE); edgeExplorer.More(); edgeExplorer.Next()) {
      const auto &edge = TopoDS::Edge(edgeExplorer.Current());

      if (edge.IsNull()) continue;
      v->Append(edge);
      }
  ShapeAnalysis_FreeBounds          fb;
  Handle(TopTools_HSequenceOfShape) wires;

  fb.ConnectEdgesToWires(v, Util3D::MinDelta, false, wires);

  qDebug() << wires->Size();
  if (wires->Size()) rv = wires->First();

  return rv;
  }


std::vector<TopoDS_Edge> Util3D::allEdgesWithin(const TopoDS_Shape& shape) {
  std::vector<TopoDS_Edge> edges;

  for (TopExp_Explorer edgeExplorer(shape, TopAbs_EDGE); edgeExplorer.More(); edgeExplorer.Next()) {
      const auto &edge = TopoDS::Edge(edgeExplorer.Current());

      if (edge.IsNull()) continue;
      edges.push_back(edge);
      }
//  if (edges.size()) return sortEdges(edges);
  return edges;
  }


std::vector<TopoDS_Face> Util3D::allFacesWithin(const TopoDS_Shape& shape) {
  std::vector<TopoDS_Face> faces;

  for (TopExp_Explorer faceExplorer(shape, TopAbs_FACE)
     ; faceExplorer.More()
     ; faceExplorer.Next()) {
      const auto& face = TopoDS::Face(faceExplorer.Current());

      if (face.IsNull()) continue;
      faces.push_back(face);
      }
  return faces;
  }


std::set<gp_Pnt> Util3D::allVertexCoordinatesWithin(const TopoDS_Shape& shape) {
  std::set<gp_Pnt> vertices;

  for (TopExp_Explorer vertexExplorer(shape, TopAbs_VERTEX); vertexExplorer.More(); vertexExplorer.Next()) {
      const auto &vertex = TopoDS::Vertex(vertexExplorer.Current());

      if (vertex.IsNull()) continue;
      vertices.insert(BRep_Tool::Pnt(vertex));
      }
  return vertices;
  }


std::vector<TopoDS_Wire> Util3D::allWiresWithin(const TopoDS_Shape& shape) {
  std::vector<TopoDS_Wire> wires;

  for (TopExp_Explorer wireExplorer(shape, TopAbs_WIRE); wireExplorer.More(); wireExplorer.Next()) {
      const auto &wire = TopoDS::Wire(wireExplorer.Current());

      if (wire.IsNull()) continue;
      wires.push_back(wire);
      }
  return wires;
  }


gp_Pnt Util3D::calcCircle(Handle(Geom_Circle) c, double param) const {
  const gp_Ax2& axis  = c->Position();
  gp_Pnt        center(axis.Location().X(), axis.Location().Y(), axis.Location().Z());
  gp_Dir        dir(axis.Direction());
  gp_Dir        xDir(axis.XDirection());
  gp_Dir        yDir(axis.YDirection());
  double        r  = c->Radius();
  double        cp = cos(param);
  double        sp = sin(param);
  gp_Pnt        ap = c->Value(param);
  gp_Pnt        rv;

  rv.SetX(center.X() + r * cp * xDir.X() + r * sp * yDir.X());
  rv.SetY(center.Y() + r * cp * xDir.Y() + r * sp * yDir.Y());
  rv.SetZ(center.Z() + r * cp * xDir.Z() + r * sp * yDir.Z());

  c->D0(param, ap);

  return rv;
  }


gp_Pnt Util3D::calcEllipse(Handle(Geom_Ellipse) e, double param) const {
  const gp_Dir& xDir   = e->XAxis().Direction();
  const gp_Dir& yDir   = e->YAxis().Direction();
  double        r      = e->MinorRadius();
  double        R      = e->MajorRadius();
  double        cp     = cos(param);
  double        sp     = sin(param);
  gp_Pnt        center = e->Location();
  gp_Pnt        ap     = e->Value(param);
  gp_Pnt        rv;


  rv.SetX(center.X() + R * cp * xDir.X() + r * sp * yDir.X());
  rv.SetY(center.Y() + R * cp * xDir.Y() + r * sp * yDir.Y());
  rv.SetZ(center.Z() + R * cp * xDir.Z() + r * sp * yDir.Z());

  e->D0(param, ap);

  return rv;
  }


gp_Pnt Util3D::calcLine(Handle(Geom_Line) l, double param) const {
  const gp_Ax1& axis  = l->Position();
  gp_Pnt        start(axis.Location().X(), axis.Location().Y(), axis.Location().Z());
  gp_Dir        dir(axis.Direction().X(), axis.Direction().Y(), axis.Direction().Z());
  gp_Pnt        ap = l->Value(param);
  gp_Pnt        rv = start;

  rv.SetX(start.X() + param * dir.X());
  rv.SetY(start.Y() + param * dir.Y());
  rv.SetZ(start.Z() + param * dir.Z());

  l->D0(param, ap);

  return rv;
  }


// S(u,v) = P + u * Dir_{u} + v * Dir_{v}
//
// (u,v) in (-infty, infty)
// times    (-infty, infty).
gp_Pnt Util3D::calcPlane(Handle(Geom_Plane) p, double param0, double param1) {
  const gp_Ax3& axis   = p->Position();
  gp_Pnt        start  = p->Location();
  gp_Dir        normal = axis.Direction();
  gp_Dir        dir0   = axis.XDirection();
  gp_Dir        dir1   = axis.YDirection();
  gp_Pnt        ap     = p->Value(param0, param1);
  gp_Pnt        rv;

  rv.SetX(start.X() + param0 * dir0.X() + param1 * dir1.X());
  rv.SetX(start.Y() + param0 * dir0.Y() + param1 * dir1.Y());
  rv.SetX(start.Z() + param0 * dir0.Z() + param1 * dir1.Z());

  p->D0(param0, param1, ap);

  return rv;
  }


TopoDS_Edge Util3D::createArc(const gp_Pnt& from, const gp_Pnt& to, double radius, bool ccw) {
  TopoDS_Edge edge;
  gp_Pnt a = from;
  gp_Pnt b = to;

  double x     = b.X() - a.X();     // get distance components
  double y     = b.Y() - a.Y();
//  double angle = atan2(y, x);       // get orientation angle
  double l     = sqrt(x*x + y*y);   // length between A and B

  if (2*radius >= l) {
     double  sweepAngle = asin(l / (2 * radius));     // find the sweep angle (actually half the sweep angle)
     double  h          = radius * cos(sweepAngle);   // triangle height from the chord to the center
     gp_Pnt  c          = gp_Pnt((double)(a.X() + x / 2 - h * (y / l))  // get center point.
                              , (double)(a.Y() + y / 2 + h * (x / l))
                              , a.Z());
     gp_Dir  dir(0, 0, ccw ? 1 : -1);
     gp_Circ rawCircle(gp_Ax2(c, dir), radius);
     edge = BRepBuilderAPI_MakeEdge(rawCircle, a, b);
     }
  return edge;
  }


Handle(AIS_Shape) Util3D::createArc(const gp_Pnt& from, const gp_Pnt& to, const gp_Pnt& center, bool ccw) {
  double r0 = center.Distance(from);
  double r1 = center.Distance(to);

//  qDebug() << "createArc: r1 - r0" << (r1 - r0);

  assert(abs(r0 - r1) < MinDelta);

  gp_Dir       dir(0, 0, ccw ? 1 : -1);
  gp_Circ      rawCircle(gp_Ax2(center, dir), r0);
  TopoDS_Edge  edge;

  if (from.Distance(to) <= gp::Resolution()) {
     edge = BRepBuilderAPI_MakeEdge(rawCircle);
     }
  else {
//     Handle(Geom_TrimmedCurve) arc = GC_MakeArcOfCircle(rawCircle, from, to, ccw);
     edge = BRepBuilderAPI_MakeEdge(rawCircle, from, to);
     }
  return new AIS_Shape(edge);
  }


Handle(AIS_Shape) Util3D::createLine(const gp_Pnt& from, const gp_Pnt& to) {
  gp_Pnt end = gp_Pnt(to);

  if (from.Distance(to) <= gp::Resolution()) end.SetX(end.X() + 0.001);
  TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(from, end);

  return new AIS_Shape(edge);
  }


Handle(AIS_Shape) Util3D::cut(const TopoDS_Shape& src, const TopoDS_Shape& tool) {
  BOPAlgo_Splitter aSplitter;
  // Setting arguments and tools
  TopTools_ListOfShape aLO; // Objects
  TopTools_ListOfShape aLT; // Tools

  aLO.Append(src);
  aLT.Append(tool);
  aSplitter.SetArguments(aLO);
  aSplitter.SetTools(aLT);

  // Set options for the algorithm
  // setting options for this algorithm is similar to setting options for GF algorithm (see "GF Usage" chapter)
  Handle(AIS_Shape) result;

  // Perform the operation
  aSplitter.Perform();
  if (aSplitter.HasErrors()) { //check error status
    return result;
  }
  result = new AIS_Shape(aSplitter.Shape());
  result->SetColor(Quantity_NOC_CYAN);

  return result;
  }


double Util3D::deburr(double v) {
  if (abs(v) < MinDelta)     return 0;
  if (abs(1 - v) < MinDelta) return 1;
  return v;
  }


gp_Dir Util3D::deburr(const gp_Dir& d) {
  return gp_Dir(deburr(d.X()), deburr(d.Y()), deburr(d.Z()));
  }


gp_Pnt Util3D::deburr(const gp_Pnt& p) {
  return gp_Pnt(deburr(p.X()), deburr(p.Y()), deburr(p.Z()));
  }


void Util3D::dumpEdges(std::vector<TopoDS_Edge> &edges) {
  for (auto& e : edges) {
      double             param0, param1;
      Handle(Geom_Curve) c = BRep_Tool::Curve(e, param0, param1);
      gp_Pnt             p0, p1;

      c->D0(param0, p0);
      c->D0(param1, p1);

      if (c->DynamicType() == STANDARD_TYPE(Geom_Line)) {
         Handle(Geom_Line) line = Handle(Geom_Line)::DownCast(c);
         const gp_Ax1&     pos  = line->Position();

         qDebug() << "line from" << p0.X() << "/" << p0.Y() << "/" << p0.Z()
                  << "\tto\t"    << p1.X() << "/" << p1.Y() << "/" << p1.Z();
         qDebug() << "\t with location:"
                  << pos.Location().X() << "/"
                  << pos.Location().Y() << "/"
                  << pos.Location().Z() << "\tand direction:"
                  << pos.Direction().X() << "/"
                  << pos.Direction().Y() << "/"
                  << pos.Direction().Z();
         }
      else if (c->DynamicType() == STANDARD_TYPE(Geom_BSplineCurve)) {
         qDebug() << "bspline-curve from" << p0.X() << "/" << p0.Y() << "/" << p0.Z()
                  << "\tto\t"             << p1.X() << "/" << p1.Y() << "/" << p1.Z();
         }
      else if (c->DynamicType() == STANDARD_TYPE(Geom_TrimmedCurve)) {
         qDebug() << "trimmed curve from" << p0.X() << "/" << p0.Y() << "/" << p0.Z()
                  << "\tto\t"             << p1.X() << "/" << p1.Y() << "/" << p1.Z();
         }
      else if (c->DynamicType() == STANDARD_TYPE(Geom_Ellipse)) {
         qDebug() << "ellipse from" << p0.X() << "/" << p0.Y() << "/" << p0.Z()
                  << "\tto\t"       << p1.X() << "/" << p1.Y() << "/" << p1.Z();
         }
      else if (c->DynamicType() == STANDARD_TYPE(Geom_BoundedCurve)) {
         qDebug() << "bounded curve from" << p0.X() << "/" << p0.Y() << "/" << p0.Z()
                  << "\tto\t"             << p1.X() << "/" << p1.Y() << "/" << p1.Z();
         }
      else if (c->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
         Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(c);
         const gp_Pnt&       pos    = circle->Position().Location();
         const gp_Dir&       dir    = circle->Position().Direction();
         double              radius = circle->Radius();

         qDebug() << "circle from" << p0.X() << "/" << p0.Y() << "/" << p0.Z()
                  << "\tto\t"      << p1.X() << "/" << p1.Y() << "/" << p1.Z();
         qDebug() << "\tand center at"
                  << pos.X() << "/"
                  << pos.Y() << "/"
                  << pos.Z() << "\t direction:"
                  << dir.X() << "/"
                  << dir.Y() << "/"
                  << dir.Z()
                  << "\t and radius:" << radius;
          }
      }
  }


void Util3D::dumpPolyline(const cavc::Polyline<double>& pl) {
  qDebug() << "dumpPolyline ...";
  for (auto& v : pl.vertexes()) {
      qDebug() << "polyline vertex:" << v.x() << "/" << v.y() << " - " << v.bulge();
      }
  qDebug() << "\tend polyLine";
  }


void Util3D::dumpVertices(const std::set<gp_Pnt>& pool) {
  for (gp_Pnt p : pool) {
      qDebug() << "found vertex at:" << p.X() << " / " << p.Y() << " / " << p.Z();
      }
  }


std::vector<Handle(AIS_Shape)> Util3D::explodeShape(const TopoDS_Shape& shape) {
  std::vector<Handle(AIS_Shape)>    shapes;
  std::vector<TopoDS_Face>          faces = allFacesWithin(shape);
  Handle(AIS_Shape)                 s;

  for (auto& f : faces) {
      s = new AIS_Shape(f);
      s->SetColor(Quantity_NOC_GREEN);
      shapes.push_back(s);
      std::vector<TopoDS_Edge> edges = allEdgesWithin(f);

      for (auto& e : edges) {
          s = new AIS_Shape(e);
          s->SetColor(Quantity_NOC_CYAN);
          shapes.push_back(s);
          }
      }
  return shapes;
  }


Handle(AIS_Shape) Util3D::fixRotation(const TopoDS_Shape& s, double angA, double angB, double angC) {
  gp_Quaternion q;
  gp_Trsf       r;

  q.SetEulerAngles(gp_Intrinsic_XYZ, deg2rad(angA)
                                   , deg2rad(angB)
                                   , deg2rad(angC));
  r.SetRotation(q);
  BRepBuilderAPI_Transform trans(r);

  trans.Perform(s, true);

  return new AIS_Shape(trans.Shape());
  }


Handle(AIS_Shape) Util3D::fixLocation(const TopoDS_Shape& s, double x, double y, double z) {
  gp_Trsf move;

  move.SetTranslation({0, 0, 0}, {x, y, z});
  BRepBuilderAPI_Transform trans(move);

  trans.Perform(s, true);

  return new AIS_Shape(trans.Shape());
  }


Handle(AIS_Shape) Util3D::genFastMove(const gp_Pnt& from, const gp_Pnt& to) {
  TopoDS_Edge       fm   = BRepBuilderAPI_MakeEdge(from, to);
  Handle(AIS_Shape) move = new AIS_Shape(fm);

  move->SetColor(Quantity_NOC_CYAN);

  return move;
  }


Handle(AIS_Shape) Util3D::genWorkArc(const gp_Pnt& from, const gp_Pnt& to, const gp_Pnt& center, bool ccw) {
  Handle(AIS_Shape) rv = createArc(from, to, center, ccw);

  rv->SetColor(Quantity_NOC_RED);
  rv->SetWidth(3);

  return rv;
  }


Handle(AIS_Shape) Util3D::genWorkLine(const gp_Pnt& from, const gp_Pnt& to) {
  TopoDS_Edge       tp   = BRepBuilderAPI_MakeEdge(from, to);
  Handle(AIS_Shape) path = new AIS_Shape(tp);

  path->SetColor(Quantity_NOC_RED);
  path->SetWidth(3);

  return path;
  }


TopoDS_Shape Util3D::intersect(const TopoDS_Shape& src, const TopoDS_Shape& tool) {
  Standard_Boolean     bRunParallel = Standard_True;
  Standard_Real        aFuzzyValue  = 2.1e-5;
  BRepAlgoAPI_Section  opCut;
  TopTools_ListOfShape aLO;
  TopTools_ListOfShape aLT;

  aLO.Append(src);
  aLT.Append(tool);
  opCut.SetArguments(aLO);
  opCut.SetTools(aLT);
  opCut.SetNonDestructive(Standard_True);
  opCut.SetCheckInverted(Standard_False);
  opCut.SetUseOBB(Standard_True);
  opCut.Build();

  return opCut.Shape();
  }


bool Util3D::isEqual(double a, double b, double minDelta) {
  if (abs(abs(a) - abs(b)) < minDelta) return true;
  return false;
  }


bool Util3D::isVertical(const gp_Dir &d) const {
  if (abs(d.X()) < MinDelta
   && abs(d.Y()) < MinDelta
   && 1 - abs(d.Z()) < MinDelta) return true;
  return false;
  }


TopoDS_Shape Util3D::loadBRep(const QString& fileName) {
  BRep_Builder builder;
  TopoDS_Shape result;

  if (!BRepTools::Read(result, fileName.toStdString().c_str(), builder))
     qDebug() << "failed to read BRep-file " << fileName;
  return result;
  }


TopoDS_Shape Util3D::loadStep(const QString& fileName) {
  STEPControl_Reader reader;

  reader.ReadFile(fileName.toStdString().c_str());
  reader.TransferRoots();

  return reader.OneShape();
  }


TopoDS_Shape Util3D::makeCube(const gp_Pnt& p0
                            , const gp_Pnt& p1) {
  gp_Pnt p[4];
  Handle(Geom_TrimmedCurve) tc[4];
  TopoDS_Edge e[4];
  TopoDS_Wire w;
  TopoDS_Face f;

  p[0] = p0;
  p[1] = gp_Pnt(p1.X(), p0.Y(), p0.Z());
  p[2] = gp_Pnt(p1.X(), p1.Y(), p0.Z());
  p[3] = gp_Pnt(p0.X(), p1.Y(), p0.Z());
  tc[ 0] = GC_MakeSegment(p[0], p[1]);
  tc[ 1] = GC_MakeSegment(p[1], p[2]);
  tc[ 2] = GC_MakeSegment(p[2], p[3]);
  tc[ 3] = GC_MakeSegment(p[3], p[0]);
  for (int i=0; i < 4; ++i)
      e[i] = BRepBuilderAPI_MakeEdge(tc[i]);
  w = BRepBuilderAPI_MakeWire(e[0], e[1], e[2],  e[3]);
  f = BRepBuilderAPI_MakeFace(w);
  gp_Vec       ev(0.0, 0.0, p1.Z() - p0.Z());
  TopoDS_Shape cube = BRepPrimAPI_MakePrism(f, ev);

  return cube;
  }


TopoDS_Shape Util3D::makeCube(const Standard_Real width
                            , const Standard_Real height
                            , const Standard_Real depth) {
  // define points
  gp_Pnt pt1(-width / 2.0, 0.0, 0.0 );
  gp_Pnt pt2(-width / 2.0, -depth / 2.0, 0.0 );
  gp_Pnt pt3(width / 2.0, -depth / 2.0, 0.0 );
  gp_Pnt pt4(width /2.0, 0.0, 0.0 );

  // define segments
  Handle_Geom_TrimmedCurve seg1 = GC_MakeSegment(pt1, pt2);
  Handle_Geom_TrimmedCurve seg2 = GC_MakeSegment(pt2, pt3);
  Handle_Geom_TrimmedCurve seg3 = GC_MakeSegment(pt3, pt4);

  // make edge
  TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(seg1);
  TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(seg2);
  TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(seg3);

  // make wire
  TopoDS_Wire wire1 = BRepBuilderAPI_MakeWire(edge1, edge2, edge3);

  //Complete Profile
  gp_Ax1  xAxis = gp::OX();
  gp_Trsf transfer;

  transfer.SetMirror(xAxis);

  BRepBuilderAPI_Transform aBRepTrsf(wire1, transfer);
  TopoDS_Shape mirroredShape = aBRepTrsf.Shape();
  TopoDS_Wire mirroredWire1 = TopoDS::Wire(mirroredShape);

  BRepBuilderAPI_MakeWire mkWire;

  mkWire.Add(wire1);
  mkWire.Add(mirroredWire1);

  TopoDS_Wire wireProfile = mkWire.Wire();

  //Body : Prism the Profile
  TopoDS_Face faceProfile = BRepBuilderAPI_MakeFace(wireProfile);
  gp_Vec prismVec(0.0, 0.0, height);

  TopoDS_Shape cube = BRepPrimAPI_MakePrism(faceProfile, prismVec);

  return cube;
  }


gp_Vec Util3D::normalOfFace(const TopoDS_Shape& face) {
  BRepAdaptor_Surface   aSurface(TopoDS::Face(face));
  Standard_Real         u1, u2, v1, v2;

  u1 = aSurface.FirstUParameter();
  u2 = aSurface.LastUParameter();
  v1 = aSurface.FirstVParameter();
  v2 = aSurface.LastVParameter();

  gp_Pnt aCenterOfFace;
  gp_Vec aVec1, aVec2;

  aSurface.D1((u1 + u2) / 2, (v1 + v2) / 2, aCenterOfFace, aVec1, aVec2);

  return aVec1 ^ aVec2;
  }


std::vector<Handle(AIS_Shape)> Util3D::pl2Shape(const cavc::Polyline<double>& pline, double commonZ, Quantity_Color c, double width) {
  std::vector<Handle(AIS_Shape)> segments;
  const double                   arcApproxError = 0.005;
  auto                           visitor        = [&](std::size_t i, std::size_t j) {
    Handle(AIS_Shape) is;
    const auto&       v1 = pline[i];
    const auto&       v2 = pline[j];

    if (v1.bulgeIsZero() || fuzzyEqual(v1.pos(), v2.pos())) {
       is = createLine(gp_Pnt(v1.x(), v1.y(), commonZ), gp_Pnt(v2.x(), v2.y(), commonZ));
       }
    else {
       auto arc = arcRadiusAndCenter(v1, v2);

       if (arc.radius < arcApproxError + cavc::utils::realThreshold<double>()) {
          qDebug() << "invalid circle segment!";
          }
       else {
          is = createArc(gp_Pnt(v1.x(), v1.y(), commonZ)
                       , gp_Pnt(v2.x(), v2.y(), commonZ)
                       , gp_Pnt(arc.center.x(), arc.center.y(), commonZ)
                       , v1.bulgeIsPos());
          }
       }
    if (!is.IsNull()) {
       is->SetWidth(width);
       is->SetColor(c);
       segments.push_back(is);
       }
    return true;
    };
  pline.visitSegIndices(visitor);

  return segments;
  }


TopoDS_Shape Util3D::pl2Wire(const cavc::Polyline<double>& pline, double commonZ) {
  Handle(TopTools_HSequenceOfShape) segments = new TopTools_HSequenceOfShape;
  Handle(TopTools_HSequenceOfShape) wires;
  auto                              visitor  = [&](std::size_t i, std::size_t j) {
    Handle(AIS_Shape) is;
    const auto&       v1 = pline[i];
    const auto&       v2 = pline[j];

    if (v1.bulgeIsZero() || fuzzyEqual(v1.pos(), v2.pos())) {
       is = createLine(gp_Pnt(v1.x(), v1.y(), commonZ), gp_Pnt(v2.x(), v2.y(), commonZ));
       }
    else {
       auto arc = arcRadiusAndCenter(v1, v2);

       if (arc.radius < MinDelta + cavc::utils::realThreshold<double>()) {
          qDebug() << "invalid circle segment!";
          }
       else {
          is = createArc(gp_Pnt(v1.x(), v1.y(), commonZ)
                       , gp_Pnt(v2.x(), v2.y(), commonZ)
                       , gp_Pnt(arc.center.x(), arc.center.y(), commonZ)
                       , v1.bulgeIsPos());
          }
       }
    if (!is.IsNull()) segments->Append(is->Shape());

    return true;
    };
  pline.visitSegIndices(visitor);
  ShapeAnalysis_FreeBounds fb;

  fb.ConnectEdgesToWires(segments, MinDelta, false, wires);

  return wires->First();
  }


cavc::Polyline<double> Util3D::toPolyline(TopoDS_Shape wire) {
  std::vector<TopoDS_Edge> edges = allEdgesWithin(wire);

  return toPolyline(edges);
  }


cavc::Polyline<double> Util3D::toPolyline(const std::vector<TopoDS_Edge>& edges) {
  cavc::Polyline<double>          polyLine;
  double                          param0, param1, bulge;
  const double                    minDelta = 0.0001;
  gp_Pnt                          nP0, nP1, p0, p1, first, start, end;

  for (int i=0; i < edges.size(); ++i) {
      TopoDS_Edge        edge = edges.at(i);
      Handle(Geom_Curve) c    = BRep_Tool::Curve(edge, param0, param1);

      c->D0(param0, p0);
      c->D0(param1, p1);
      if (!i) {
         if (edges.size() > 1) {
            double             nParam0, nParam1;
            TopoDS_Edge        next = edges.at(i+1);
            Handle(Geom_Curve) nc   = BRep_Tool::Curve(next, nParam0, nParam1);

            nc->D0(nParam0, nP0);
            nc->D0(nParam1, nP1);

            // matching vertex between both edges gives orientation of both
            // this check needs to be done on first edge only
            if (p0.IsEqual(nP0, minDelta) || p0.IsEqual(nP1, minDelta)) {
               start = p1;
               end   = p0;
               }
            else {
               start = p0;
               end   = p1;
               }
            }
         else {
            start = p0;
            end   = p1;
            }
         first = start;
         }
      else {
         // we got start vertex from last run, so only need to determine end/next start
         if (start.IsEqual(p0, minDelta)) end = p1;
         else                             end = p0;
         }

      if (c->DynamicType() == STANDARD_TYPE(Geom_Line)) {
         bulge = 0;
         qDebug() << "line from" << start.X() << "/" << start.Y()
                  << " to "   << end.X() << "/" << end.Y();
         polyLine.addVertex(abs(start.X()) < minDelta ? 0 : start.X()
                          , abs(start.Y()) < minDelta ? 0 : start.Y()
                          , bulge);
         }
      else if (c->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
         Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(c);
         int                 orient = edge.Orientation();
         bool                ccw    = circle->Axis().Direction().Z() < 0;

         if (orient) ccw = !ccw;
         bulge = tan((param1 - param0) / 4);
         if ((param1 - param0) > M_PI) {
            double tmp = (param1 - param0) / 2;

            c->D0(tmp, p0);
            bulge = tan((tmp - param0) / 4);
            if (ccw) bulge *= -1;

            qDebug() << "arc0 p0:" << start.X() << "/" << start.Y()
                     << " - p1:"   << p0.X() << "/" << p0.Y()
                     << "angle: " << (tmp - param0)
                     << "\tbulge:" << bulge << "\tccw:" << (ccw ? "TRUE" : "FALSE") << "orient:" << orient;
            qDebug() << "arc1 p0:" << start.X() << "/" << start.Y()
                     << " - p1:"   << end.X() << "/" << end.Y()
                     << "angle: " << (param1 - tmp)
                     << "\tbulge:" << bulge << "\tccw:" << (ccw ? "TRUE" : "FALSE") << "orient:" << orient;

            polyLine.addVertex(abs(start.X()) < minDelta ? 0 : start.X()
                             , abs(start.Y()) < minDelta ? 0 : start.Y()
                             , bulge);

            polyLine.addVertex(abs(p0.X()) < minDelta ? 0 : p0.X()
                             , abs(p0.Y()) < minDelta ? 0 : p0.Y()
                             , bulge);
            }
         else {
            if (ccw) bulge *= -1;

            qDebug() << "arc0 p0:" << start.X() << "/" << start.Y()
                     << " - p1:"   << end.X() << "/" << end.Y()
                     << "angle: " << (param1 - param0) << "\tbulge:"
                     << bulge << "\tccw:" << (ccw ? "TRUE" : "FALSE") << "orient:" << orient;

            polyLine.addVertex(abs(start.X()) < minDelta ? 0 : start.X()
                             , abs(start.Y()) < minDelta ? 0 : start.Y()
                             , bulge);
            }
         }
      start = end;
      }
  if (first.IsEqual(p0, minDelta) || first.IsEqual(p1, minDelta))
     polyLine.isClosed() = true;
  else
     polyLine.addVertex(end.X(), end.Y(), bulge);
  return polyLine;
  }


double const Util3D::MinDelta = 0.001;
