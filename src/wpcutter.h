/* 
 * **************************************************************************
 * 
 *  file:       wpcutter.h
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
#ifndef WPCUTTER_H
#define WPCUTTER_H
#include <AIS_Shape.hxx>
#include <TopoDS_Edge.hxx>
class Geom_Curve;
class GOContour;
class QObject;


class WPCutter
{
public:
  WPCutter(Handle(AIS_Shape) workPiece, QObject* parent = nullptr);

  std::vector<GOContour*> processShape(TopoDS_Shape curve, const gp_Pnt& center);

protected:
  void determineCutShape();
  int  checkCIntersection(const gp_Pnt& center, double radius);
  int  checkLIntersection(const gp_Pnt& p0, const gp_Pnt& p1);
  int  clipRegion(const gp_Pnt& p);
  TopoDS_Edge cutCircleCircle(Handle(Geom_Curve) c, double first, double last, int orientation, const gp_Pnt& endPoint, bool showCutPoints = false);
  TopoDS_Edge cutCircleRect(Handle(Geom_Curve) c, double first, double last, int orientation, const gp_Pnt& endPoint);
  TopoDS_Edge cutCurve(Handle(Geom_Curve) c, double first, double last, int orientation, const gp_Pnt& endPoint, bool showCutPoints = false);
  TopoDS_Edge cutLineCircle(Handle(Geom_Curve) c, double first, double last, int orientation, const gp_Pnt& endPoint);
  TopoDS_Edge cutLineRect(Handle(Geom_Curve) c, double first, double last, int orientation, const gp_Pnt& endPoint);

  static const int Inside = 0;
  static const int Left   = 1;
  static const int Right  = 2;
  static const int Bottom = 4;
  static const int Top    = 8;

private:
  Handle(AIS_Shape) workPiece;
  TopoDS_Shape      cutShape;
  Bnd_Box           bbWP;
  gp_Pnt            cMaster;
  gp_Dir            dMaster;
  double            rMaster;
  bool              isRound;
  };

#endif // WPCUTTER_H
