/* 
 * **************************************************************************
 * 
 *  file:       goline.h
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
#ifndef GOLINE_H
#define GOLINE_H
#include "graphicobject.h"
#include <TopoDS_Edge.hxx>


class GOLine : public GraphicObject
{
public:
  explicit GOLine(const gp_Pnt& from, const gp_Pnt& to);
  virtual ~GOLine() = default;

  gp_Dir                 direction() const;
  virtual GraphicObject* extendStart(double length) override;
  virtual GraphicObject* extendEnd(double length) override;
  virtual GraphicObject* invert() override;
  virtual void           setZ(double z) override;
  virtual GraphicObject* split() override;

protected:
  explicit GOLine(const QString& source);
  void     createLine();

  friend class Util3D;
  friend class GOContour;
  };
#endif // GOLINE_H
