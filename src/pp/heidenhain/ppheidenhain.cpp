/* 
 * **************************************************************************
 * 
 *  file:       ppheidenhain.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    17.5.2022 by Django Reinhard
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
#include "ppheidenhain.h"
#include "DrillCycle.h"
#include "gp_Pnt.hxx"
#include <QString>
const double MinDelta = 1e-5;  //TODO: move to superclass
const int    Decimals = 3;


inline QString num2Pos(double num) {
  return QString("%1%2").arg(num > 0 ? "+" : "").arg(num, 0, 'f', Decimals);
  }


PPHeidenhain::PPHeidenhain(QObject* parent)
 : AbstractPostProcessor(parent) {
  }


QString PPHeidenhain::genArc(const gp_Pnt& nxtPos, const gp_Pnt& center, bool ccw, double feed) {
  QString cmd("CC");

  cmd += " X" + num2Pos(center.X())
       + " Y" + num2Pos(center.Y())
       + "\n";

  cmd += "C X" + num2Pos(nxtPos.X())
       +  " Y" + num2Pos(nxtPos.Y())
       +  " Z" + num2Pos(nxtPos.Z())
       +  (ccw ? " DR+" : " DR-");
  lPos = nxtPos;

  return cmd;
  }


QString PPHeidenhain::genDefineCycle(int c, double topZ, double r0, double r1, double depth, double qMin, double qMax, double retract, double dwell, int feed) {
  QString cmd;

  switch (c) {
    case FineBoringCycle: // G76 (wohlhaupter)
         cmd += QString("CYCLE DEF 202\n")
              + QString(" Q200=%1\n").arg(r0, 0, 'f', Decimals)             // safe z 1
              + QString(" Q201=%1\n").arg(topZ - depth, 0, 'f', Decimals)   // depth
              + QString(" Q203=%1\n").arg(topZ, 0, 'f', Decimals)           // top surface
              + QString(" Q204=%1\n").arg(r1, 0, 'f', Decimals)             // safe z 2
              + QString(" Q206=%1\n").arg(feed, 0)                          // feed
              + QString(" Q208=%1\n").arg(2 * feed, 0)                      // feed retract
              + " Q211=0.1\n"                                               // dwell
              + " Q214=1\n"                                                 // retract direction
                // direction: 0 - no retract, 1 x-, 2 x+, 3 y-, 4 y+
              + " Q336=0\n";                                                // angle spindle orientation
         break;
    case SpotDrillCycle:  // G81
         cmd += QString("CYCLE DEF 240\n")
              + QString(" Q200=%1\n").arg(r0, 0, 'f', Decimals)             // safe z 1
              + QString(" Q201=%1\n").arg(topZ - depth, 0, 'f', Decimals)   // depth
              + QString(" Q203=%1\n").arg(topZ, 0, 'f', Decimals)           // top surface
              + QString(" Q204=%1\n").arg(r1, 0, 'f', Decimals)             // safe z 2
              + QString(" Q206=%1\n").arg(feed, 0)                          // feed
              + " Q211=0.1\n"                                               // dwell
              + " Q343=0";                                                  // use depth
         break;
    case DrillWithDwell:  // G82 (dwell at final z)
         break;
    case PeckDrilling:    // G83
         cmd += QString("CYCLE DEF 200\n")
              + QString(" Q200=%1\n").arg(r0, 0, 'f', Decimals)             // safe z 1
              + QString(" Q201=%1\n").arg(topZ - depth, 0, 'f', Decimals)   // depth
              + QString(" Q202=%1\n").arg(qMax,   0, 'f', Decimals)         // step size
              + QString(" Q203=%1\n").arg(topZ, 0, 'f', Decimals)           // top surface
              + QString(" Q204=%1\n").arg(r1, 0, 'f', Decimals)             // safe z 2
              + QString(" Q206=%1\n").arg(feed, 0)                          // feed
              + " Q211=0.1\n"                                               // dwell
              + " Q395=0\n";                                                // use tool tip depth
         break;
    case Tapping:         // G84
         cmd += QString("CYCLE DEF 206\n")
              + QString(" Q200=%1\n").arg(r0, 0, 'f', Decimals)             // safe z 1
              + QString(" Q201=%1\n").arg(topZ - depth, 0, 'f', Decimals)   // depth
              + QString(" Q203=%1\n").arg(topZ, 0, 'f', Decimals)           // top surface
              + QString(" Q204=%1\n").arg(r1, 0, 'f', Decimals)             // safe z 2
              + QString(" Q206=%1\n").arg(feed, 0)                          // feed
              + " Q211=0.1\n";                                              // dwell
         break;
    case BoringCycle:     // G85 (retract with feed)
         break;
    default: break;
    }
  return cmd;
  }


QString PPHeidenhain::genDefineWorkpiece(const gp_Pnt& minCorner, const gp_Pnt& maxCorner) {
  QString rv = QString("BLK FORM 0.1 Z X%1 Y%2 Z%3\n").arg(minCorner.X(), 0, 'f', 3)
                                                      .arg(minCorner.Y(), 0, 'f', 3)
                                                      .arg(minCorner.Z(), 0, 'f', 3);
  rv += QString("BLK FORM 0.2 X%1 Y%2 Z%3").arg(maxCorner.X(), 0, 'f', 3)
                                           .arg(maxCorner.Y(), 0, 'f', 3)
                                           .arg(maxCorner.Z(), 0, 'f', 3);
  return rv;
  };


QString PPHeidenhain::genEndCycle() {
  return QString();
  }


QString PPHeidenhain::genExecCycle(int c, double x, double y) {
  QString cmd("L");

  cmd += " X" + num2Pos(x)
       + " Y" + num2Pos(y)
       + " R0 FMAX M99";
  lPos.SetX(x);
  lPos.SetY(y);

  return cmd;
  }


QString PPHeidenhain::genEndOfLine() {
  return "\n";
  }


QString PPHeidenhain::getFileExtension() const {
  return "h";
  }


QString PPHeidenhain::fixtureID(int f) {
//  if (f < 7) return QString("%1").arg(53 + f);
//  return QString("59.%1").arg(f - 6);
  return "";
  }


QString PPHeidenhain::genJobIntro(const QString& jobName) {
  QString name(jobName);

  name.replace('.', '_');

  return QString("BEGIN PGM %1 MM").arg(name);
  }


QString PPHeidenhain::genJobExit(const QString& jobName) {
  QString name(jobName);
  QString cmd("TOOL CALL 0\n\n");

  name.replace('.', '_');
  cmd += QString("END PGM %1 MM").arg(name);

  return cmd;
  }


QString PPHeidenhain::genLengthCorrEnd() {
  return QString();
  }


QString PPHeidenhain::genLengthCorrStart(int) {
  return QString();
  }


QString PPHeidenhain::genLineComment(const QString &msg) {
  return QString("( %1 )").arg(msg);
  }


QString PPHeidenhain::genOPIntro(int num, int fixture, const gp_Pnt& pos, double speed, double feed, int toolNum, int cooling, int nxtToolNum) {
  QString cmd("TOOL CALL");
  QString mcCooling;

  switch (cooling) {
    case 1:  mcCooling = "M7"; break;   // mist  cooling
    case 2:  mcCooling = "M8"; break;   // flood cooling
    default: break;
    }

  cmd += QString(" %1 Z S%2 F%3 M3 %4\n")
                .arg(toolNum)
                .arg(speed, 0, 'f', 0)
                .arg(feed, 0, 'f', 0)
                .arg(mcCooling);
  cmd += QString("TOOL DEF %1\n").arg(nxtToolNum);
  cmd += "L X" + num2Pos(pos.X())
       + " Y"  + num2Pos(pos.Y())
       + " R0 FMAX\n";
  cmd += "L Z" + num2Pos(pos.Z()) + " R0 FMAX";
  lPos = pos;

  return cmd;
  }


QString PPHeidenhain::genOPExit() {
  return "L Z+150 R0 FMAX M5 M9";
  }


QString PPHeidenhain::genPrepareTool(int toolNum) {
  return QString("TOOL DEF %1").arg(toolNum);
  }


QString PPHeidenhain::genProminentComment(const QString &msg) {
  return QString("( %1 )").arg(msg);
  }


QString PPHeidenhain::genRadiusCorrEnd() {
  radiusCorr = 0;

  return QString();
  }


QString PPHeidenhain::genRadiusCorrStart(const gp_Pnt& nxtPos, int toolSetNum, bool right) {
  radiusCorr = right ? 1 : -1;

  return QString();
  }


//TODO: what to use - cycle 19 or plane spatial?
QString PPHeidenhain::genRotation(double a, double b, double c) {
  QString cmd("PLANE SPATIAL");

  cmd += QString(" SPA") + num2Pos(a)
       + QString(" SPB") + num2Pos(b)
       + QString(" SPC") + num2Pos(c);
  return cmd;
  }


QString PPHeidenhain::genStraightMove(const gp_Pnt &nxtPos, double feed) {
  double  deltaX = nxtPos.X() - lPos.X();
  double  deltaY = nxtPos.Y() - lPos.Y();
  double  deltaZ = nxtPos.Z() - lPos.Z();
  QString cmd("L");

  if (abs(deltaX) > MinDelta) cmd += " X" + num2Pos(nxtPos.X());
  if (abs(deltaY) > MinDelta) cmd += " Y" + num2Pos(nxtPos.Y());
  if (abs(deltaZ) > MinDelta) cmd += " Z" + num2Pos(nxtPos.Z());
  if (radiusCorr > 0)      cmd += " RR";
  else if (radiusCorr < 0) cmd += " RL";
  else                     cmd += " R0";
  lPos = nxtPos;

  return cmd;
  }


QString PPHeidenhain::genToolChange() {
  return "";
  }


QString PPHeidenhain::genTraverse(const gp_Pnt &nxtPos, int lastCode) {
  double  deltaX = nxtPos.X() - lPos.X();
  double  deltaY = nxtPos.Y() - lPos.Y();
  double  deltaZ = nxtPos.Z() - lPos.Z();
  QString cmd("L");

  if (abs(deltaX) > MinDelta) cmd += " X" + num2Pos(nxtPos.X());
  if (abs(deltaY) > MinDelta) cmd += " Y" + num2Pos(nxtPos.Y());
  if (abs(deltaZ) > MinDelta) cmd += " Z" + num2Pos(nxtPos.Z());
  if (radiusCorr > 0)      cmd += " RR";
  else if (radiusCorr < 0) cmd += " RL";
  else                     cmd += " R0";
  cmd += " FMAX";
  lPos = nxtPos;

  return cmd;
  }


gp_Pnt PPHeidenhain::lastPos() const {
  return lPos;
  }


void PPHeidenhain::setLastPos(const gp_Pnt& pos) {
  lPos = pos;
  }
