/* 
 * **************************************************************************
 * 
 *  file:       ppfanuc.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    11.4.2022 by Django Reinhard
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
#include "ppfanuc.h"
#include <DrillCycle.h>
#include <gp_Pnt.hxx>
#include <QString>


PPFanuc::PPFanuc(QObject* parent)
 : DINPostProcessor(parent) {
  }


QString PPFanuc::genDefineCycle(int c, double topZ, double r0, double r1, double depth, double qMin, double qMax, double retract, double dwell, int feed) {
  if (depth >= topZ) return QString();
  switch (c) {
    case FineBoringCycle: // G76 (wohlhaupter)
         return QString("G76 Z%1 R%2 Q%3 P%4 F%5").arg(depth, 0, 'f', Decimals)
                                              .arg(topZ + r0, 0, 'f', Decimals)
                                              .arg(retract, 0, 'f', Decimals)
                                              .arg(dwell, 0, 'f', 0)
                                              .arg(feed);
         break;
    case SpotDrillCycle:  // G81
         return QString("G81 Z%1 R%2 F%3").arg(depth, 0, 'f', Decimals)
                                          .arg(topZ + r0, 0, 'f', Decimals)
                                          .arg(feed);
         break;
    case DrillWithDwell:  // G82 (dwell at final z)
         return QString("G82 Z%1 R%2 P%3 F%4").arg(depth, 0, 'f', Decimals)
                                              .arg(topZ + r0, 0, 'f', Decimals)
                                              .arg(dwell, 0, 'f', 0)
                                              .arg(feed);
         break;
    case PeckDrilling:    // G83
         return QString("G83 Z%1 R%2 Q%3 F%4").arg(depth, 0, 'f', Decimals)
                                              .arg(topZ + r0, 0, 'f', Decimals)
                                              .arg(qMax, 0, 'f', 0)
                                              .arg(feed);
         break;
    case Tapping:         // G84
         return QString("G84 Z%1 R%2 F%3").arg(depth, 0, 'f', Decimals)
                                          .arg(topZ + r0, 0, 'f', Decimals)
                                          .arg(feed);
         break;
    case BoringCycle:     // G85 (retract with feed)
         return QString("G85 Z%1 R%2 F%3").arg(depth, 0, 'f', Decimals)
                                          .arg(topZ + r0, 0, 'f', Decimals)
                                          .arg(feed);
         break;
    default: break;
    }
  return QString();
  }


QString PPFanuc::genEndCycle() {
  return "G80";
  }


QString PPFanuc::genEndOfLine() {
  return ";\n";
  }


QString PPFanuc::getFileExtension() const {
  return "dnc";
  }


QString PPFanuc::fixtureID(int f) {
  if (f < 7) return QString("G%1").arg(53 + f);
  }
