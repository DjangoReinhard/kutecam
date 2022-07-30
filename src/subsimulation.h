/* 
 * **************************************************************************
 * 
 *  file:       subsimulation.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    11.6.2022 by Django Reinhard
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
#ifndef SUBSIMULATION_H
#define SUBSIMULATION_H
#include "operationsubpage.h"
#include <QBasicTimer>
QT_BEGIN_NAMESPACE
namespace Ui {
class OpSim;
}
QT_END_NAMESPACE
class PathBuilder;


class SubSimulation : public OperationSubPage
{
  Q_OBJECT
public:
  explicit SubSimulation(OperationListModel* olm, TargetDefListModel* tdModel, PathBuilder* pb, QWidget* parent = nullptr);
  virtual ~SubSimulation() = default;

  void createTool(int toolNum);
  virtual void loadOP(Operation* op) override;
  virtual void processSelection() override;
  void restartSimulation();
  void stopSimulation();
  void resumeSimulation();
  virtual void genRoughingToolPath();
  virtual void genFinishingToolPath();

public slots:
  void moveCone(const gp_Pnt& pos);
  void timerEvent(QTimerEvent *e) override;

signals:
  void updatePosition(const gp_Pnt& pos);

private:
  Ui::OpSim*        ui;
  Handle(AIS_Shape) asTool;
  QBasicTimer       timer;
  int               timerShapeIndex;
  double            timerCurveOffset;
  };
#endif // SUBSIMULATION_H
