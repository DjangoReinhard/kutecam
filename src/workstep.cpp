/* 
 * **************************************************************************
 * 
 *  file:       workstep.cpp
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
#include "workstep.h"
#include <QSettings>
#include <QDebug>


Workstep::Workstep(WorkstepType wt, const gp_Pnt& from, const gp_Pnt& to, QObject* parent)
 : wsType(wt)
 , start(from)
 , end(to) {
  }


Workstep::Workstep(WorkstepType wt, QSettings& s, QObject* parent)
 : wsType(wt) {
  start.SetX(s.value("wsStartX").toDouble());
  start.SetY(s.value("wsStartY").toDouble());
  start.SetZ(s.value("wsStartZ").toDouble());
  end.SetX(s.value("wsEndX").toDouble());
  end.SetY(s.value("wsEndY").toDouble());
  end.SetZ(s.value("wsEndZ").toDouble());
  }


QString Workstep::className() const {
  return "Workstep";
  }


void Workstep::dump() const {
  qDebug() << className() << "-Type:" << wsType << "from:" << start.X() << " / " << start.Y() << " / " << start.Z()
                                                << "  to:" << end.X() << " / " << end.Y() << " / " << end.Z();
  }


gp_Pnt Workstep::endPos() const {
  return end;
  }


gp_Pnt Workstep::startPos() const {
  return start;
  }


WorkstepType Workstep::type() const {
  return wsType;
  }


void Workstep::store(QSettings& s) {
  s.setValue("wsType",   wsType);
  s.setValue("wsStartX", start.X());
  s.setValue("wsStartY", start.Y());
  s.setValue("wsStartZ", start.Z());
  s.setValue("wsEndX", end.X());
  s.setValue("wsEndY", end.Y());
  s.setValue("wsEndZ", end.Z());
  }
