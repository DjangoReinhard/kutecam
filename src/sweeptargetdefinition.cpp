/* 
 * **************************************************************************
 * 
 *  file:       sweeptargetdefinition.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    24.4.2022 by Django Reinhard
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
#include "sweeptargetdefinition.h"
#include "core.h"
#include "gocontour.h"
#include "util3d.h"
#include <QSettings>


SweepTargetDefinition::SweepTargetDefinition(const gp_Pnt& pos, const gp_Dir& dir, double radius, QObject *parent)
 : TargetDefinition(pos, parent)
 , soDir(dir)
 , cc(nullptr)
 , r(radius)
 , baseIsBorder(false) {
  }


SweepTargetDefinition::SweepTargetDefinition(GOContour* c, QObject* parent)
 : TargetDefinition(gp_Pnt(), parent)
 , cc(c)
 , r(0)
 , baseIsBorder(false) {
  }


SweepTargetDefinition::SweepTargetDefinition(QSettings& s, QObject* parent)
 : TargetDefinition(s, parent)
 , cc(nullptr)
 , r(0)
 , baseIsBorder(false) {
  double x = s.value("std-dirX").toDouble();
  double y = s.value("std-dirY").toDouble();
  double z = s.value("std-dirZ").toDouble();
  QString cSrc = s.value("contour").toString();

  if (!cSrc.isEmpty()) {
     GraphicObject* go = Core().helper3D()->parseGraphicObject(s.value("contour").toString());

     cc = static_cast<GOContour*>(go);
     }
  soDir = gp_Dir(x, y, z);
  r = s.value("std-r").toDouble();
  x = s.value("std-bdX").toDouble();
  y = s.value("std-bdY").toDouble();
  z = s.value("std-bdZ").toDouble();
  dirBase = gp_Dir(x, y, z);
  x = s.value("std-bXmin").toDouble();
  y = s.value("std-bYmin").toDouble();
  z = s.value("std-bZmin").toDouble();
  gp_Pnt min(x, y, z);

  x = s.value("std-bXmax").toDouble();
  y = s.value("std-bYmax").toDouble();
  z = s.value("std-bZmax").toDouble();
  bbBase = Bnd_Box(min, {x, y, z});
  baseIsBorder = s.value("std-bib").toBool();
  zmin = s.value("std-zMin").toDouble();
  zmax = s.value("std-zMax").toDouble();
  }


void SweepTargetDefinition::store(QSettings &s) {
  s.setValue("tdType", "SweepTarget");
  TargetDefinition::store(s);
  s.setValue("std-r", radius());
  s.setValue("std-dirX", soDir.X());
  s.setValue("std-dirY", soDir.Y());
  s.setValue("std-dirZ", soDir.Z());
  s.setValue("std-bdX", dirBase.X());
  s.setValue("std-bdY", dirBase.Y());
  s.setValue("std-bdZ", dirBase.Z());
  s.setValue("std-bXmin", bbBase.CornerMin().X());
  s.setValue("std-bYmin", bbBase.CornerMin().Y());
  s.setValue("std-bZmin", bbBase.CornerMin().Z());
  s.setValue("std-bXmax", bbBase.CornerMax().X());
  s.setValue("std-bYmax", bbBase.CornerMax().Y());
  s.setValue("std-bZmax", bbBase.CornerMax().Z());
  s.setValue("std-bib",  baseIsBorder);
  s.setValue("std-zMin", zMin());
  s.setValue("std-zMax", zMax());
  if (cc) s.setValue("contour", cc->toString());
  }


QString SweepTargetDefinition::toString() const {
  QString rv = QString("%1/%2/%3  dir  %4/%5/%6\tZ:(%7 - %8)")
                      .arg(tdPos.X())
                      .arg(tdPos.Y())
                      .arg(tdPos.Z())
                      .arg(soDir.X())
                      .arg(soDir.Y())
                      .arg(soDir.Z())
                      .arg(zmin)
                      .arg(zmax);
  return rv;
  }
