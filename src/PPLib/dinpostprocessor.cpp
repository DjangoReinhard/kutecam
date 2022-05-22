/* 
 * **************************************************************************
 * 
 *  file:       dinpostprocessor.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    22.5.2022 by Django Reinhard
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
#include "dinpostprocessor.h"
#include <gp_Pnt.hxx>


DINPostProcessor::DINPostProcessor(QObject *parent)
 : AbstractPostProcessor(parent) {
  }


QString DINPostProcessor::fixtureID(int f) {
  if (f < 7) return QString("G%1").arg(53 + f);
  return QString("G59.%1").arg(f - 6);
  }


QString DINPostProcessor::genArc(const gp_Pnt& nxtPos, const gp_Pnt& center, bool ccw, double feed) {
  double  deltaX = nxtPos.X() - lPos.X();
  double  deltaY = nxtPos.Y() - lPos.Y();
  double  deltaZ = nxtPos.Z() - lPos.Z();
  double  cX = center.X() - lPos.X();
  double  cY = center.Y() - lPos.Y();
  double  cZ = center.Z() - lPos.Z();
  QString cmd    = QString("G%1").arg(ccw ? 3 : 2);

  if (abs(deltaX) > MinDelta) cmd += QString(" X%2").arg(nxtPos.X(), 0, 'f', Decimals);
  if (abs(deltaY) > MinDelta) cmd += QString(" Y%3").arg(nxtPos.Y(), 0, 'f', Decimals);
  if (abs(deltaZ) > MinDelta) cmd += QString(" Z%4").arg(nxtPos.Z(), 0, 'f', Decimals);
  if (abs(cX)     > MinDelta) cmd += QString(" I%1").arg(cX, 0, 'f', Decimals);
  if (abs(cY)     > MinDelta) cmd += QString(" J%1").arg(cY, 0, 'f', Decimals);
  if (abs(cZ)     > MinDelta) cmd += QString(" K%1").arg(cZ, 0, 'f', Decimals);
  if (feed)                   cmd += QString(" F%1").arg(feed, 0, 'f', 0);

  lPos = nxtPos;

  return cmd;
  }


QString DINPostProcessor::genDefineCycle(int c, double topZ, double r0, double r1, double finalZ, double qMin, double qMax, double retract, double dwell, int feed) {
  return QString();
  }


QString DINPostProcessor::genDefineWorkpiece(const gp_Pnt& minCorner, const gp_Pnt& maxCorner) {
  return "";
  }


QString DINPostProcessor::genEndOfLine() {
  return "\n";
  }



QString DINPostProcessor::genExecCycle(int c, double x, double y) {
  QString cmd = QString(" X%1 Y%2").arg(x, 0, 'f', Decimals)
                                   .arg(y, 0, 'f', Decimals);
  lPos.SetX(x);
  lPos.SetY(y);

  return cmd;
  }


QString DINPostProcessor::getFileExtension() const {
  return "DIN";
  }


QString DINPostProcessor::genJobIntro(const QString&) {
  QString cmd(genRadiusCorrEnd());

  cmd += " " + genEndCycle();

  return cmd;
  }


QString DINPostProcessor::genJobExit(const QString&) {
  return "G91 G28 Y0 Z0;\nM30";
  }


QString DINPostProcessor::genLengthCorrEnd() {
  return QString();
  }


QString DINPostProcessor::genLengthCorrStart(int toolNum) {
  return QString("G43 H%1").arg(toolNum);
  }


QString DINPostProcessor::genLineComment(const QString &msg) {
  return QString("( %1 )").arg(msg);
  }


QString DINPostProcessor::genOPIntro(int num, int fixture, const gp_Pnt& pos, double speed, double feed, int toolNum, int cooling, int nxtToolNum) {
  QString mcCooling = "";
  QString cmd = QString("N%1 G0 G90 %2 X%3 Y%4 S%5 M3 ")
                       .arg(num * 10)
                       .arg(fixtureID(fixture))
                       .arg(pos.X(), 0, 'f', Decimals)
                       .arg(pos.Y(), 0, 'f', Decimals)
                       .arg(speed, 0, 'f', 0);
  switch (cooling) {
    case 1:  mcCooling = "M7"; break;   // mist  cooling
    case 2:  mcCooling = "M8"; break;   // flood cooling
    default: break;
    }
  cmd += genEndOfLine();
  cmd += genPrepareTool(nxtToolNum);
  cmd += genEndOfLine();
  cmd += genLengthCorrStart(toolNum);
  cmd += QString(" Z%8 %9")
                .arg(pos.Z(), 0, 'f', Decimals)
                .arg(mcCooling);
  lPos = pos;

  return cmd;
  }


QString DINPostProcessor::genOPExit() {
  QString cmd("G0 G90 Z150 M5");

  cmd += genEndOfLine();
  cmd += "M9";

  return cmd;
  }


QString DINPostProcessor::genProminentComment(const QString &msg) {
  return QString("( %1 )").arg(msg);
  }


QString DINPostProcessor::genRadiusCorrEnd() {
  radiusCorr = 0;

  return "G40";
  }


QString DINPostProcessor::genRadiusCorrStart(const gp_Pnt& nxtPos, int toolSetNum, bool right) {
  QString cmd = QString("G1 G4%1 D%2")
                       .arg(right ? 2 : 1)
                       .arg(toolSetNum);

  cmd += QString(" X%1 Y%2").arg(nxtPos.X(), 0, 'f', Decimals)
                            .arg(nxtPos.Y(), 0, 'f', Decimals);
  if (abs(nxtPos.Z() - lPos.Z()) > MinDelta)
     cmd += QString(" Z%1").arg(nxtPos.Z(), 0, 'f', Decimals);
  radiusCorr = right ? 1 : -1;
  lPos    = nxtPos;

  return cmd;
  }


QString DINPostProcessor::genRotation(double a, double b, double c) {
  QString cmd = QString("G90 A%1 B%2 C%3").arg(a, 0, 'f', Decimals)
                                          .arg(b, 0, 'f', Decimals)
                                          .arg(c, 0, 'f', Decimals);
  rot.SetX(a);
  rot.SetY(b);
  rot.SetZ(c);

  return cmd;
  }


QString DINPostProcessor::genStraightMove(const gp_Pnt &nxtPos, double feed) {
  double  deltaX = nxtPos.X() - lPos.X();
  double  deltaY = nxtPos.Y() - lPos.Y();
  double  deltaZ = nxtPos.Z() - lPos.Z();
  QString cmd("G1 ");

  if (abs(deltaX) > MinDelta) cmd += QString(" X%1").arg(nxtPos.X(), 0, 'f', Decimals);
  if (abs(deltaY) > MinDelta) cmd += QString(" Y%1").arg(nxtPos.Y(), 0, 'f', Decimals);
  if (abs(deltaZ) > MinDelta) cmd += QString(" Z%1").arg(nxtPos.Z(), 0, 'f', Decimals);
  if (feed)                   cmd += QString(" F%1").arg(feed, 0, 'f', 0);

  lPos = nxtPos;

  return cmd;
  }


QString DINPostProcessor::genToolChange() {
  return "M98 P100";
  }


QString DINPostProcessor::genPrepareTool(int toolNum) {
  return QString("T%1").arg(toolNum);
  }


QString DINPostProcessor::genTraverse(const gp_Pnt &nxtPos, int lastCode) {
  double  deltaX = nxtPos.X() - lPos.X();
  double  deltaY = nxtPos.Y() - lPos.Y();
  double  deltaZ = nxtPos.Z() - lPos.Z();
  QString cmd;

  if (lastCode) cmd = "G0 ";
  if (abs(deltaX) > MinDelta) cmd += QString(" X%1").arg(nxtPos.X(), 0, 'f', Decimals);
  if (abs(deltaY) > MinDelta) cmd += QString(" Y%1").arg(nxtPos.Y(), 0, 'f', Decimals);
  if (abs(deltaZ) > MinDelta) cmd += QString(" Z%1").arg(nxtPos.Z(), 0, 'f', Decimals);

  lPos = nxtPos;

  return cmd;
  }


gp_Pnt DINPostProcessor::lastPos() const {
  return lPos;
  }


void DINPostProcessor::setLastPos(const gp_Pnt& pos) {
  lPos = pos;
  }
