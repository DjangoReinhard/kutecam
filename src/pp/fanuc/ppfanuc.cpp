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
#include <toolentry.h>
#include <gp_Pnt.hxx>
#include <QString>

const double MinDelta = 1e-5;
const int    Decimals = 3;


PPFanuc::PPFanuc() {
  }


QString PPFanuc::genArc(const gp_Pnt& lastPos, const gp_Pnt& nxtPos, const gp_Pnt& center, bool ccw, double feed) {
  double  deltaX = center.X() - lastPos.X();
  double  deltaY = center.Y() - lastPos.Y();
  double  deltaZ = center.Z() - lastPos.Z();
  QString cmd    = QString("G%1").arg(ccw ? 3 : 2);

//  if (nxtPos.X() != x) {

//     }
  cmd += QString("X%2").arg(nxtPos.X(), 0, 'f', Decimals);
  cmd += QString("Y%3").arg(nxtPos.Y(), 0, 'f', Decimals);
  cmd += QString("Z%4").arg(nxtPos.Z(), 0, 'f', Decimals);

  if (abs(deltaX) > MinDelta) cmd += QString(" I%1").arg(deltaX, 0, 'f', Decimals);
  if (abs(deltaY) > MinDelta) cmd += QString(" J%1").arg(deltaY, 0, 'f', Decimals);
  if (abs(deltaZ) > MinDelta) cmd += QString(" K%1").arg(deltaZ, 0, 'f', Decimals);
  if (feed)                   cmd += QString(" F%1").arg(feed, 0, 'f', 0);

  return cmd;
  }


QString PPFanuc::genDefineCycle(int c, double r0, double r1, double finalZ, double qMin, double qMax, double retract, double dwell, int feed) {
  switch (c) {
    case FineBoringCycle: // G76 (wohlhaupter)
         return QString("G76 Z%1 R%2 Q%3 P%4 F%5").arg(finalZ, 0, 'f', Decimals)
                                              .arg(r0, 0, 'f', Decimals)
                                              .arg(retract, 0, 'f', Decimals)
                                              .arg(dwell, 0, 'f', 0)
                                              .arg(feed);
         break;
    case SpotDrillCycle:  // G81
         return QString("G81 Z%1 R%2 F%3").arg(finalZ, 0, 'f', Decimals)
                                          .arg(r0, 0, 'f', Decimals)
                                          .arg(feed);
         break;
    case DrillWithDwell:  // G82 (dwell at final z)
         return QString("G82 Z%1 R%2 P%3 F%4").arg(finalZ, 0, 'f', Decimals)
                                              .arg(r0, 0, 'f', Decimals)
                                              .arg(dwell, 0, 'f', 0)
                                              .arg(feed);
         break;
    case PeckDrilling:    // G83
         return QString("G83 Z%1 R%2 Q%3 F%4").arg(finalZ, 0, 'f', Decimals)
                                              .arg(r0, 0, 'f', Decimals)
                                              .arg(qMax, 0, 'f', 0)
                                              .arg(feed);
         break;
    case Tapping:         // G84
         return QString("G84 Z%1 R%2 F%3").arg(finalZ, 0, 'f', Decimals)
                                          .arg(r0, 0, 'f', Decimals)
                                          .arg(feed);
         break;
    case BoringCycle:     // G85 (retract with feed)
         return QString("G85 Z%1 R%2 F%3").arg(finalZ, 0, 'f', Decimals)
                                          .arg(r0, 0, 'f', Decimals)
                                          .arg(feed);
         break;
    default: break;
    }
  return QString();
  }


QString PPFanuc::genExecCycle(int c, double x, double y) {
  return QString("X%1 Y%2").arg(x, 0, 'f', Decimals)
                           .arg(y, 0, 'f', Decimals);
  }


QString PPFanuc::genEndOfLine() {
  return ";\n";
  }


QString PPFanuc::genJobIntro() {
  return "G40 G80";
  }


QString PPFanuc::genJobExit() {
  return "G91 G28 Y0 Z0;\nM30";
  }


QString PPFanuc::genLineComment(const QString &msg) {
  return QString("( %1 )").arg(msg);
  }


QString PPFanuc::fixtureID(int f) {
  if (f < 7) return QString("%1").arg(53 + f);
  return QString("59.%1").arg(f - 6);
  }


QString PPFanuc::genOPIntro(int num, int fixture, const gp_Pnt& pos, double speed, ToolEntry* tool, int cooling, int nxtToolNum) {
  QString mcCooling = "";

  switch (cooling) {
    case 1:  mcCooling = "M7"; break;   // mist  cooling
    case 2:  mcCooling = "M8"; break;   // flood cooling
    default: break;
    }

  return QString("N%1 G0 G90 G%2 X%3 Y%4 S%5 M3 T%6;\nG43 H%7 Z%8 %9")
            .arg(num * 10)
            .arg(fixtureID(fixture))
            .arg(pos.X(), 0, 'f', Decimals)
            .arg(pos.Y(), 0, 'f', Decimals)
            .arg(speed, 0, 'f', 0)
            .arg(nxtToolNum)
            .arg(tool->toolNumber())
            .arg(pos.Z(), 0, 'f', Decimals)
            .arg(mcCooling);
  }


QString PPFanuc::genOPExit() {
  return "G0 G80 G90 Z150 M5;\nM9";
  }


QString PPFanuc::genProminentComment(const QString &msg) {
  return QString("( %1 )").arg(msg);
  }


QString PPFanuc::genRotation(double a, double b, double c) {
  return QString("G90 A%1 B%2 C%3").arg(a, 0, 'f', Decimals)
                                   .arg(b, 0, 'f', Decimals)
                                   .arg(c, 0, 'f', Decimals);
  }


QString PPFanuc::genStraightMove(const gp_Pnt &lastPos, const gp_Pnt &nxtPos, double feed) {
  if (feed) {
     return QString("G1 X%1 Y%2 Z%3 F%4").arg(nxtPos.X(), 0, 'f', Decimals)
                                         .arg(nxtPos.Y(), 0, 'f', Decimals)
                                         .arg(nxtPos.Z(), 0, 'f', Decimals)
                                         .arg(feed, 0, 'f', 0);
     }
  else {
     return QString("G1 X%1 Y%2 Z%3").arg(nxtPos.X(), 0, 'f', Decimals)
                                     .arg(nxtPos.Y(), 0, 'f', Decimals)
                                     .arg(nxtPos.Z(), 0, 'f', Decimals);
     }
  }


QString PPFanuc::genToolChange() {
  return "M98 P100";
  }


QString PPFanuc::genPrepareTool(ToolEntry* tool) {
  return QString("T%1").arg(tool->toolNumber());
  }


QString PPFanuc::genTraverse(const gp_Pnt &lastPos, const gp_Pnt &nxtPos, int lastCode) {    
  QString cmd;

  if (lastCode) cmd = "G0 ";
  cmd += QString("X%1 Y%2 Z%3").arg(nxtPos.X(), 0, 'f', Decimals)
                               .arg(nxtPos.Y(), 0, 'f', Decimals)
                               .arg(nxtPos.Z(), 0, 'f', Decimals);

  return cmd;
  }
