/* 
 * **************************************************************************
 * 
 *  file:       drilltargetdefinition.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    7.4.2022 by Django Reinhard
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
#include "drilltargetdefinition.h"
#include <QSettings>


DrillTargetDefinition::DrillTargetDefinition(const gp_Pnt& pos, const gp_Dir& dir, double radius, QObject* parent)
 : TargetDefinition(pos, parent)
 , doDir(dir)
 , doRadius(radius) {
  }


DrillTargetDefinition::DrillTargetDefinition(QSettings& s, QObject* parent)
 : TargetDefinition(s, parent) {
  doDir.SetX(s.value("dtd-dirX").toDouble());
  doDir.SetY(s.value("dtd-dirY").toDouble());
  doDir.SetZ(s.value("dtd-dirZ").toDouble());
  doRadius = s.value("dtd-r").toDouble();
  }


void DrillTargetDefinition::store(QSettings& s) {
  s.setValue("tdType", "DrillTarget");
  TargetDefinition::store(s);
  s.setValue("dtd-r", radius());
  s.setValue("dtd-dirX", doDir.X());
  s.setValue("dtd-dirY", doDir.Y());
  s.setValue("dtd-dirZ", doDir.Z());
  }


QString DrillTargetDefinition::toString() const {
  QString rv = QString("R: %3\tat\t%1\t/\t%2").arg(tdPos.X())
                                              .arg(tdPos.Y())
                                              .arg(doRadius);
  return rv;
  }
