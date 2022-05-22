/*
 * **************************************************************************
 *
 *  file:       postprocessor.h
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
#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H
#include <QtPlugin>
class gp_Pnt;


class PostProcessor
{
public:
  explicit PostProcessor() {};
  virtual ~PostProcessor() = default;

  virtual QString fixtureID(int f) = 0;
  virtual QString genEndOfLine()   = 0;
  virtual QString genEndCycle()    = 0;
  virtual QString genJobIntro(const QString& jobName) = 0;
  virtual QString genJobExit(const QString& jobName)  = 0;
  virtual QString genLengthCorrEnd() = 0;
  virtual QString genLengthCorrStart(int toolNum) = 0;
  virtual QString genOPExit()        = 0;
  virtual QString genRadiusCorrEnd() = 0;
  virtual QString genRadiusCorrStart(const gp_Pnt& nxtPos, int toolSetNum, bool right = false) = 0;
  virtual QString genToolChange()    = 0;

  virtual QString genArc(const gp_Pnt& nxtPos, const gp_Pnt& center, bool ccw, double feed) = 0;
  virtual QString genDefineCycle(int c, double topZ, double r0, double r1, double depth, double qMin, double qMax, double retract, double dwell, int feed) = 0;
  virtual QString genDefineWorkpiece(const gp_Pnt& minCorner, const gp_Pnt& maxCorner) = 0;
  virtual QString genExecCycle(int c, double x, double y) = 0;
  virtual QString genLineComment(const QString& msg) = 0;
  virtual QString genOPIntro(int num, int fixture, const gp_Pnt& pos, double speed, double feed, int toolNum, int cooling, int nxtToolNum) = 0;
  virtual QString genProminentComment(const QString& msg) = 0;
  virtual QString genRotation(double a, double b, double c) = 0;
  virtual QString genStraightMove(const gp_Pnt& nxtPos, double feed) = 0;
  virtual QString genPrepareTool(int toolNum) = 0;
  virtual QString genTraverse(const gp_Pnt& nxtPos, int lastCode) = 0;

  virtual QString getFileExtension() const = 0;
  virtual gp_Pnt  lastPos() const = 0;
  virtual void    setLastPos(const gp_Pnt& pos) = 0;
  };


QT_BEGIN_NAMESPACE
#define PostProcessorPlugin_iid "de.schwarzrot.kuteCAM.PostProcessor/0.1"
Q_DECLARE_INTERFACE(PostProcessor, PostProcessorPlugin_iid)
QT_END_NAMESPACE
#endif // POSTPROCESSOR_H
