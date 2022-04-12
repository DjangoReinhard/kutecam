/* 
 * **************************************************************************
 * 
 *  file:       wsarc.cpp
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
#include "wsarc.h"
#include <QSettings>
#include <QDebug>


WSArc::WSArc(const gp_Pnt& from, const gp_Pnt& to, const gp_Pnt& center, bool ccw, QObject* parent)
 : Workstep(WTArc, from, to, parent)
 , center(center)
 , ccw(ccw)
 , inverted(false) {
  }


WSArc::WSArc(QSettings& s, QObject* parent)
 : Workstep(WTArc, s, parent)
 , ccw(false)
 , inverted(false) {
  center.SetX(s.value("wsCenterX").toDouble());
  center.SetY(s.value("wsCenterY").toDouble());
  center.SetZ(s.value("wsCenterZ").toDouble());
  ccw = s.value("wsCCW").toBool();
  inverted = s.value("wsInv").toBool();
  }


gp_Pnt WSArc::centerPos() const {
  return center;
  }


QString WSArc::className() const {
  return "WSArc";
  }


void WSArc::dump() const {
  Workstep::dump();
  qDebug() << "  WSArc - center:" << center.X() << " / " << center.Y() << " / " << center.Z()
                         << "ccw" << (ccw ? "YES" : "NO") << "inverted:"<< (inverted ? "YES" : "NO");
  }


bool WSArc::isCCW() const  {
  return ccw;
  }


void WSArc::store(QSettings& s) {
  Workstep::store(s);
  s.setValue("wsCenterX", center.X());
  s.setValue("wsCenterY", center.Y());
  s.setValue("wsCenterZ", center.Z());
  s.setValue("wsCCW", ccw);
  s.setValue("wsInv", inverted);
  }
