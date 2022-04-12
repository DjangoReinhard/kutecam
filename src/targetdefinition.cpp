/* 
 * **************************************************************************
 * 
 *  file:       targetdefinition.cpp
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
#include "targetdefinition.h"
#include <QSettings>


TargetDefinition::TargetDefinition(const gp_Pnt& pos, QObject* parent)
 : QObject(parent)
 , tdPos(pos) {
  }


TargetDefinition::TargetDefinition(QSettings& s, QObject* parent) {
  tdPos.SetX(s.value("tdPosX").toDouble());
  tdPos.SetY(s.value("tdPosY").toDouble());
  tdPos.SetZ(s.value("tdPosZ").toDouble());
  }


void TargetDefinition::store(QSettings& s) {
  s.setValue("tdPosX", tdPos.X());
  s.setValue("tdPosY", tdPos.Y());
  s.setValue("tdPosZ", tdPos.Z());
  }
