/* 
 * **************************************************************************
 * 
 *  file:       graphicobject.h
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
#ifndef GRAPHICOBJECT_H
#define GRAPHICOBJECT_H
#include <gp_Pnt.hxx>
#include <AIS_Shape.hxx>
#include <Geom_Curve.hxx>
#include <TopoDS_Edge.hxx>
#include <QString>


enum GraphicType
{
  GTInvalid
, GTLine
, GTCircle
, GTContour
, GTPocket
, GTUnknown
  };


class GraphicObject
{
public:
  explicit GraphicObject(GraphicType gt, const gp_Pnt& from, const gp_Pnt& to);
  virtual ~GraphicObject() = default;

  gp_Pnt  startPoint() const;
  gp_Pnt  endPoint()   const;
  QString sType()      const;
  int     type()       const;

  virtual GraphicObject*    extendStart(double length) = 0;
  virtual GraphicObject*    extendEnd(double length)   = 0;
  virtual GraphicObject*    invert()                   = 0;
  virtual void              setZ(double z);
  virtual GraphicObject*    split();
  virtual Handle(AIS_Shape) toShape(double z = 0);
  virtual QString           toString() const;

protected:
  explicit GraphicObject(GraphicType gt, const QString& source);

  void setStartPoint(const gp_Pnt& p);
  void setEndPoint(const gp_Pnt& p);
  void swapEndPoints();

  TopoDS_Edge        edge;
  Handle(Geom_Curve) curve;
  double             p0, p1;

private:
  GraphicType gType;
  gp_Pnt      fromPnt;
  gp_Pnt      toPnt;
  int         relPos;
  int         relStart;
  int         relEnd;
  };
#endif // GRAPHICOBJECT_H
