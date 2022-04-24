/* 
 * **************************************************************************
 * 
 *  file:       gocircle.h
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
#ifndef GOCIRCLE_H
#define GOCIRCLE_H
#include "graphicobject.h"
#include <Geom_Curve.hxx>


class GOCircle : public GraphicObject
{
public:
  explicit GOCircle(const gp_Pnt& from, const gp_Pnt& to, const gp_Pnt& midPnt);
  virtual ~GOCircle() = default;

  gp_Pnt                    center() const;
  Handle(Geom_Curve)        endTangent(double length, double* param0 = nullptr, double* param1 = nullptr) const;
  bool                      isCCW() const;
  double                    radius() const;
  Handle(Geom_Curve)        startTangent(double length, double* param0 = nullptr, double* param1 = nullptr) const;

  virtual GraphicObject*    extendStart(double length) override;
  virtual GraphicObject*    extendEnd(double length) override;
  virtual GraphicObject*    invert() override;
  virtual void              setZ(double z) override;
  virtual Handle(AIS_Shape) toShape(double z = 0) override;
  virtual QString           toString() const override;

protected:
  explicit GOCircle(const QString& source);
  void     createCircle();

private:
  gp_Pnt centerPnt;
  gp_Dir axis;
  double r;

  friend class Util3D;
  };
#endif // GOCIRCLE_H
