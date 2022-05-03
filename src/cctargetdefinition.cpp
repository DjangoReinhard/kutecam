/* 
 * **************************************************************************
 * 
 *  file:       cctargetdefinition.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    28.4.2022 by Django Reinhard
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
#include "cctargetdefinition.h"
#include <QSettings>


CCTargetDefinition::CCTargetDefinition(const gp_Pnt& pMin, const gp_Pnt& pMax, QObject *parent)
 : TargetDefinition(pMin, 0, parent) {
  this->pMax = pMax;
  }


CCTargetDefinition::CCTargetDefinition(QSettings& s, QObject* parent)
 : TargetDefinition(s, parent) {
  double x = s.value("ccPosX").toDouble();
  double y = s.value("ccPosY").toDouble();
  double z = s.value("ccPosZ").toDouble();

  pMax = gp_Pnt(x, y, z);
  }


void CCTargetDefinition::store(QSettings& s) {
  s.setValue("tdType", "CCTarget");
  TargetDefinition::store(s);
  s.setValue("ccPosX", pMax.X());
  s.setValue("ccPosY", pMax.Y());
  s.setValue("ccPosZ", pMax.Z());
  }


QString CCTargetDefinition::toString() const {
  QString rv = QString("%1/%2/%3 - %4/%5/%6")
                      .arg(pos().X())
                      .arg(pos().Y())
                      .arg(pos().Z())
                      .arg(pMax.X())
                      .arg(pMax.Y())
                      .arg(pMax.Z());
  return rv;
  }
