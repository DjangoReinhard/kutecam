/* 
 * **************************************************************************
 * 
 *  file:       gopocket.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    21.4.2022 by Django Reinhard
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
#ifndef GOPOCKET_H
#define GOPOCKET_H
#include "graphicobject.h"
#include <vector>
class GOContour;


class GOPocket : public GraphicObject
{
public:
//  GOPocket(double from, double to);
  GOPocket(const gp_Pnt& start, const gp_Pnt& endd);

  double a0() const;
  double a1() const;

  const std::vector<GOContour*>& contours() const;
  virtual bool                   add(GOContour* c);
  virtual GraphicObject*         extendBy(double length);
  virtual GraphicObject*         extendStart(double length) override;
  virtual GraphicObject*         extendEnd(double length) override;
  virtual GraphicObject*         invert() override;

protected:
  GOPocket(const QString& source);

private:
  std::vector<GOContour*> pool;

  friend class Util3D;
  };

extern bool compPocket(GOPocket* left, GOPocket* right);

#endif // GOPOCKET_H
