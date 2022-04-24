/* 
 * **************************************************************************
 * 
 *  file:       ppfanuc.h
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
#ifndef PPFANUC_H
#define PPFANUC_H
#include "postprocessor.h"


class PPFanuc : public PostProcessor
{
public:
  explicit PPFanuc();
  virtual ~PPFanuc() = default;

  QString fixtureID(int f);

  virtual QString genEndOfLine()  override;
  virtual QString genJobIntro()   override;
  virtual QString genJobExit()    override;
  virtual QString genOPExit()     override;
  virtual QString genToolChange() override;

  virtual QString genArc(const gp_Pnt& lastPos, const gp_Pnt& nxtPos, const gp_Pnt& center, bool ccw, double feed) override;
  virtual QString genDefineCycle(int c, double r0, double r1, double finalZ, double qMin, double qMax, double retract, double dwell, int feed) override;
  virtual QString genExecCycle(int c, double x, double y) override;
  virtual QString genLineComment(const QString& msg) override;
  virtual QString genOPIntro(int num, int fixture, const gp_Pnt& pos, double speed, ToolEntry* tool, int cooling, int nxtToolNum) override;
  virtual QString genProminentComment(const QString& msg) override;
  virtual QString genRotation(double a, double b, double c) override;
  virtual QString genStraightMove(const gp_Pnt& lastPos, const gp_Pnt& nxtPos, double feed) override;
  virtual QString genPrepareTool(ToolEntry* tool) override;
  virtual QString genTraverse(const gp_Pnt& lastPos, const gp_Pnt& nxtPos, int lastCode) override;
  };
#endif // PPFANUC_H
