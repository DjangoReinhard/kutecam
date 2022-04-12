/* 
 * **************************************************************************
 * 
 *  file:       wscycle.cpp
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
#include "wscycle.h"
#include <QSettings>
#include <QDebug>


WSCycle::WSCycle(int cycle, const gp_Pnt& from, const gp_Pnt& to, double safeZ0, double safeZ1, double depth, QObject* parent)
 : Workstep(WTCycle, from, to, parent)
 , cycle(cycle)
 , sZ0(safeZ0)
 , sZ1(safeZ1)
 , depth(depth) {
  }


WSCycle::WSCycle(QSettings& s, QObject* parent)
 : Workstep(WTCycle, s, parent) {
  cycle = s.value("wsCycle").toInt();
  sZ0   = s.value("wsSafeZ0").toDouble();
  sZ1   = s.value("wsSafeZ1").toDouble();
  depth = s.value("wsDepth").toDouble();
  }


QString WSCycle::className() const {
  return "WSCycle";
  }


int WSCycle::drillCycle() const {
  return cycle;
  }


void WSCycle::dump() const {
  qDebug() << "WSCycle(" << cycle << ") from" << sZ1 << "through" << sZ0 << "until" << depth;
  }

double WSCycle::finalDepth() const {
  return depth;
  }


double WSCycle::safeZ0() const {
  return sZ0;
  }


double WSCycle::safeZ1() const {
  return sZ1;
  }


void WSCycle::store(QSettings& s) {
  Workstep::store(s);
  s.setValue("wsCycle", drillCycle());
  s.setValue("wsSafeZ0", safeZ0());
  s.setValue("wsSafeZ1", safeZ1());
  s.setValue("wsDepth", finalDepth());
  }
