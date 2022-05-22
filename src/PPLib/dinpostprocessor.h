/* 
 * **************************************************************************
 * 
 *  file:       dinpostprocessor.h
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
#ifndef DINPOSTPROCESSOR_H
#define DINPOSTPROCESSOR_H
#include "abstractpostprocessor.h"


class DINPostProcessor : public AbstractPostProcessor, public PostProcessor
{
  Q_OBJECT
  Q_INTERFACES(PostProcessor)
public:
  explicit DINPostProcessor(QObject *parent = nullptr);
  virtual ~DINPostProcessor() = default;

  virtual QString fixtureID(int f) override;
  virtual QString genArc(const gp_Pnt& nxtPos, const gp_Pnt& center, bool ccw, double feed) override;
  virtual QString genDefineCycle(int c, double topZ, double r0, double r1, double depth, double qMin, double qMax, double retract, double dwell, int feed) override;
  virtual QString genDefineWorkpiece(const gp_Pnt& minCorner, const gp_Pnt& maxCorner) override;
  virtual QString genEndOfLine()  override;
  virtual QString genExecCycle(int c, double x, double y) override;
  virtual QString genJobExit(const QString& jobName)  override;
  virtual QString genJobIntro(const QString& jobName) override;
  virtual QString genLengthCorrEnd() override;
  virtual QString genLengthCorrStart(int toolNum) override;
  virtual QString genLineComment(const QString& msg) override;
  virtual QString genOPExit()     override;
  virtual QString genOPIntro(int num, int fixture, const gp_Pnt& pos, double speed, double feed, int toolNum, int cooling, int nxtToolNum) override;
  virtual QString genPrepareTool(int toolNum) override;
  virtual QString genProminentComment(const QString& msg) override;
  virtual QString genRadiusCorrEnd() override;
  virtual QString genRadiusCorrStart(const gp_Pnt& nxtPos, int toolSetNum, bool right = false) override;
  virtual QString genRotation(double a, double b, double c) override;
  virtual QString genStraightMove(const gp_Pnt& nxtPos, double feed) override;
  virtual QString genToolChange() override;
  virtual QString genTraverse(const gp_Pnt& nxtPos, int lastCode) override;
  virtual QString getFileExtension() const override;
  virtual gp_Pnt  lastPos() const override;
  virtual void    setLastPos(const gp_Pnt& pos) override;
  };
#endif // DINPOSTPROCESSOR_H
