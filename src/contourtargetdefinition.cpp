/* 
 * **************************************************************************
 * 
 *  file:       contourtargetdefinition.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    29.3.2022 by Django Reinhard
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
#include "contourtargetdefinition.h"


ContourTargetDefinition::ContourTargetDefinition(const gp_Pnt& pos, QObject *parent)
 : TargetDefinition(pos, 0, parent) {
  }


QString ContourTargetDefinition::toString() const {
  QString rv = QString("%1/%2/%3")
                      .arg(pos().X())
                      .arg(pos().Y())
                      .arg(pos().Z());
  return rv;
  }
