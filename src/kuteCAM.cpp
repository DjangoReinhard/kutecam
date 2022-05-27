/* 
 * **************************************************************************
 * 
 *  file:       kuteCAM.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    11.4.2022 by Django Reinhard
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
#include "kuteCAM.h"
#include <QObject>


namespace kute {

int getDominantAxis(const gp_Dir& dir) {
  int rv = 0;

  if (abs(dir.X()) > abs(dir.Y())) {
     if (abs(dir.X()) > abs(dir.Z())) {
        if (dir.X() > 0) return 1;
        return -1;
        }
     if (dir.Z() > 0) return  3;
     return -3;
     }
  else {
     if (abs(dir.Y()) > abs(dir.Z())) {
        if (dir.Y() > 0) return  2;
        return -2;
        }
     if (dir.Z() > 0) return 3;
     return -3;
     }
  return rv;
  }


int relPos(const gp_Pnt& reference, const gp_Pnt& other) {
  double d = -reference.X() * other.Y() + reference.Y() * other.X();

  if (d < 0) return -1;
  if (d > 0) return 1;
  return 0;
  }


const double  MinDelta    = 0.001;
const QString BasePath    = "/media/Scratch";
const QString CADBase     = "/media/CAD";
const QString CADFilter   = QObject::tr("BRep Files (*.brep);;Step Files (*.stp *.step)");
const QString GCodeFilter = QObject::tr("Fanuc Files (*.dnc);;Heidenhain Files (*.h);;Sinumeric Files (*.arc)");
}
