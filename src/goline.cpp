/* 
 * **************************************************************************
 * 
 *  file:       goline.cpp
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
#include "goline.h"
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRep_Tool.hxx>
#include <ElCLib.hxx>
#include <Geom_Line.hxx>
#include <QDebug>


GOLine::GOLine(const gp_Pnt& from, const gp_Pnt& to)
 : GraphicObject(GraphicType::GTLine, from, to) {
  createLine();
  }


GOLine::GOLine(const QString& s)
 : GraphicObject(GraphicType::GTLine, s) {
  createLine();
  }


void GOLine::createLine() {
  edge  = BRepBuilderAPI_MakeEdge(startPoint(), endPoint());
  curve = BRep_Tool::Curve(edge, p0, p1);
  }


GraphicObject* GOLine::extendEnd(double length) {
  double pN = p1 + length;

  setEndPoint(curve->Value(pN));
  createLine();

  return this;
  }


gp_Dir GOLine::direction() const {
  Handle(Geom_Line) line = Handle(Geom_Line)::DownCast(curve);

  return line->Position().Direction();
  }


GraphicObject* GOLine::extendStart(double length) {
  double delta = p0 - p1;
  double pN = delta < 0 ? p0 - length : p0 + length;

  setStartPoint(curve->Value(pN));
  createLine();

  return this;
  }


GraphicObject* GOLine::invert() {
  swapEndPoints();
  createLine();

  return this;
  }


gp_Pnt GOLine::midPoint() const {
  return curve->Value((p1 - p0) / 2);
  }


void GOLine::setEndPoint(const gp_Pnt &p) {
  Handle(Geom_Line) line = Handle(Geom_Line)::DownCast(curve);

  if (!line.IsNull())
     p1 = ElCLib::Parameter(line->Lin(), p);
  GraphicObject::setEndPoint(p);
  }


void GOLine::setStartPoint(const gp_Pnt &p) {
  Handle(Geom_Line) line = Handle(Geom_Line)::DownCast(curve);

  if (!line.IsNull())
     p0 = ElCLib::Parameter(line->Lin(), p);
  GraphicObject::setStartPoint(p);
  }


void GOLine::setZ(double z) {
  gp_Pnt s(startPoint());
  gp_Pnt e(endPoint());

  s.SetZ(z);
  e.SetZ(z);

  setStartPoint(s);
  setEndPoint(e);
  createLine();
  }


GraphicObject* GOLine::split() {
  gp_Pnt end   = endPoint();
  gp_Pnt split = curve->Value((p1 - p0) / 2);

  setEndPoint(split);

  return new GOLine(split, end);
  }
