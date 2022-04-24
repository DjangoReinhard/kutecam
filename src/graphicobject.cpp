/* 
 * **************************************************************************
 * 
 *  file:       graphicobject.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    23.4.2022 by Django Reinhard
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
#include "graphicobject.h"
#include "core.h"
#include "util3d.h"
#include <ElCLib.hxx>
#include <TopoDS_Edge.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <QDebug>


GraphicObject::GraphicObject(GraphicType gt, const gp_Pnt& from, const gp_Pnt& to)
 : gType(gt)
 , fromPnt(from)
 , toPnt(to) {
  }


GraphicObject::GraphicObject(GraphicType gt, const QString& source)
 : gType(gt) {
  QStringList sl    = source.split(";");
  bool        ok    = false;
  int         type  = sl.at(0).toInt(&ok);

  if (type != gt) throw std::domain_error("invalid call! - source type does not match");
  QStringList subSL = sl.at(1).split("/");
  double      v     = subSL.at(0).toDouble(&ok);

  if (ok) fromPnt.SetX(v);
  v = subSL.at(1).toDouble(&ok);
  if (ok) fromPnt.SetY(v);
  v = subSL.at(2).toDouble(&ok);
  if (ok) fromPnt.SetZ(v);

  subSL = sl.at(2).split("/");
  int n = subSL.at(2).indexOf("|");

  if (n) subSL[2] = subSL.at(2).mid(0, n);

  v = subSL.at(0).toDouble(&ok);
  if (ok) toPnt.SetX(v);
  v = subSL.at(1).toDouble(&ok);
  if (ok) toPnt.SetY(v);
  v = subSL.at(2).toDouble(&ok);
  if (ok) toPnt.SetZ(v);
  }


gp_Pnt GraphicObject::startPoint() const {
  return fromPnt;
  }


gp_Pnt GraphicObject::endPoint() const {
  return toPnt;
  }


void GraphicObject::swapEndPoints() {
  gp_Pnt tmp = fromPnt;

  fromPnt = toPnt;
  toPnt   = tmp;
  }


void GraphicObject::setStartPoint(const gp_Pnt &p) {
  Handle(Geom_Line) line = Handle(Geom_Line)::DownCast(curve);

  if (!line.IsNull())
     p0 = ElCLib::Parameter(line->Lin(), p);
  fromPnt = p;
  }


void GraphicObject::setEndPoint(const gp_Pnt &p) {
  Handle(Geom_Line) line = Handle(Geom_Line)::DownCast(curve);

  if (!line.IsNull())
     p1 = ElCLib::Parameter(line->Lin(), p);
  toPnt = p;
  }


void GraphicObject::setZ(double z) {
  }


GraphicObject* GraphicObject::split() {
  return nullptr;
  }


QString GraphicObject::sType() const {
  switch (gType) {
    case GTInvalid: return "Invalid";
    case GTLine:    return "Line";
    case GTCircle:  return "Circle";
    case GTContour: return "Contour";
    case GTPocket:  return "Pocket";
    default: break;
    }
  return "Unknown";
  }


Handle(AIS_Shape) GraphicObject::toShape(double z) {
  gp_Pnt start(fromPnt);
  gp_Pnt end(toPnt);

  if (Core().helper3D()->isEqual(fromPnt, end)) end.SetX(end.X() + 0.001);
  if (!Core().helper3D()->isEqual(z, 0)) {
     start.SetZ(z);
     end.SetZ(z);
     }
  TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(start, end);

  return new AIS_Shape(edge);
  }


QString GraphicObject::toString() const {
  return QString("%1;%2/%3/%4;%5/%6/%7").arg(gType)
                                        .arg(fromPnt.X(), 0, 'f', 4)
                                        .arg(fromPnt.Y(), 0, 'f', 4)
                                        .arg(fromPnt.Z(), 0, 'f', 4)
                                        .arg(toPnt.X(), 0, 'f', 4)
                                        .arg(toPnt.Y(), 0, 'f', 4)
                                        .arg(toPnt.Z(), 0, 'f', 4);
  }


int GraphicObject::type() const {
  return gType;
  }
