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
 : TargetDefinition(pos, radius, parent)
 , soDir(dir)
 , baseIsBorder(false) {
  }


SweepTargetDefinition::SweepTargetDefinition(GOContour* c, QObject* parent)
 : TargetDefinition(c->centerPoint(), 0, parent)
 , baseIsBorder(false) {
  setContour(c);
  }


SweepTargetDefinition::SweepTargetDefinition(QSettings& s, QObject* parent)
 : TargetDefinition(s, parent)
 , baseIsBorder(false) {
  double x = s.value("std-dirX").toDouble();
  double y = s.value("std-dirY").toDouble();
  double z = s.value("std-dirZ").toDouble();
  soDir = gp_Dir(x, y, z);
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
  }


void SweepTargetDefinition::store(QSettings &s) {
  s.setValue("tdType", "SweepTarget");
  TargetDefinition::store(s);
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
  }


QString SweepTargetDefinition::toString() const {
  QString rv = QString("%1/%2/%3  dir  %4/%5/%6\tZ:(%7 - %8)")
                      .arg(pos().X())
                      .arg(pos().Y())
                      .arg(pos().Z())
                      .arg(soDir.X())
                      .arg(soDir.Y())
                      .arg(soDir.Z())
                      .arg(zMin())
                      .arg(zMax());
  return rv;
  }
