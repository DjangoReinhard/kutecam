/* 
 * **************************************************************************
 * 
 *  file:       tdfactory.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    7.4.2022 by Django Reinhard
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
#include "tdfactory.h"
#include "cctargetdefinition.h"
#include "contourtargetdefinition.h"
#include "drilltargetdefinition.h"
#include "notchtargetdefinition.h"
#include "sweeptargetdefinition.h"
#include <QSettings>


TargetDefinition* TDFactory::createTargetDefinition(QSettings &s) {
  QString           type = s.value("tdType").toString();

  if (type == "DrillTarget")        return new DrillTargetDefinition(s);
  else if (type == "SweepTarget")   return new SweepTargetDefinition(s);
  else if (type == "CCTarget")      return new CCTargetDefinition(s);
  else if (type == "ContourTarget") return new ContourTargetDefinition(s);
  else if (type == "NotchTarget")   return new NotchTargetDefinition(s);
//  else if (type == "Face3DTarget")  return new Face3DTargetDefinition(s);
  return nullptr;
  }
