/* 
 * **************************************************************************
 * 
 *  file:       targetdefinition.cpp
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
#include "targetdefinition.h"
#include "core.h"
#include "graphicobject.h"
#include "gocontour.h"
#include "util3d.h"
#include <QSettings>


TargetDefinition::TargetDefinition(const gp_Pnt& pos, double radius, QObject* parent)
 : QObject(parent)
 , tdPos(pos)
 , zmin(0)
 , zmax(0)
 , r(radius)
 , cc(nullptr) {
  }


TargetDefinition::TargetDefinition(QSettings& s, QObject* parent) {
  tdPos.SetX(s.value("tdPosX").toDouble());
  tdPos.SetY(s.value("tdPosY").toDouble());
  tdPos.SetZ(s.value("tdPosZ").toDouble());
  r    = s.value("tdRadius").toDouble();
  zmin = s.value("tdZMin").toDouble();
  zmax = s.value("tdZMax").toDouble();
  QString cSrc = s.value("contour").toString();

  if (!cSrc.isEmpty()) {
     GraphicObject* go = Core().helper3D()->parseGraphicObject(s.value("contour").toString());

     cc = static_cast<GOContour*>(go);
     }
  }


void TargetDefinition::store(QSettings& s) {
  s.setValue("tdPosX", tdPos.X());
  s.setValue("tdPosY", tdPos.Y());
  s.setValue("tdPosZ", tdPos.Z());
  s.setValue("tdRadius", r);
  s.setValue("tdZMin", zMin());
  s.setValue("tdZMax", zMax());
  if (cc) s.setValue("contour", cc->toString());
  }


bool TargetDefinition::compareASC(TargetDefinition *left, TargetDefinition *right) {
  if (!left || !right) return false;
  return atan2(left->pos().Y(), left->pos().X()) < atan2(right->pos().Y(), right->pos().X());
  }


bool TargetDefinition::compareDESC(TargetDefinition *left, TargetDefinition *right) {
  if (!left || !right) return false;
  return atan2(left->pos().Y(), left->pos().X()) > atan2(right->pos().Y(), right->pos().X());
  }
