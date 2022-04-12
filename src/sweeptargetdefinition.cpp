/* 
 * **************************************************************************
 * 
 *  file:       sweeptargetdefinition.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    10.4.2022 by Django Reinhard
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
#include "sweeptargetdefinition.h"
#include <QSettings>


SweepTargetDefinition::SweepTargetDefinition(const gp_Pnt& pos, const gp_Dir& dir, double radius, QObject *parent)
 : TargetDefinition(pos, parent)
 , soDir(dir)
 , r(radius) {
  }


SweepTargetDefinition::SweepTargetDefinition(QSettings& s, QObject* parent)
 : TargetDefinition(s, parent) {
  double x = s.value("std-dirX").toDouble();
  double y = s.value("std-dirY").toDouble();
  double z = s.value("std-dirZ").toDouble();

  soDir = gp_Dir(x, y, z);
  r = s.value("std-r").toDouble();
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
  s.setValue("std-zMin", zMin());
  s.setValue("std-zMax", zMax());
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
