/* 
 * **************************************************************************
 * 
 *  file:       wstraverse.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    10.4.2022 by Django Reinhard
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
#include "wstraverse.h"
#include "core.h"
#include "util3d.h"
#include <QSettings>


WSTraverse::WSTraverse(const gp_Pnt& from, const gp_Pnt& to, QObject* parent)
 : Workstep(WTTraverse, from, to, parent) {
  if (Core().helper3D()->isEqual(from, to)) throw new std::domain_error("startpoint and endpoint may not be the same!");
  }


WSTraverse::WSTraverse(QSettings& s, QObject* parent)
 : Workstep(WTTraverse, s, parent) {
  }


QString WSTraverse::className() const {
  return "WSTraverse";
  }
