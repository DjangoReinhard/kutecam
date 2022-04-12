/* 
 * **************************************************************************
 * 
 *  file:       PointInPolygon.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    1.4.2022 by Django Reinhard
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
#ifndef POINTINPOLYGON_H
#define POINTINPOLYGON_H

//! Make conversion's easy:
//#define toRadians M_PI/180.0
//#define toDegrees (180.0/M_PI)

//! Opencascade.
#include <gp_Pnt.hxx>
#include <gp_Circ.hxx>
#include <AIS_Shape.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <TopoDS_Edge.hxx>
#include <GeomAdaptor_Curve.hxx>

#include <gce_MakeCirc.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <Geom_Circle.hxx>
#include <BRep_Tool.hxx>

#include <vector>
#include <math.h>


//! Sketch class, contains how to build compound's.
//! https://github.com/grotius-cnc/occt-samples-qopenglwidget/blob/master/OcctQtWidget/OcctQtLib/libsketch/sketchcontrol.cpp

/*
Usage:

    #include "PointInPolygon.h"

    int InPolygon=0;
    PointInPolygon({20,50,0}).TubeShape({0,0,0},100,InPolygon);

    std::cout<<"InPolygon 50,50,0 : "<<InPolygon<<std::endl;

 * */
class PointInPolygon
{
public:
  gp_Pnt P;

  //! Point to check              : P
  //! Assuming the raw material is a tube shape instead of a box shape.
  //! Raw material centerpoint    : CP
  //! Raw material radius         : R
  PointInPolygon(gp_Pnt _P) : P(_P){}


  //! Tube shape, top view.
  //! Tube centerpoint : CP
  //! If InPolygon = 1, point is inside polygon.
  void TubeShape(gp_Pnt CP, double radius, int& InPolygon) {
    std::vector<gp_Pnt> Pvec;
    gp_Dir              dir(0,0,1);
    gp_Circ             circle(gp_Ax2(CP, dir),radius);
    TopoDS_Edge         aEdge = BRepBuilderAPI_MakeEdge(circle);

    //! Workaround to get the curve "Value" 0 to 2 Pi Radians.
    double              First, Last;
    Handle(Geom_Curve)  myCurve = BRep_Tool::Curve(aEdge, First, Last);
    Handle(Geom_Circle) myCircleCurve = Handle(Geom_Circle)::DownCast(myCurve);

    for (double i=0; i < (2 * M_PI); i += 0.01 * M_PI) {
        Pvec.push_back(myCircleCurve->Value(i));
        //! std::cout<<"Circle contour points x:"<<myCircleCurve->Value(i).X()<<" y:"<<myCircleCurve->Value(i).Y()<<" z:"<<myCircleCurve->Value(i).Z()<<std::endl;
        }
    InPolygon = PIP(Pvec, P);
    }


  //! For a box or other shape, pass points.
  void CustomShape(std::vector<gp_Pnt> PointVec, int &InPolygon) {
    InPolygon=PIP(PointVec,P);
    }

private:
  //! Point in Polygon (PIP)
  int PIP (std::vector<gp_Pnt> polygon, gp_Pnt point) {
    int n = polygon.size();

    //! pos = array of x,y positions of the polygon to investegate
    //! x,y = point to check inside the polygon
    //! c = 1 if true
    int i, j, c = 0;

    for (i = 0, j = n - 1; i < n; j = i++) {
        if (((polygon.at(i).Y() > point.Y()) != (polygon.at(j).Y() > point.Y()))
            &&
             (point.X() < (polygon.at(j).X() - polygon.at(i).X())
                        * (point.Y()         - polygon.at(i).Y())
                        / (polygon.at(j).Y() - polygon.at(i).Y())
                        +  polygon.at(i).X())) {
           c = !c;
           }
        }
    return c;
    }
  };
#endif // POINTINPOLYGON_H
