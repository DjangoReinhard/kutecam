/* 
 * **************************************************************************
 * 
 *  file:       selectioninfohandler.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    26.5.2022 by Django Reinhard
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
#include "selectioninfohandler.h"
#include "core.h"
#include "occtviewer.h"
#include "util3d.h"
#include <BRep_Tool.hxx>
#include <QDebug>


SelectionInfoHandler::SelectionInfoHandler(QObject *parent)
 : QObject(parent) {
  }


void SelectionInfoHandler::evalSelection() {
  std::vector<TopoDS_Shape> selection = Core().view3D()->selection();

  for (auto& s : selection) {
      if (s.ShapeType() == TopAbs_FACE) {
         qDebug() << "selection is face ...";
         exploreFace(TopoDS::Face(s));
         }
      else if (s.ShapeType() == TopAbs_EDGE) {
         qDebug() << "selection is edge ...";
         exploreEdge(TopoDS::Edge(s));
         }
      else {
         qDebug() << "selection is unknown: " << s.ShapeType();
         }
      }
  }


void SelectionInfoHandler::exploreCurve(opencascade::handle<Geom_Curve> c, double first, double last) {
  if (c->DynamicType() == STANDARD_TYPE(Geom_Line)) {
     Handle(Geom_Line) line = Handle(Geom_Line)::DownCast(c);
     gp_Pnt p0 = c->Value(first);
     gp_Pnt p1 = c->Value(last);

     qDebug() << "edge is a straight line from"
              << p0.X() << " / "
              << p0.Y() << " / "
              << p0.Z() << "   to   "
              << p1.X() << " / "
              << p1.Y() << " / "
              << p1.Z();
     }
  else if (c->DynamicType() == STANDARD_TYPE(Geom_Ellipse)) {
     Handle(Geom_Ellipse) ellipse = Handle(Geom_Ellipse)::DownCast(c);
     //     qDebug() << "edge is an arc (ellipse)";
     // parameter: gp_Pnt loc  = c->Location();
     //            gpAx1  axis = c->Axis();
     //            double rMax = c->MajorRadius();
     //            double rMin = c->MinorRadius();
     qDebug() << "edge is Ellipse with center at"
              << ellipse->Location().X() << " / "
              << ellipse->Location().Y() << " / "
              << ellipse->Location().Z()
              << "   and R(major): " << ellipse->MajorRadius()
              << "   and R(minor): " << ellipse->MinorRadius();
     }
  else if (c->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
     Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(c);
     const gp_Pnt&       pos    = circle->Position().Location();
     const gp_Dir&       dir    = circle->Position().Direction();
     double              radius = circle->Radius();

     qDebug() << "edge is a circle with center at"
              << pos.X() << " / "
              << pos.Y() << " / "
              << pos.Z() << "\t and radius:" << radius;
     }
  else {
     qDebug() << "edge is unknown curve (" << c->DynamicType()->Name() << ")";
     }
  }


void SelectionInfoHandler::exploreEdge(TopoDS_Edge e) {
  if (BRep_Tool::IsGeometric(e)) {
     double             first, last;
     Handle(Geom_Curve) c = BRep_Tool::Curve(e, first, last);

     exploreCurve(c, first, last);
     }
  else {
     qDebug() << "OUPS - edge from selected face is NOT geometric?!?";
     }
  }


void SelectionInfoHandler::exploreFace(TopoDS_Face f) {
  std::vector<TopoDS_Edge> edges = Core().helper3D()->allEdgesWithin(f);

  for (auto e : edges) {
      exploreEdge(e);
      }
  }
