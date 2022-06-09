/* 
 * **************************************************************************
 * 
 *  file:       gocontour.h
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
#ifndef GOCONTOUR_H
#define GOCONTOUR_H
#include "graphicobject.h"
#include "kuteCAM.h"
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <vector>


class GOContour : public GraphicObject
{
public:
  explicit GOContour(const gp_Pnt& center, int order = 0);
  virtual ~GOContour() = default;

  virtual void              dump() const override;
  virtual GraphicObject*    extendStart(double length) override;
  virtual GraphicObject*    extendEnd(double length) override;
  virtual GraphicObject*    invert() override;
  virtual gp_Pnt            midPoint() const override;
  virtual Handle(AIS_Shape) toShape(double z = 0) override;
  virtual QString           toString() const override;
  TopoDS_Shape              toWire(double z = 0);

  bool                         add(GraphicObject* o);
  bool                         add(GOContour* other);
  double                       a0() const;
  double                       a1() const;
  gp_Pnt                       centerPoint() const;
  gp_Pnt                       changeStart2Close(const gp_Pnt& p);
  double                       distStart() const;
  double                       distEnd() const;
  GraphicObject&               extendBy(double length);
  bool                         isClosed() const;
  int                          order() const;
  int                          size() const;
  std::vector<GraphicObject*>& segments();
  void                         setContour(TopoDS_Shape contour);
  std::vector<GraphicObject*>& simplify(double z, bool cw = true);

  static GraphicObject*        occ2GO(TopoDS_Edge e, double defZ = 0);

protected:
  explicit GOContour(const QString& source);

private:
  std::vector<GraphicObject*> segs;
  gp_Pnt                      center;
  int                         level;

  friend class Util3D;
  };


extern bool compContour(GOContour* left, GOContour* right);

#endif // GOCONTOUR_H
