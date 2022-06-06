/* 
 * **************************************************************************
 * 
 *  file:       gocircle.cpp
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
#include "gocircle.h"
#include "core.h"
#include "kuteCAM.h"
#include "util3d.h"
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRep_Tool.hxx>
#include <ElCLib.hxx>
#include <gp_Circ.hxx>
#include <Geom_Line.hxx>
#include <StdFail_NotDone.hxx>
#include <TopoDS_Edge.hxx>
#include <QVector3D>
#include <QDebug>


// constructor for 3D circle out of 3 points
// found this on stackoverflow written by Mark
// https://stackoverflow.com/questions/13977354/build-circle-from-3-points-in-3d-space-implementation-in-c-or-c
// changed it to use Qt and opencascade
GOCircle::GOCircle(const gp_Pnt& from, const gp_Pnt& to, const gp_Pnt& midPoint)
 : GraphicObject(GraphicType::GTCircle, from, to) {
  QVector3D p1(from.X(), from.Y(), from.Z());  // triangle "edges"
  QVector3D p2(to.X(), to.Y(), to.Z());
  QVector3D p3(midPoint.X(), midPoint.Y(), midPoint.Z());
  QVector3D t   = p2 - p1;
  QVector3D u   = p3 - p1;
  QVector3D v   = p3 - p2;
  QVector3D w   = QVector3D::crossProduct(t, u);  // triangle normal
  double    wsl = QVector3D::dotProduct(w, w);
  // TODO: if (wsl<10e-14) return false; // area of the triangle is too small (you may additionally check the points for colinearity if you are paranoid)

  // helpers
  double iwsl2 = 1.0 / (2.0 * wsl);
  double tt    = QVector3D::dotProduct(t, t);
  double uu    = QVector3D::dotProduct(u, u);

  // result circle
  QVector3D cc = p1 + (u * tt * (QVector3D::dotProduct(u, v))
                    -  t * uu * (QVector3D::dotProduct(t, v))) * iwsl2;

  centerPnt = gp_Pnt(cc.x(), cc.y(), cc.z());
  r         = sqrt(tt * uu * (QVector3D::dotProduct(v, v)) * iwsl2 * 0.5);
  QVector3D circAxis = w / sqrt(wsl);

  axis = gp_Dir(circAxis.x(), circAxis.y(), -circAxis.z());
  createCircle();
  }


GOCircle::GOCircle(Handle(Geom_Circle) hc, double p0, double p1)
 : GraphicObject(GraphicType::GTCircle, hc->Value(p0), hc->Value(p1)) {
  curve = hc;
  this->p0 = p0;
  this->p1 = p1;
  this->r  = hc->Radius();
  this->centerPnt = hc->Position().Location();
  this->axis      = hc->Position().Direction();
  }


GOCircle::GOCircle(const QString& s)
 : GraphicObject(GraphicType::GTCircle, s) {
  QStringList sl    = s.split(";");
  QStringList subSL = sl.at(3).split("/");
  bool        ok    = false;
  double      x, y, z;

  x = subSL.at(0).toDouble(&ok);
  y = subSL.at(1).toDouble(&ok);
  z = subSL.at(2).toDouble(&ok);
  if (!kute::isEqual(z, startPoint().Z())
   && !kute::isEqual(z, endPoint().Z()))
     z = startPoint().Z();
  centerPnt = gp_Pnt(x, y, z);

  subSL = sl.at(4).split("/");
  x = subSL.at(0).toDouble(&ok);
  y = subSL.at(1).toDouble(&ok);
  z = subSL.at(2).toDouble(&ok);
  axis = gp_Dir(x, y, z);

  r = sl.at(5).toDouble(&ok);
  createCircle();
  }


gp_Pnt GOCircle::center() const {
  return centerPnt;
  }


void GOCircle::createCircle() {
  gp_Circ     rawCircle(gp_Ax2(centerPnt, axis), r);
  TopoDS_Edge tmp;

  try {
      if (kute::isEqual(startPoint(), endPoint())) {
         qDebug() << "create full circle ...";
         tmp = BRepBuilderAPI_MakeEdge(rawCircle);
         }
      else {
         qDebug() << "create some arc ...";
         tmp = BRepBuilderAPI_MakeEdge(rawCircle, startPoint(), endPoint());
         }
      }
  catch (const StdFail_NotDone& e) {
      qDebug() << "creation of circle failed with: " << e.GetMessageString();
      }
  curve = BRep_Tool::Curve(tmp, p0, p1);
  gp_Pnt s0 = curve->Value(p0);
  gp_Pnt e0 = curve->Value(p1);

  if (!kute::isEqual(s0, startPoint())) {
     qDebug() << "failed to create circle - startpoints don't match!";
     setStartPoint(s0);
     }
  if (!kute::isEqual(e0, endPoint())) {
     qDebug() << "failed to create circle - endpoints don't match!";
     setEndPoint(e0);
     }
  }


Handle(Geom_Curve) GOCircle::endTangent(double length, double* param0, double* param1) const {
  gp_Circ     rawCircle(gp_Ax2(centerPnt, axis), r);
  TopoDS_Edge eRad = BRepBuilderAPI_MakeEdge(endPoint(), centerPnt);
  double      p0, p1;
  Handle(Geom_Curve) gRad    = BRep_Tool::Curve(eRad, p0, p1);
  Handle(Geom_Line)  lRad    = Handle(Geom_Line)::DownCast(gRad);
  gp_Dir             dirRad  = lRad->Position().Direction();
  gp_Dir             dirZ    = axis;
  gp_Dir             normRad(dirRad.Y() * dirZ.Z() - dirRad.Z() * dirZ.Y()
                           , dirRad.Z() * dirZ.X() - dirRad.X() * dirZ.Z()
                           , dirRad.X() * dirZ.Y() - dirRad.Y() * dirZ.X());
  Handle(Geom_Line)  tangent = new Geom_Line(gp_Ax1(endPoint(), normRad));
  TopoDS_Edge        newEdge = BRepBuilderAPI_MakeEdge(tangent, 0, length);
  Handle(Geom_Curve) rv      = BRep_Tool::Curve(newEdge, p0, p1);

  if (param0) *param0 = p0;
  if (param1) *param1 = p1;

  return rv;
  }


GraphicObject* GOCircle::extendEnd(double length) {
  return this;
  }


GraphicObject* GOCircle::extendStart(double length) {
  return this;
  }


GraphicObject* GOCircle::invert() {
  swapEndPoints();
  axis = axis.Reversed();
  createCircle();

  return this;
  }


gp_Pnt GOCircle::midPoint() const {
  return curve->Value((p1 - p0) / 2);
  }


bool GOCircle::isCCW() const {
  return axis.Z() > 0;
  }


double GOCircle::radius() const {
  return r;
  }


void GOCircle::setZ(double z) {
  gp_Pnt s(startPoint());
  gp_Pnt e(endPoint());
  gp_Pnt c(centerPnt);
  s.SetZ(z);
  e.SetZ(z);
  c.SetZ(z);
  centerPnt = c;
  createCircle();
  }


void GOCircle::setEndPoint(const gp_Pnt &p) {
  Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(curve);

  if (!circle.IsNull())
     p1 = ElCLib::Parameter(circle->Circ(), p);
  GraphicObject::setEndPoint(p);
  }


void GOCircle::setStartPoint(const gp_Pnt &p) {
  Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(curve);

  if (!circle.IsNull())
     p0 = ElCLib::Parameter(circle->Circ(), p);
  GraphicObject::setStartPoint(p);
  }


GraphicObject* GOCircle::split() {
  double px1   = p1;
  double px0   = (p1 - p0) / 2;
  gp_Pnt split = curve->Value(px0);

  setEndPoint(split);
  createCircle();

  return new GOCircle(Handle(Geom_Circle)::DownCast(curve), px0, px1);
  }

Handle(Geom_Curve) GOCircle::startTangent(double length, double* param0, double* param1) const {
  gp_Circ     rawCircle(gp_Ax2(centerPnt, axis), r);
  TopoDS_Edge eRad = BRepBuilderAPI_MakeEdge(startPoint(), centerPnt);
  double      p0, p1;
  Handle(Geom_Curve) gRad    = BRep_Tool::Curve(eRad, p0, p1);
  Handle(Geom_Line)  lRad    = Handle(Geom_Line)::DownCast(gRad);
  gp_Dir             dirRad  = lRad->Position().Direction();
  gp_Dir             dirZ    = axis;
  gp_Dir             normRad(dirRad.Y() * dirZ.Z() - dirRad.Z() * dirZ.Y()
                           , dirRad.Z() * dirZ.X() - dirRad.X() * dirZ.Z()
                           , dirRad.X() * dirZ.Y() - dirRad.Y() * dirZ.X());
  Handle(Geom_Line)  tangent = new Geom_Line(gp_Ax1(startPoint(), normRad));
  TopoDS_Edge        newEdge = BRepBuilderAPI_MakeEdge(tangent, 0, -length);
  Handle(Geom_Curve) rv      = BRep_Tool::Curve(newEdge, p0, p1);

  if (param0) *param0 = p0;
  if (param1) *param1 = p1;

  return rv;
  }


Handle(AIS_Shape) GOCircle::toShape(double z) {
  if (!kute::isEqual(abs(z), 0))
     centerPnt.SetZ(z);
  gp_Circ     rawCircle(gp_Ax2(centerPnt, axis), r);
  TopoDS_Edge edge;

  if (startPoint().Distance(endPoint()) <= gp::Resolution()) {
     edge = BRepBuilderAPI_MakeEdge(rawCircle);
     }
  else {
     gp_Pnt start(startPoint());
     gp_Pnt end(endPoint());
     if (!kute::isEqual(abs(z), 0)) {
        start.SetZ(z);
        centerPnt.SetZ(z);
        end.SetZ(z);
        }
     double p0 = ElCLib::Parameter(rawCircle, start);
     double p1 = ElCLib::Parameter(rawCircle, end);
     BRepBuilderAPI_MakeEdge me(rawCircle, p0, p1);

     if (!me.IsDone()) {
        int error = me.Error();

        qDebug() << "error #" << error;
        }
     else edge = me.Edge();
     }
  return new AIS_Shape(edge);
  }


QString GOCircle::toString() const {
  QString rv = GraphicObject::toString();

  rv += QString(";%1/%2/%3;%4/%5/%6;%7").arg(centerPnt.X(), 0, 'f', 4)
                                        .arg(centerPnt.Y(), 0, 'f', 4)
                                        .arg(centerPnt.Z(), 0, 'f', 4)
                                        .arg(axis.X(), 0, 'f', 4)
                                        .arg(axis.Y(), 0, 'f', 4)
                                        .arg(axis.Z(), 0, 'f', 4)
                                        .arg(r, 0, 'f', 4);
  return rv;
  }
