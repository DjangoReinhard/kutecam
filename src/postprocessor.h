/* 
 * **************************************************************************
 * 
 *  file:       postprocessor.h
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
#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H
#include <QString>
class Operation;
class ToolEntry;
class gp_Pnt;


class PostProcessor
{
public:
  explicit PostProcessor();
  virtual ~PostProcessor();

  virtual QString genEndOfLine()  = 0;
  virtual QString genJobIntro()   = 0;
  virtual QString genJobExit()    = 0;
  virtual QString genOPExit()     = 0;
  virtual QString genToolChange() = 0;

  virtual QString genArc(const gp_Pnt& lastPos, const gp_Pnt& nxtPos, const gp_Pnt& center, bool ccw, double feed) = 0;
  virtual QString genDefineCycle(int c, double r0, double r1, double finalZ, double qMin, double qMax, double retract, double dwell, int feed) = 0;
  virtual QString genDefineWorkpiece(const gp_Pnt& minCorner, const gp_Pnt& maxCorner) { return ""; };
  virtual QString genExecCycle(int c, double x, double y) = 0;
  virtual QString genLineComment(const QString& msg) = 0;
  virtual QString genOPIntro(int num, int fixture, const gp_Pnt& pos, double speed, ToolEntry* tool, int cooling, int nxtToolNum) = 0;
  virtual QString genProminentComment(const QString& msg) = 0;
  virtual QString genRotation(double a, double b, double c) = 0;
  virtual QString genStraightMove(const gp_Pnt& lastPos, const gp_Pnt& nxtPos, double feed) = 0;
  virtual QString genPrepareTool(ToolEntry* tool) = 0;
  virtual QString genTraverse(const gp_Pnt& lastPos, const gp_Pnt& nxtPos, int lastCode) = 0;
  };
#endif // POSTPROCESSOR_H
